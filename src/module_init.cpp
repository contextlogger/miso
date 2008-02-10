//
// Copyright 2005 Helsinki Institute for Information Technology (HIIT)
// and the authors.  All rights reserved.
//
// Authors: Tero Hasu <tero.hasu@hut.fi>
//          "Cyke64" <cyke64 at users.sourceforge.net>
//

// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation files
// (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <e32std.h>
#include <Python.h>
#include <symbian_python_ext_util.h>
#include "local_epoc_py_utils.h"

#include <f32file.h> // RFs
#include <es_sock.h> // RSocketServ
#include <bttypes.h> // TBTDevAddr
#include <bt_sock.h> // BT protocol family constants
#include <hal.h> // HAL

#ifdef __BTDEVICENAME_IN_BTMANCLIENT__
#include <btmanclient.h> // TBTDeviceName
#endif

#ifdef __HAS_BT_SUBSCRIBE__
#include <bt_subscribe.h>
#endif

#ifdef __SYMBIAN_9__
#include <e32power.h>
#endif

#ifdef __DO_LOGGING__
#include <flogger.h> // flogger.lib required, Symbian 7.0-up
#endif

// ----------------------------------------------------------------------
// threads and processes

static PyObject* miso_GetThreadPriority(PyObject* /*self*/, PyObject* /*args*/)
{
  return Py_BuildValue("i", static_cast<int>(RThread().Priority()));
}

static PyObject* miso_SetThreadPriority(PyObject* /*self*/, PyObject* args)
{
  int pri;
  if (!PyArg_ParseTuple(args, "i", &pri))
    {
      return NULL;
    }
  
  RThread().SetPriority(static_cast<TThreadPriority>(pri));
  
  RETURN_NO_VALUE;
}

static PyObject* miso_GetProcessPriority(PyObject* /*self*/, PyObject* /*args*/)
{
  return Py_BuildValue("i", static_cast<int>(RThread().ProcessPriority()));
}

static PyObject* miso_SetProcessPriority(PyObject* /*self*/, PyObject* args)
{
  int pri;
  if (!PyArg_ParseTuple(args, "i", &pri))
    {
      return NULL;
    }
  
  RThread().SetProcessPriority(static_cast<TProcessPriority>(pri));
  
  RETURN_NO_VALUE;
}

static PyObject* miso_KillProcess(PyObject* /*self*/, PyObject* args)
{
  char* b;
  int l;
  int reason;
  if (!PyArg_ParseTuple(args, "u#i", &b, &l, &reason))
  {
    return NULL;
  }
  TPtrC processSpec((TUint16*)b, l);

  // The pattern is something like "test.exe*" or "*[12345678]*".
  TFindProcess processFinder(processSpec);

  TFullName result; // set to full process name by Next
  RProcess processHandle;
  TInt error;
  int count = 0;
  while (processFinder.Next(result) == KErrNone) 
    {
      error = processHandle.Open(processFinder, EOwnerThread);
      if (error)
	return SPyErr_SetFromSymbianOSErr(error);
      processHandle.Kill(reason); // requires PowerMgmt capability
      processHandle.Close();
      count++;
    }
  
  return Py_BuildValue("i", count);
}

// ----------------------------------------------------------------------
// heap information

static PyObject* miso_NumAllocHeapCells(PyObject* /*self*/, PyObject* /*args*/)
{
  return Py_BuildValue("i", User::CountAllocCells());
}

static PyObject* miso_NumFreeHeapCells(PyObject* /*self*/, PyObject* /*args*/)
{
  TInt freeCount;
  User::CountAllocCells(freeCount);
  return Py_BuildValue("i", freeCount);
}

static PyObject* miso_AllocHeapCellsSize(PyObject* /*self*/, PyObject* /*args*/)
{
  TInt size;
  User::AllocSize(size);
  return Py_BuildValue("i", size);
}

static PyObject* miso_HeapBiggestAvail(PyObject* /*self*/, PyObject* /*args*/)
{
  TInt biggest;
  User::Available(biggest);
  return Py_BuildValue("i", biggest);
}

static PyObject* miso_HeapTotalAvail(PyObject* /*self*/, PyObject* /*args*/)
{
  TInt biggest;
  return Py_BuildValue("i", User::Available(biggest));
}

static PyObject* miso_CheckHeap(PyObject* /*self*/, PyObject* /*args*/)
{
  User::Check();
  RETURN_NO_VALUE;
}

static PyObject* misty_CompressAllHeaps(PyObject* /*self*/, PyObject* /*args*/)
{
  TInt error = User::CompressAllHeaps();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }
  RETURN_NO_VALUE;
}

