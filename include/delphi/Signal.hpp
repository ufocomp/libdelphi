/*++

Library name:

  libdelphi

Module Name:

  Signal.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_SIGNAL_HPP
#define DELPHI_SIGNAL_HPP

//extern sig_atomic_t    sig_fatal;
//----------------------------------------------------------------------------------------------------------------------

#define INVALID_PID (-1)
//----------------------------------------------------------------------------------------------------------------------

#define signal_value_helper(n)      SIG##n
#define signal_value(n)             signal_value_helper(n)

#define sig_value_helper(n)         #n
#define sig_value(n)                sig_value_helper(n)
//----------------------------------------------------------------------------------------------------------------------

#define SIG_SHUTDOWN_SIGNAL         QUIT
#define SIG_TERMINATE_SIGNAL        TERM
#define SIG_NOACCEPT_SIGNAL         WINCH
#define SIG_RECONFIGURE_SIGNAL      HUP

#if (LINUX_THREADS)
#define SIG_REOPEN_SIGNAL           INFO
#define SIG_CHANGEBIN_SIGNAL        XCPU
#else
#define SIG_REOPEN_SIGNAL           USR1
#define SIG_CHANGEBIN_SIGNAL        USR2
#endif
//----------------------------------------------------------------------------------------------------------------------

typedef void (* CSignalHandler) (int signo, siginfo_t *siginfo, void *ucontext);
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Signal {

        //--------------------------------------------------------------------------------------------------------------

        //-- CSignal ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSignal: public CCollectionItem {
        private:

            int             m_SigNo;

            LPCTSTR         m_Code;
            LPCTSTR         m_Name;

            CSignalHandler  m_Handler;

        protected:

            void SetCode(LPCTSTR Value);
            void SetName(LPCTSTR Value);

        public:

            CSignal(CCollection *ACollection, int ASigNo);

            int SigNo() const { return m_SigNo; };

            LPCTSTR Code() const { return m_Code; };
            void Code(LPCTSTR Value) { SetCode(Value); };

            LPCTSTR Name() const { return m_Name; };
            void Name(LPCTSTR Value) { SetName(Value); };

            CSignalHandler &Handler() { return m_Handler; }
            const CSignalHandler &Handler() const { return m_Handler; }
            void Handler(CSignalHandler && Value) { m_Handler = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSignals --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSignals: public CCollection {
            typedef CCollection inherited;

        private:

            mutable CHandle m_Handle;

            sigset_t m_SigSet {};

            CSignal *Get(int Index) const;
            void Put(int Index, CSignal *Signal);

            void InitHandle(int iFlags) const;

        protected:

            CHandle GetHandle() const;

        public:

            CSignals(): CCollection(this), m_Handle(INVALID_HANDLE_VALUE) {};

            CSignal *AddSignal(int ASigNo, LPCTSTR ACode, LPCTSTR AName, CSignalHandler && AHandler);

            ~CSignals() override;

            int IndexOfSigNo(int SigNo) const;

            void InitSignals();

            CHandle Handle() const { return GetHandle(); };

            sigset_t &SignalSet() { return m_SigSet; };
            const sigset_t &SignalSet() const { return m_SigSet; };

            void SigProcMask(int How, sigset_t *OldSet = nullptr) const;

            int SignalsCount() const { return Count(); };

            CSignal *Signals(int Index) const { return Get(Index); };

            void Strings(int Index, CSignal *Value) { return Put(Index, Value); };

            CSignal *operator[] (int Index) const override { return Signals(Index); }

            virtual void SignalHandler(int signo, siginfo_t *siginfo, void *ucontext) abstract;

        };

    }
}

using namespace Delphi::Signal;
}

#endif //DELPHI_SIGNAL_HPP
