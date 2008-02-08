//
// Copyright 2005 Helsinki Institute for Information Technology (HIIT)
// and the authors.  All rights reserved.
//
// Authors: Tero Hasu <tero.hasu@hut.fi>
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

#include <f32file.h>

// -------------------------------------------------------------------
// panics...

enum TMisoFsNotifyChangePanicCodes
{
  EPanicRequestAlreadyPending = 0,
  EPanicExceptionInCallback = 1,
  EPanicSessionAlreadyClosed = 2
};

static void Panic(TInt aReason)
{
  _LIT(KPanicCat, "MisoFsNotify");
  User::Panic(KPanicCat, aReason);
}

// -------------------------------------------------------------------
// C++ class...

class CMisoFsNotifyChange : public CActive
{
public:
  static CMisoFsNotifyChange* NewLC();
  static CMisoFsNotifyChange* NewL();
  ~CMisoFsNotifyChange();
private:
  CMisoFsNotifyChange();
  void ConstructL();

private: // CActive
  void RunL();
  void RunError();
  void DoCancel();

public:
  void NotifyChangeL(TNotifyType aType,
		     PyObject* aCallback,
		     const TDesC* const aPathName);
  void Free();
private:
  RFs iFs;
  PyObject* iCallback;
  HBufC* iPathName;
};

CMisoFsNotifyChange* CMisoFsNotifyChange::NewLC()
{
  CMisoFsNotifyChange* object = new (ELeave) CMisoFsNotifyChange();
  CleanupStack::PushL(object);
  object->ConstructL();
  return object;
}

CMisoFsNotifyChange* CMisoFsNotifyChange::NewL()
{
  CMisoFsNotifyChange* object = CMisoFsNotifyChange::NewLC();
  CleanupStack::Pop();
  return object;
}

CMisoFsNotifyChange::~CMisoFsNotifyChange()
{
  Cancel();

  if (iFs.Handle() != 0)
    iFs.Close();

  Free();
}

CMisoFsNotifyChange::CMisoFsNotifyChange() : CActive(EPriorityStandard)
{
  CActiveScheduler::Add(this);
}

void CMisoFsNotifyChange::ConstructL()
{
  User::LeaveIfError(iFs.Connect());
}

void CMisoFsNotifyChange::RunL()
{
  TInt error = iStatus.Int();
  PyEval_RestoreThread(PYTHON_TLS->thread_state);
  // if this fails, an exception should get set,
  // and probably thrown later in some other context;
  // note that it seems that the parameter must be
  // a tuple (as always, such details are not
  // easy to find from the Python API reference)
  PyObject* arg = Py_BuildValue("(i)", error);
  if (arg)
    {
      PyObject* result = PyObject_CallObject(iCallback, arg);
      Py_DECREF(arg);
      Py_XDECREF(result);
      if (!result)
	{
	  // Callbacks are not supposed to throw exceptions.
	  // Make sure that the error gets noticed.
	  PyErr_Clear();
	  Panic(EPanicExceptionInCallback);
	}
    }
  PyEval_SaveThread();
}

void CMisoFsNotifyChange::RunError()
{
  // RunL never leaves, so nothing to do here
}

void CMisoFsNotifyChange::DoCancel()
{
  // using the synchronous version here;
  // we have no choice here, really
  iFs.NotifyChangeCancel();
}

void CMisoFsNotifyChange::
NotifyChangeL(TNotifyType aType,
	      PyObject* aCallback, 
	      const TDesC* const aPathName)
{
  if (IsActive())
    Panic(EPanicRequestAlreadyPending);

  Free();

  iCallback = aCallback;
  Py_INCREF(aCallback);
  
  if (aPathName) {
    iPathName = aPathName->AllocL();
    iFs.NotifyChange(aType, iStatus, *iPathName);
  } else {
    iFs.NotifyChange(aType, iStatus);
  }
  SetActive();
}

void CMisoFsNotifyChange::Free()
{
  if (iCallback)
    {
      Py_DECREF(iCallback);
      iCallback = NULL;
    }
  if (iPathName)
    {
      delete iPathName;
      iPathName = NULL;
    }
}