static PyObject* miso_HeapBaseAddress(PyObject* /*self*/, PyObject* /*args*/)
{
  return Py_BuildValue("i", reinterpret_cast<int>(User::Heap().Base()));
}

static PyObject* miso_AllocHeapCell(PyObject* /*self*/, PyObject* args)
{
  int size;
  if (!PyArg_ParseTuple(args, "i", &size))
  {
    return NULL;
  }

  TAny* cell = User::Alloc(size);
  if (!cell) {
    return PyErr_NoMemory();
  }

  return Py_BuildValue("i", reinterpret_cast<int>(cell));
}

static PyObject* miso_FreeHeapCell(PyObject* /*self*/, PyObject* args)
{
  int address;
  if (!PyArg_ParseTuple(args, "i", &address))
  {
    return NULL;
  }

  User::Free(reinterpret_cast<TAny*>(address));

  RETURN_NO_VALUE;
}

// ----------------------------------------------------------------------
// disks

#if 0
static PyObject* miso_DiskTotalAvail(PyObject* /*self*/, PyObject* args)
{
  int drive;
  if (!PyArg_ParseTuple(args, "i", &drive))
  {
    return NULL;
  }

  RFs fs;
  TInt error = fs.Connect();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  TVolumeInfo info;
  error = fs.Volume(info, drive);
  fs.Close();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  TInt64 freeSpace = info.iFree;
#ifdef __HAS_I64_MACROS__
  if (I64HIGH(freeSpace) != 0)
#else
  if (freeSpace.High() != 0)
#endif
  {
    return SPyErr_SetFromSymbianOSErr(KErrOverflow);
  }
  
#ifdef __HAS_I64_MACROS__
  return Py_BuildValue("i", I64LOW(freeSpace));
#else
  return Py_BuildValue("i", freeSpace.Low());
#endif
}
#endif

static PyObject* miso_GetSubstPath(PyObject* /*self*/, PyObject* args)
{
  int drive;
  if (!PyArg_ParseTuple(args, "i", &drive))
  {
    return NULL;
  }

  RFs fs;
  TInt error = fs.Connect();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  TFileName fileName;
  error = fs.Subst(fileName, drive);
  fs.Close();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  return Py_BuildValue("u#", fileName.Ptr(), fileName.Length());
}

static PyObject* miso_CreateDriveSubst(PyObject* /*self*/, PyObject* args)
{
  int drive;
  char* b;
  int l;
  if (!PyArg_ParseTuple(args, "iu#", &drive, &b, &l))
  {
    return NULL;
  }
  TPtrC fileName((TUint16*)b, l);

  RFs fs;
  TInt error = fs.Connect();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  error = fs.SetSubst(fileName, drive);
  fs.Close();
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }

  RETURN_NO_VALUE;
}

// ----------------------------------------------------------------------
// Bluetooth

static TInt GetLocalBtName(TDes& aName)
{
  TInt error = KErrNone;

  RSocketServ socketServ;
  error = socketServ.Connect();
  if (!error) {
    TProtocolName protocolName;
    // address and name queries are apparently supplied
    // by the BT stack's link manager
    _LIT(KBtLinkManager, "BTLinkManager");
    protocolName.Copy(KBtLinkManager);
    TProtocolDesc protocolDesc;
    error = socketServ.FindProtocol(protocolName, protocolDesc);
    if (!error) {
      RHostResolver hostResolver;
      error = hostResolver.Open(socketServ,
				protocolDesc.iAddrFamily,
				protocolDesc.iProtocol);
      if (!error) {
	error = hostResolver.GetHostName(aName);
	hostResolver.Close();
      }
    }
    socketServ.Close();
  }  

  return error;
}

static TInt GetLocalBtAddress(TBTDevAddr& aAddress)
{
#ifdef __HAS_BT_SUBSCRIBE__
  TPtr8 ptr(aAddress.Des());
  TInt error = RProperty::Get(KPropertyUidBluetoothCategory,
			      KPropertyKeyBluetoothLocalDeviceAddress,
			      ptr);
#else
  RSocketServ socketServ;
  TInt error = socketServ.Connect();
  if (error)
    return error;

  // this solution comes from the "bthci" Series 60 example;
  // does not work on Symbian 8-up
  RSocket socket;
  error = socket.Open(socketServ, KBTAddrFamily, KSockSeqPacket, KL2CAP);
  if (!error) {
    TPckgBuf<TBTDevAddr> btDevAddrPckg;
    TRequestStatus status;
    socket.Ioctl(KHCILocalAddressIoctl, status, &btDevAddrPckg, KSolBtHCI);
    User::WaitForRequest(status);
    error = status.Int();
    if (!error) {
      TPtrC8 src(btDevAddrPckg().Des());
      TPtr8 dest(aAddress.Des());
      dest.Copy(src);
    }

    socket.Close();
  }
  
  socketServ.Close();
#endif

  return error;
}

