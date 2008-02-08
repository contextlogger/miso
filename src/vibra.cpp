//
// vibra.cpp
//
// Copyright 2005-2007 Helsinki Institute for Information Technology (HIIT)
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

#ifdef __DO_LOGGING__
#include <flogger.h> // flogger.lib required, Symbian 7.0-up
_LIT(KLogFileDir, "miso");
_LIT(KLogFileName, "vibra.txt");
#endif

#if defined(__HAS_HWRMVIBRA__)
#include <e32base.h> // required due to hwrmvibra.h error
#include <hwrmvibra.h>
#else
#include <vibractrl.h> // CVibraControl
#endif

#if defined(__HAS_HWRMVIBRA__)
typedef TInt DurType;
typedef MHWRMVibraObserver ObserverType;
typedef CHWRMVibra ControllerType;
#else
typedef TUint16 DurType;
typedef MVibraControlObserver ObserverType;
typedef CVibraControl ControllerType;
#endif

class CMisoVibra : public CBase, public ObserverType
{
public:
  static CMisoVibra* NewLC();
  void ConstructL();
  ~CMisoVibra();
private:
  CMisoVibra();
public:
  void VibrateL(DurType aDuration, TInt aIntensity); // synchronous
private:
  CActiveSchedulerWait* iLoop;
  ControllerType* iVibraControl;
  TRequestStatus iStatus;
#if defined(__HAS_HWRMVIBRA__)
private: // MHWRMVibraObserver
  void VibraModeChanged(CHWRMVibra::TVibraModeState /*aStatus*/) {}
  void VibraStatusChanged(CHWRMVibra::TVibraStatus aStatus);
#else
private: // MVibraControlObserver
  void VibraModeStatus(CVibraControl::TVibraModeState /*aStatus*/) {}
  void VibraRequestStatus(CVibraControl::TVibraRequestStatus aStatus);
#endif
};

CMisoVibra* CMisoVibra::NewLC()
{
  CMisoVibra* misoVibra = new (ELeave) CMisoVibra;
  CleanupStack::PushL(misoVibra);
  misoVibra->ConstructL();
  return misoVibra;
}

CMisoVibra::CMisoVibra()
{
  // nothing
}

void CMisoVibra::ConstructL()
{
  iLoop = new (ELeave) CActiveSchedulerWait();
#if defined(__HAS_HWRMVIBRA__)
  iVibraControl = ControllerType::NewL(this);
#else
  iVibraControl = VibraFactory::NewL(this);
#endif
}

CMisoVibra::~CMisoVibra()
{
  delete iVibraControl;
  delete iLoop;
}

#if defined(__HAS_HWRMVIBRA__)
void CMisoVibra::VibrateL(TInt aDuration, TInt aIntensity)
{
  iVibraControl->StartVibraL(aDuration, aIntensity);
  iLoop->Start();
  User::LeaveIfError(iStatus.Int());
}
#endif

#if !defined(__HAS_HWRMVIBRA__)
void CMisoVibra::VibrateL(TUint16 aDuration, TInt aIntensity)
{
#ifdef __DO_LOGGING__
  RFileLogger::Write(KLogFileDir, KLogFileName,
		     EFileLoggingModeAppend,
		     _L("making vibra request"));
#endif

  // it seems that this function in some cases calls
  // VibraRequestStatus, which is rather questionable
  // behavior; oh well, I guess we will just have
  // to keep track of this behavior
  iStatus = KRequestPending;
  iVibraControl->StartVibraL(aDuration, aIntensity);

  if (iStatus == KRequestPending) {
#ifdef __DO_LOGGING__
    RFileLogger::Write(KLogFileDir, KLogFileName,
		       EFileLoggingModeAppend,
		       _L("starting loop"));
#endif
    iLoop->Start();
#ifdef __DO_LOGGING__
    RFileLogger::Write(KLogFileDir, KLogFileName,
		       EFileLoggingModeAppend,
		       _L("loop finished"));
#endif
  }

#ifdef __DO_LOGGING__
  RFileLogger::WriteFormat(KLogFileDir, KLogFileName,
			   EFileLoggingModeAppend,
			   _L("VibrateL done with %d"), iStatus.Int());
#endif
  User::LeaveIfError(iStatus.Int());
}
#endif // !defined(__HAS_HWRMVIBRA__)

#if defined(__HAS_HWRMVIBRA__)
void CMisoVibra::VibraStatusChanged(CHWRMVibra::TVibraStatus aStatus)
{
  if (aStatus == CHWRMVibra::EVibraStatusOn)
    return; // wait until vibra stops

  if (aStatus == CHWRMVibra::EVibraStatusUnknown)
    iStatus = KErrGeneral;
  else if (aStatus == CHWRMVibra::EVibraStatusNotAllowed)
    iStatus = KErrAccessDenied;
  else if (aStatus == CHWRMVibra::EVibraStatusStopped)
    iStatus = KErrNone;
  else
    iStatus = KErrUnknown;

  if (iLoop->IsStarted()) {
    iLoop->AsyncStop();
  }
}
#endif

#if !defined(__HAS_HWRMVIBRA__)
void CMisoVibra::VibraRequestStatus(CVibraControl::TVibraRequestStatus aStatus)
{
#ifdef __DO_LOGGING__
  RFileLogger::WriteFormat(KLogFileDir, KLogFileName,
			   EFileLoggingModeAppend,
			   _L("request status %d"), aStatus);
#endif

  if (aStatus == CVibraControl::EVibraRequestOK)
    return; // wait until vibra stops

  TInt error = KErrUnknown;
  if (aStatus == CVibraControl::EVibraRequestFail) {
    error = KErrGeneral;
  } else if (aStatus == CVibraControl::EVibraRequestNotAllowed) {
    error = KErrAccessDenied;
  } else if (aStatus == CVibraControl::EVibraRequestStopped) {
    error = KErrNone;
  } else if (aStatus == CVibraControl::EVibraRequestUnableToStop) {
    // signal completion anyway
  } else if (aStatus == CVibraControl::EVibraRequestUnknown) {
    // signal completion anyway
  }

  iStatus = error;

  if (iLoop->IsStarted()) {
#ifdef __DO_LOGGING__
    RFileLogger::Write(KLogFileDir, KLogFileName,
		       EFileLoggingModeAppend,
		       _L("stopping loop"));
#endif
    iLoop->AsyncStop();
  }
}
#endif

void DoVibrateL(DurType aDuration, TInt aIntensity)
{
  CMisoVibra* vibra = CMisoVibra::NewLC();
  vibra->VibrateL(aDuration, aIntensity);
  CleanupStack::PopAndDestroy();
}
