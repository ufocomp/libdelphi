/*++

Library name:

  libdelphi

Module Name:

  Process.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Process.hpp"

extern "C++" {

namespace Delphi {

    namespace Process {

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomProcess --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCustomProcess::CCustomProcess(CCustomProcess *AParent, CProcessType AType, LPCTSTR AName): CObject(),
                m_Type(AType), m_pParent(AParent), m_ProcessName(AName) {

            m_Pid = MainThreadID;

            m_fDaemonized = false;
            m_NewBinary = 0;

            m_Status = 0;

            m_respawn = false;
            m_just_spawn = false;
            m_detached = false;
            m_exiting = false;
            m_exited = false;

            m_pData = nullptr;

            m_pwd.uid = -1;
            m_pwd.gid = -1;

            m_pwd.username = nullptr;
            m_pwd.groupname = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::Assign(CCustomProcess *AProcess) {
            m_fDaemonized = AProcess->Daemonized();
            m_NewBinary = AProcess->NewBinary();
        }
        //--------------------------------------------------------------------------------------------------------------

        pid_t CCustomProcess::ParentId() {
            if (Assigned(m_pParent))
                return m_pParent->Pid();
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::Terminate() {
            kill(m_Pid, signal_value(SIG_TERMINATE_SIGNAL));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::ExecuteProcess(CExecuteContext *AContext) {
            if (execve(AContext->path, AContext->argv, AContext->envp) == -1) {
                throw EOSError(errno, _T("execve() failed while executing %s \"%s\""), AContext->name, AContext->path);
            }

            exit(1);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::SetPwd() const {
            if (geteuid() == 0) {
                if (setgid(m_pwd.gid) == -1) {
                    throw EOSError(errno, "setgid(%d) failed.", m_pwd.gid);
                }

                if (initgroups(m_pwd.username, m_pwd.gid) == -1) {
                    throw EOSError(errno, "initgroups(%s, %d) failed.", m_pwd.username, m_pwd.gid);
                }

                if (setuid(m_pwd.uid) == -1) {
                    throw EOSError(errno, "setuid(%d) failed.", m_pwd.username, m_pwd.gid);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::SetUser(const char *AUserName, const char *AGroupName) {
            if (m_pwd.uid == (uid_t) -1 && geteuid() == 0) {

                struct group   *grp;
                struct passwd  *pwd;

                errno = 0;
                pwd = getpwnam(AUserName);
                if (pwd == nullptr) {
                    throw EOSError(errno, "getpwnam(\"%s\") failed.", AUserName);
                }

                errno = 0;
                grp = getgrnam(AGroupName);
                if (grp == nullptr) {
                    throw EOSError(errno, "getgrnam(\"%s\") failed.", AGroupName);
                }

                m_pwd.username = AUserName;
                m_pwd.uid = pwd->pw_uid;

                m_pwd.groupname = AGroupName;
                m_pwd.gid = grp->gr_gid;

                SetPwd();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::SetUser(const CString &UserName, const CString &GroupName) {
            SetUser(UserName.c_str(), GroupName.c_str());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCustomProcess::SetLimitNoFile(uint32_t value) {
            if (value != static_cast<uint32_t>(-1)) {
                struct rlimit rlmt = { 0, 0 };

                rlmt.rlim_cur = (rlim_t) value;
                rlmt.rlim_max = (rlim_t) value;

                if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
                    throw EOSError(errno, "setrlimit(RLIMIT_NOFILE, %i) failed.", value);
                }
            }
        }
    }
}
}
