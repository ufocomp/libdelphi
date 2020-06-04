/*++

Library name:

  libdelphi

Module Name:

  Application.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Application.hpp"

extern "C++" {

namespace Delphi {

    namespace Application {

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomApplication ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCustomApplication::CCustomApplication(int argc, char *const *argv): CObject() {

            m_exitcode = 0;

            m_argc = argc;
            m_os_argv = (char **) argv;
            m_os_environ = environ;

            m_environ = nullptr;
            m_os_argv_last = nullptr;

            Initialize();
            SetEnviron();
        }
        //--------------------------------------------------------------------------------------------------------------

        CCustomApplication::~CCustomApplication() {
            //delete [] m_environ; //!!! don`t free m_environ
            m_environ = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomApplication::Initialize() {

            for (int i = 0; i < m_argc; ++i) {
                m_argv.Add(m_os_argv[i]);
            }

            m_cmdline = m_argv[0];

            for (int i = 1; i < m_argc; ++i) {
                m_cmdline += " ";
                m_cmdline += m_argv[i];
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomApplication::SetEnviron() {

            int     i;
            char   *tmp;
            size_t  size = 0;

            for (i = 0; environ[i]; ++i) {
                size += strlen(environ[i]) + 1;
            }

            if (m_environ == nullptr)
                m_environ = new char[size];

            ZeroMemory(m_environ, size);
            tmp = m_environ;

            m_os_argv_last = m_os_argv[0];

            for (i = 0; m_os_argv[i]; ++i) {
                if (m_os_argv_last == m_os_argv[i]) {
                    m_os_argv_last = m_os_argv[i] + strlen(m_os_argv[i]) + 1;
                }
            }

            for (i = 0; environ[i]; ++i) {
                if (m_os_argv_last == environ[i]) {

                    size = strlen(environ[i]) + 1;
                    m_os_argv_last = environ[i] + size;

                    ld_cpystrn(tmp, environ[i], size);
                    environ[i] = tmp;
                    tmp += size;
                }
            }

            m_os_argv_last--;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomApplication::SetHeader(LPCTSTR Value) {

            char        *p;

            m_os_argv[1] = nullptr;

            p = ld_cpystrn(m_os_argv[0], (char *) Value, m_os_argv_last - m_os_argv[0]);

            if (m_os_argv_last - p) {
                memset(p, '\0', m_os_argv_last - p);
            }

            m_header = Value;
        }

    }
}
}