// -------------------------------------------------------------------
// Python class...

typedef struct
{
  PyObject_VAR_HEAD;
  CMisoFsNotifyChange* iCppObject;
} obj_MisoFsNotifyChange;

static PyObject* miso_fnc_NotifyChange(obj_MisoFsNotifyChange* self, PyObject* args)
{
  if (!self->iCppObject)
    Panic(EPanicSessionAlreadyClosed);

  int type;
  PyObject* cb;
  char* b = NULL;
  int l;
  if (!PyArg_ParseTuple(args, "iO|u#", &type, &cb, &b, &l))
    {
      return NULL;
    }
  if (!PyCallable_Check(cb))
    {
      PyErr_SetString(PyExc_TypeError, "parameter must be callable");
      return NULL;
    }

  TInt error;
  if (b) {
    TPtrC fileName((TUint16*)b, l);
    TRAP(error, self->iCppObject->NotifyChangeL
	 (static_cast<TNotifyType>(type), cb, &fileName));
  } else {
    TRAP(error, self->iCppObject->NotifyChangeL
	 (static_cast<TNotifyType>(type), cb, NULL));
  }
  
  RETURN_ERROR_OR_PYNONE(error);
}

static PyObject* miso_fnc_Cancel(obj_MisoFsNotifyChange* self, PyObject* /*args*/)
{
  if (!self->iCppObject)
    Panic(EPanicSessionAlreadyClosed);

  self->iCppObject->Cancel();
  RETURN_NO_VALUE;
}

static PyObject* miso_fnc_Close(obj_MisoFsNotifyChange* self, PyObject* /*args*/)
{
  delete self->iCppObject;
  self->iCppObject = NULL;
  RETURN_NO_VALUE;
}

static const PyMethodDef FsNotifyChange_methods[] =
  {
    {"notify_change", (PyCFunction)miso_fnc_NotifyChange, METH_VARARGS, NULL},
    {"cancel", (PyCFunction)miso_fnc_Cancel, METH_NOARGS, NULL},
    {"close", (PyCFunction)miso_fnc_Close, METH_NOARGS, NULL},
    {NULL, NULL} /* sentinel */
  };

static void del_MisoFsNotifyChange(obj_MisoFsNotifyChange *self)
{
  delete self->iCppObject;
  self->iCppObject = NULL;
  PyObject_Del(self);
}

static PyObject *getattr_MisoFsNotifyChange(obj_MisoFsNotifyChange *self, char *name)
{
  return Py_FindMethod(METHOD_TABLE(FsNotifyChange), reinterpret_cast<PyObject*>(self), name);
}

const static PyTypeObject tmpl_MisoFsNotifyChange =
  {
    PyObject_HEAD_INIT(NULL)
    0, /*ob_size*/
    "miso.FsNotifyChange", /*tp_name*/
    sizeof(obj_MisoFsNotifyChange), /*tp_basicsize*/
    0, /*tp_itemsize*/
    /* methods */
    (destructor)del_MisoFsNotifyChange, /*tp_dealloc*/
    0, /*tp_print*/
    (getattrfunc)getattr_MisoFsNotifyChange, /*tp_getattr*/
    0, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr*/
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0 /*tp_hash*/
  };

TInt def_MisoFsNotifyChange()
{
  return ConstructType(&tmpl_MisoFsNotifyChange, "miso.FsNotifyChange");
}

PyObject* new_MisoFsNotifyChange(PyObject* /*self*/, PyObject* /*args*/)
{
  PyTypeObject* typeObject = reinterpret_cast<PyTypeObject*>(SPyGetGlobalString("miso.FsNotifyChange"));
  obj_MisoFsNotifyChange* self = PyObject_New(obj_MisoFsNotifyChange, typeObject);
  if (self == NULL)
      return NULL;
  self->iCppObject = NULL;

  TRAPD(error,
    self->iCppObject = CMisoFsNotifyChange::NewL();
  );
  if (error) {
    PyObject_Del(self);
    return SPyErr_SetFromSymbianOSErr(error);
  }

  return reinterpret_cast<PyObject*>(self);
}