static TInt GetLocalBtAddressAsString(TDes8& aString)
{
  TBTDevAddr addr;
  TInt error = GetLocalBtAddress(addr);
  if (!error) {
    // GetReadable() does not produce a "standard" result,
    // so have to construct a string manually.
    aString.Zero();
    _LIT8(KColon, ":");
    for (TInt i=0; i<6; i++) {
      const TUint8& val = addr[i];
      aString.AppendNumFixedWidthUC(val, EHex, 2);
      if (i < 5)
	aString.Append(KColon);
    }
  }
  return error;
}

static PyObject* miso_LocalBtName(PyObject* /*self*/, PyObject* /*args*/)
{
  TBTDeviceName text;
  TInt error = GetLocalBtName(text);
  if (error)
    {
      return SPyErr_SetFromSymbianOSErr(error);
    }
  return Py_BuildValue("u#", text.Ptr(), text.Length());
}

static PyObject* miso_LocalBtAddress(PyObject* /*self*/, PyObject* /*args*/)
{
  TBuf8<6*2+5> text;
  TInt error = GetLocalBtAddressAsString(text);
  if (error)
    {
      return SPyErr_SetFromSymbianOSErr(error);
    }

  return Py_BuildValue("s#", text.Ptr(), text.Length());
}

// ----------------------------------------------------------------------
// Hardware Abstraction Layer (HAL)

static PyObject* miso_SetHalAttr(PyObject* /*self*/, PyObject* args)
{
  int id, value;
  if (!PyArg_ParseTuple(args, "ii", &id, &value))
  {
    return NULL;
  }

  TInt error = HAL::Set(static_cast<HALData::TAttribute>(id), value);
  if (error)
    {
      return SPyErr_SetFromSymbianOSErr(error);
    }

  RETURN_NO_VALUE;
}

static PyObject* miso_GetHalAttr(PyObject* /*self*/, PyObject* args)
{
  int id;
  if (!PyArg_ParseTuple(args, "i", &id))
  {
    return NULL;
  }

  TInt value;
  TInt error = HAL::Get(static_cast<HALData::TAttribute>(id), value);
  if (error)
    {
      return SPyErr_SetFromSymbianOSErr(error);
    }
  return Py_BuildValue("i", value);
}

// ----------------------------------------------------------------------
// time

static PyObject* miso_TickCount(PyObject* /*self*/, PyObject* /*args*/)
{
  return Py_BuildValue("i", static_cast<TInt>(User::TickCount()));
}

static PyObject* miso_ResetInactivityTime(PyObject* /*self*/, PyObject* /*args*/)
{
  User::ResetInactivityTime();
  RETURN_NO_VALUE;
}

// ----------------------------------------------------------------------
// reset, reboot...

/* UserSvr::ResetMachine would not appear to work (at least not reliably) on S60 2nd FP2 at least, so we are instead using a non-public API. Credit for this goes to Mika Raento, who adviced about the existence of the API, and provided information about it. See also http://meaning.3xi.org/download/source/meaning-20060905.tar.gz. */

// Merkitys codebase has info on other values, but this one is enough for us,
// unless we want to support factory resets or somesuch things.
enum TSWStartupReason
  {
    ESWNone = 100
  };

// link against sysutil.lib
class SysStartup 
{
public:
  IMPORT_C static TInt ShutdownAndRestart(const TUid& aSource, TSWStartupReason aReason);
};

static PyObject* miso_RestartPhone(PyObject* /*self*/, PyObject* /*args*/)
{
  // xxx assuming the undocumented return value is an error code
  TInt error;
  // xxx any reason why we would need to provide a proper UID?
  const TUid uid = {0};
  error = SysStartup::ShutdownAndRestart(uid, ESWNone);
  if (error)
  {
    return SPyErr_SetFromSymbianOSErr(error);
  }
  
  RETURN_NO_VALUE;
}

// ----------------------------------------------------------------------
// vibration

#if defined(__HAS_VIBRACTRL__) || defined(__HAS_HWRMVIBRA__)
#if defined(__HAS_HWRMVIBRA__)
void DoVibrateL(TInt aDuration, TInt aIntensity);
#else
void DoVibrateL(TUint16 aDuration, TInt aIntensity);
#endif

