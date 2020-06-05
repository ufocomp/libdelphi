/*++

Library name:

  libdelphi

Module Name:

  Process.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_PROCESS_HPP
#define DELPHI_PROCESS_HPP
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Process {

        struct CExecuteContext {
            char         *path;
            const char   *name;
            char *const  *argv;
            char *const  *envp;
        };
        //--------------------------------------------------------------------------------------------------------------

        enum CProcessType {
            ptMain, ptSingle, ptMaster, ptSignaller, ptNewBinary, ptWorker, ptHelper, ptCustom
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomProcess --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCustomProcess: public CObject {
        private:

            CCustomProcess *m_pParent;

            pid_t m_Pid;

            CProcessType m_Type;

            int m_Status;

            CString m_ProcessName;

            struct pwd_t {
                const char *username;
                const char *groupname;
                uid_t uid;
                gid_t gid;

                pwd_t () {
                    username = nullptr;
                    groupname = nullptr;
                    uid = (uid_t) -1;
                    gid = (gid_t) -1;
                }

            } m_pwd;

            bool m_respawn: true;
            bool m_just_spawn: true;
            bool m_detached: true;
            bool m_exiting: true;
            bool m_exited: true;

            pid_t m_NewBinary;

            bool m_fDaemonized;

            Pointer m_pData;

            void SetPwd() const;

        protected:

            void SetPid(pid_t Value) { m_Pid = Value; };

            void SetProcessName(LPCTSTR Value) { m_ProcessName = Value; };
            void SetData(Pointer Value) { m_pData = Value; };
            void SetStatus(int Value) { m_Status = Value; };

            void SetRespawn(bool Value) { m_respawn = Value; };
            void SetJustSpawn(bool Value) { m_just_spawn = Value; };
            void SetDetached(bool Value) { m_detached = Value; };
            void SetExited(bool Value) { m_exited = Value; };
            void SetExiting(bool Value) { m_exiting = Value; };

        public:

            CCustomProcess(): CCustomProcess(nullptr, ptMain, "main") {

            };

            explicit CCustomProcess(CProcessType AType, LPCTSTR AName): CCustomProcess(nullptr, AType, AName) {

            };

            explicit CCustomProcess(CCustomProcess *AParent, CProcessType AType, LPCTSTR AName);

            ~CCustomProcess() override = default;

            virtual void BeforeRun() abstract;
            virtual void AfterRun() abstract;

            virtual void Run() abstract;

            virtual void Terminate();

            virtual void Assign(CCustomProcess *AProcess);

            static void ExecuteProcess(CExecuteContext *AContext);

            CProcessType Type() const { return m_Type; };

            pid_t Pid() const { return m_Pid; };
            void Pid(pid_t Value) { SetPid(Value); };

            pid_t ParentId();

            CCustomProcess *Parent() { return m_pParent; };

            CString& ProcessName() { return m_ProcessName; };
            const CString& ProcessName() const { return m_ProcessName; };
            void ProcessName(LPCTSTR Value) { SetProcessName(Value); };

            LPCTSTR GetProcessName() const { return m_ProcessName.c_str(); };

            Pointer Data() { return m_pData; };
            void Data(Pointer Value) { SetData(Value); };

            pid_t NewBinary() const { return m_NewBinary; };
            void NewBinary(pid_t Value) { m_NewBinary = Value; };

            bool Daemonized() const { return m_fDaemonized; };
            void Daemonized(bool Value) { m_fDaemonized = Value; };

            bool Respawn() const { return m_respawn; };
            void Respawn(bool Value) { SetRespawn(Value); };

            bool JustSpawn() const { return m_just_spawn; };
            void JustSpawn(bool Value) { SetJustSpawn(Value); };

            bool Detached() const { return m_detached; };
            void Detached(bool Value) { SetDetached(Value); };

            bool Exited() const { return m_exited; };
            void Exited(bool Value) { SetExited(Value); };

            bool Exiting() const { return m_exiting; };
            void Exiting(bool Value) { SetExiting(Value); };

            int Status() const { return m_Status; };
            void Status(int Value) { SetStatus(Value); };

            void SetUser(const char *AUserName, const char *AGroupName);
            void SetUser(const CString& UserName, const CString& GroupName);

            static void SetLimitNoFile(uint32_t value);

        }; // class CCustomProcess

    }
}

using namespace Delphi::Process;
}
#endif // DELPHI_PROCESS_HPP