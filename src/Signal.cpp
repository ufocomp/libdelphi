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

        void CSignals::InitSignals() {

            CSignal *Signal;
            struct sigaction sa = {};

            for (int I = 0; I < Count(); ++I) {
                ZeroMemory(&sa, sizeof(struct sigaction));

                Signal = Get(I);

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
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        sigset_t *CSignals::SigAddSet(sigset_t *set) {
            if (Assigned(set)) {
                sigemptyset(set);
                for (int I = 0; I < Count(); ++I) {
                    if (sigaddset(set, Get(I)->SigNo()) == -1) {
                        throw EOSError(errno, _T("call sigaddset() failed"));
                    }
                }
            }

            return set;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSignals::SigProcMask(int How, const sigset_t *Set, sigset_t *OldSet) {
            if (Assigned(Set)) {
                if (sigprocmask(How, Set, OldSet) == -1) {
                    throw EOSError(errno, _T("call sigprocmask() failed"));
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSignals::IndexOfSigNo(int SigNo) {

            for (int I = 0; I < Count(); ++I) {
                if (Get(I)->SigNo() == SigNo)
                    return I;
            }

            return -1;
        }

    }
}
}
