/*++

Library name:

  libdelphi

Module Name:

  Signal.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Signal.hpp"
#include "sys/signalfd.h"

extern "C++" {

namespace Delphi {

    namespace Signal {

        //--------------------------------------------------------------------------------------------------------------

        //-- CSignal ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSignal::CSignal(CCollection *ACollection, int ASigNo): CCollectionItem(ACollection), m_SigNo(ASigNo) {
            m_Code = nullptr;
            m_Name = nullptr;
            m_Handler = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignal::SetCode(LPCTSTR Value) {
            if (m_Code != Value)
                m_Code = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignal::SetName(LPCTSTR Value) {
            if (m_Name != Value)
                m_Name = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignal::SetHandler(CSignalHandler Value) {
            if (m_Handler != Value)
                m_Handler = Value;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSignals --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSignals::~CSignals() {
            if (m_Handle != INVALID_HANDLE_VALUE) {
                ::close(m_Handle);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::AddSignal(int ASigNo, LPCTSTR ACode, LPCTSTR AName, CSignalHandler AHandler) {
            auto Signal = new CSignal(this, ASigNo);

            Signal->Code(ACode);
            Signal->Name(AName);
            Signal->Handler(AHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSignal *CSignals::Get(int Index) const {
            if ((Index < 0) || (Index >= Count()))
                throw ExceptionFrm(SListIndexError, Index);

            return (CSignal *) GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::Put(int Index, CSignal *Signal) {
            if ((Index < 0) || (Index >= Count()))
                throw ExceptionFrm(SListIndexError, Index);
            SetItem(Index, Signal);
        }
        //--------------------------------------------------------------------------------------------------------------

        CHandle CSignals::GetHandle() const {
            if (m_Handle == INVALID_HANDLE_VALUE) {
                InitHandle(SFD_NONBLOCK);
            }
            return m_Handle;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::InitHandle(int iFlags) const {
            m_Handle = signalfd(-1, &m_SigSet, iFlags);
            if (m_Handle == INVALID_HANDLE_VALUE) {
                throw EOSError(errno, _T("call signalfd() failed"));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::InitSignals() {
            struct sigaction sa = {};

            sigemptyset(&m_SigSet);

            for (int i = 0; i < Count(); ++i) {
                ZeroMemory(&sa, sizeof(struct sigaction));

                auto Signal = Get(i);

                if (Signal->Handler()) {
                    sa.sa_sigaction = Signal->Handler();
                    sa.sa_flags = SA_SIGINFO;
                } else {
                    sa.sa_handler = SIG_IGN;
                }

                if (Signal->SigNo() && !Signal->Name()) {
                    Signal->Name(strsignal(Signal->SigNo()));
                }

                sigemptyset(&sa.sa_mask);
                if (sigaction(Signal->SigNo(), &sa, nullptr) == -1) {
                    throw EOSError(errno, "sigaction(%s) failed", Signal->Code());
                }

                if (sigaddset(&m_SigSet, Signal->SigNo()) == -1) {
                    throw EOSError(errno, _T("call sigaddset() failed"));
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::SigProcMask(int How, sigset_t *OldSet) {
            if (sigprocmask(How, &m_SigSet, OldSet) == -1) {
                throw EOSError(errno, _T("call sigprocmask() failed"));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSignals::IndexOfSigNo(int SigNo) {

            for (int i = 0; i < Count(); ++i) {
                if (Get(i)->SigNo() == SigNo)
                    return i;
            }

            return -1;
        }

    }
}
}