static PyObject* miso_Vibrate(PyObject* /*self*/, PyObject* args)
{
  TInt durationInt, intensity;
  if (!PyArg_ParseTuple(args, "ii", &durationInt, &intensity))
  {
    return NULL;
  }

#if !defined(__HAS_HWRMVIBRA__)
  TUint16 duration = static_cast<TUint16>(durationInt);
  if ((duration <= 0) ||
      ((intensity < -100 || intensity > 100)))
    // CHWRMVibra::StartVibraL leaves with the same error code if either
    // of the arguments is out of range, so this behavior is consistent
    // with that.
    return SPyErr_SetFromSymbianOSErr(KErrArgument);
#endif

  TInt error;
  Py_BEGIN_ALLOW_THREADS;
#if defined(__HAS_HWRMVIBRA__)
  TRAP(error, DoVibrateL(durationInt, intensity));
#else
  TRAP(error, DoVibrateL(duration, intensity));
#endif
  Py_END_ALLOW_THREADS;
  if (error)
      return SPyErr_SetFromSymbianOSErr(error);

  RETURN_NO_VALUE;
}
#else
static PyObject* miso_Vibrate(PyObject* /*self*/, PyObject* /*args*/)
{
  return SPyErr_SetFromSymbianOSErr(KErrNotSupported);
}
#endif

// -------------------------------------------------------------------
// The APIs of the other source files...

extern PyObject* new_MisoFsNotifyChange(PyObject*, PyObject*);
extern TInt def_MisoFsNotifyChange();

// -------------------------------------------------------------------
// Python module...

static const PyMethodDef Miso_methods[] =
  {
    {"get_thread_priority", (PyCFunction)miso_GetThreadPriority, METH_NOARGS},
    {"set_thread_priority", (PyCFunction)miso_SetThreadPriority, METH_VARARGS},
    {"get_process_priority", (PyCFunction)miso_GetProcessPriority, METH_NOARGS},
    {"set_process_priority", (PyCFunction)miso_SetProcessPriority, METH_VARARGS},
    {"kill_process", (PyCFunction)miso_KillProcess, METH_VARARGS},
    {"num_alloc_heap_cells", (PyCFunction)miso_NumAllocHeapCells, METH_NOARGS},
    {"num_free_heap_cells", (PyCFunction)miso_NumFreeHeapCells, METH_NOARGS},
    {"alloc_heap_cells_size", (PyCFunction)miso_AllocHeapCellsSize, METH_NOARGS},
    {"heap_biggest_avail", (PyCFunction)miso_HeapBiggestAvail, METH_NOARGS},
    {"heap_total_avail", (PyCFunction)miso_HeapTotalAvail, METH_NOARGS},
    {"check_heap", (PyCFunction)miso_CheckHeap, METH_NOARGS},
    {"compress_all_heaps", (PyCFunction)misty_CompressAllHeaps, METH_NOARGS},
    {"heap_base_address", (PyCFunction)miso_HeapBaseAddress, METH_NOARGS},
    {"alloc_heap_cell", (PyCFunction)miso_AllocHeapCell, METH_VARARGS},
    {"free_heap_cell", (PyCFunction)miso_FreeHeapCell, METH_VARARGS},
    {"get_subst_path", (PyCFunction)miso_GetSubstPath, METH_VARARGS},
    {"create_drive_subst", (PyCFunction)miso_CreateDriveSubst, METH_VARARGS},
    {"local_bt_name", (PyCFunction)miso_LocalBtName, METH_NOARGS},
    {"local_bt_address", (PyCFunction)miso_LocalBtAddress, METH_NOARGS},
    {"set_hal_attr", (PyCFunction)miso_SetHalAttr, METH_VARARGS},
    {"get_hal_attr", (PyCFunction)miso_GetHalAttr, METH_VARARGS},
    {"tick_count", (PyCFunction)miso_TickCount, METH_NOARGS},
    {"reset_inactivity_time", (PyCFunction)miso_ResetInactivityTime, METH_NOARGS},
    {"restart_phone", (PyCFunction)miso_RestartPhone, METH_NOARGS},
    {"vibrate", (PyCFunction)miso_Vibrate, METH_VARARGS},
    {"FsNotifyChange", (PyCFunction)new_MisoFsNotifyChange, METH_NOARGS},
    {NULL, NULL} /* sentinel */
  };

DL_EXPORT(void) initmiso()
{
  PyObject* module = 
    Py_InitModule("miso", METHOD_TABLE(Miso));
  if (!module)
    return;

  if (def_MisoFsNotifyChange() < 0) return;
}

// -------------------------------------------------------------------
// DLL entry point...

#ifndef EKA2
GLDEF_C TInt E32Dll(TDllReason)
{
  return KErrNone;
}
#endif /*EKA2*/
