/*++

Library name:

  libdelphi

Module Name:

  Application.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_APPLICATION_HPP
#define DELPHI_APPLICATION_HPP

extern "C++" {

namespace Delphi {

    namespace Application {

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomApplication ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCustomApplication: public CObject {
        private:

            int                 m_argc;
            CStringList         m_argv;

            CString             m_cmdline;

            CString             m_name;
            CString             m_description;
            CString             m_version;
            CString             m_title;

            CString             m_header;

        protected:

            int                 m_exitcode;

            char              **m_os_argv;
            char              **m_os_environ;
            char               *m_os_argv_last;
            char               *m_environ;

            void Initialize();
            void SetEnviron();

            void SetHeader(LPCTSTR Value);

        public:

            CCustomApplication(int argc, char *const *argv);

            ~CCustomApplication() override;

            char *const *Environ() { return &m_environ; };

            int ExitCode() const { return m_exitcode; };
            void ExitCode(int Status) { m_exitcode = Status; };

            const CStringList &argv() { return m_argv; };

            int argc() const { return m_argc; };

            char *const *os_argv() { return m_os_argv; };

            const CString& CmdLine() { return m_cmdline; };

            CString& Name() { return m_name; };
            const CString& Name() const { return m_name; };

            CString& Description() { return m_description; };
            const CString& Description() const { return m_description; };

            CString& Version() { return m_version; };
            const CString& Version() const { return m_version; };

            CString& Title() { return m_title; };
            const CString& Title() const { return m_title; };

            CString& Header() { return m_header; };

            // Replace command name
            void Header(const CString& Value) { SetHeader(Value.c_str()); };
            void Header(LPCTSTR Value) { SetHeader(Value); };

        }; // class CCustomApplication

    }
}

using namespace Delphi::Application;
}

#endif //DELPHI_APPLICATION_HPP
