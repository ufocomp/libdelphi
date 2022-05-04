/*++

Library name:

  libdelphi

Module Name:

  Sockets.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_SOCKETS_HPP
#define DELPHI_SOCKETS_HPP
//----------------------------------------------------------------------------------------------------------------------

#pragma once
//----------------------------------------------------------------------------------------------------------------------

#include <sys/socket.h>
#include <sys/epoll.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef WITH_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifndef INT_LF
#define INT_LF 10
#endif

#ifndef INT_CR
#define INT_CR 13
#endif

#define EOLW L"\r\n\0"
#define EOLA "\r\n\0"

#ifdef  _UNICODE
#    define EOL    EOLW
#else
#    define EOL         EOLA
#endif
//----------------------------------------------------------------------------------------------------------------------

#define NI_MAXHOST      1025
#define NI_MAXIP        25
//----------------------------------------------------------------------------------------------------------------------

#define TimeoutDefault  (-1)
#define TimeoutInfinite (-2)
//----------------------------------------------------------------------------------------------------------------------

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    (-1)
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (-1)
#endif
//----------------------------------------------------------------------------------------------------------------------

#define WS_FIN                  0x80u
#define WS_MASK                 0x80u

#define WS_OPCODE_CONTINUATION  0x00u
#define WS_OPCODE_TEXT          0x01u
#define WS_OPCODE_BINARY        0x02u
#define WS_OPCODE_CLOSE         0x08u
#define WS_OPCODE_PING          0x09u
#define WS_OPCODE_PONG          0x0Au

#define WS_PAYLOAD_LENGTH_16    126u
#define WS_PAYLOAD_LENGTH_64    127u
//----------------------------------------------------------------------------------------------------------------------

typedef struct sockaddr SOCKADDR, *LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN, *LPSOCKADDRIN;
//----------------------------------------------------------------------------------------------------------------------

typedef struct servent SERVENT, *LPSERVENT;
//----------------------------------------------------------------------------------------------------------------------

namespace Delphi {

    namespace Socket {

        typedef SOCKET CSocket;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCommand;

        class LIB_DELPHI CTCPServer;

        class LIB_DELPHI CPeerThread;

        class LIB_DELPHI CSocketThread;

        class LIB_DELPHI CTCPConnection;

        class LIB_DELPHI CTCPServerConnection;

        class LIB_DELPHI CListenerThread;
        //--------------------------------------------------------------------------------------------------------------

        enum COverlappedType {
            otRecv, otSend
        };
        //--------------------------------------------------------------------------------------------------------------

        enum CErrorGroup {
            egSystem, egSocket
        };

        class LIB_DELPHI CStack;
        //--------------------------------------------------------------------------------------------------------------

        extern LIB_DELPHI CStack *GStack;
        //--------------------------------------------------------------------------------------------------------------

        LIB_DELPHI char *inet_ntoa2(in_addr_t ulAddr);

        LIB_DELPHI bool IsCurrentThread(CThread *AThread);

        LIB_DELPHI void SplitColumns(LPCTSTR AData, size_t ASize, CStringList *AStrings, char ADelim);
        LIB_DELPHI void SplitColumns(const CString &Data, CStringList &Strings, char Delimiter = ';');

        //--------------------------------------------------------------------------------------------------------------

        //-- CStack ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

#ifdef WITH_SSL
        enum CSSLMethod { sslNotUsed = -1, sslClient = 0, sslServer };
        //--------------------------------------------------------------------------------------------------------------
#endif
        class LIB_DELPHI CStack {
        private:

            TCHAR m_szBuffer[1024] = {0};

            int m_LastError;
#ifdef WITH_SSL
            unsigned long m_SSLError;
#endif
        public:

            CStack();

            virtual ~CStack() = default;

            inline static class CStack *CreateSocket() { return GStack = new CStack; };

            inline static void DeleteSocket() { delete GStack; };

            CSocket CreateSocketHandle(int ASocketType, int AProtocol = IPPROTO_IP, unsigned int AFlag = 0);
#ifdef WITH_SSL
            static void SSLInit();
            static void SSLFinalize();
            static SSL *SSLNew(bool IsSever = false);
            static void SSLFree(SSL *ssl);
            static CSocket SSLGetSocket(SSL *ssl);
            static void SSLAllocate(SSL *ssl, CSocket ASocket);

            static int SSLClear(SSL *ssl);
            static int SSLShutdown(SSL *ssl);
            static int SSLConnect(SSL *ssl);

            static ssize_t SSLRecv(SSL *ssl, void *ABuffer, size_t ABufferLength);
            static ssize_t SSLSend(SSL *ssl, void *ABuffer, size_t ABufferLength);

            virtual unsigned long GetSSLError();

            bool CheckForSSLError(ssize_t AResult);
            bool CheckForSSLError(ssize_t AResult, unsigned long const AIgnore[], int ACount);

            static ssize_t RecvPacket(SSL *ssl, void *ABuffer, size_t ABufferSize);
            static ssize_t SendPacket(SSL * ssl, void *ABuffer, size_t ABufferSize);

            void RaiseSSLError();

            int SSLError() const { return m_SSLError; }
#endif
            int LastError() const { return m_LastError; }

            static void RaiseSocketError(int AErr);

            virtual bool CheckForSocketError(ssize_t AResult, CErrorGroup AErrGroup);
            virtual bool CheckForSocketError(ssize_t AResult, int const AIgnore[], int ACount, CErrorGroup AErrGroup);

            virtual CSocket Accept(CSocket ASocket, char *VIP, size_t ASize, unsigned short *VPort, unsigned int AFlags);

            virtual int Bind(CSocket ASocket, sa_family_t AFamily, LPCSTR AIP, unsigned short APort);

            virtual int CloseSocket(CSocket ASocket);

            virtual int Connect(CSocket ASocket, sa_family_t AFamily, LPCSTR AIP, unsigned short APort);

            virtual int Listen(CSocket ASocket, int ABackLog);

            virtual uint16_t HToNS(uint16_t AHostShort);

            virtual uint16_t NToHS(uint16_t ANetShort);

            virtual uint32_t HToNL(uint32_t AHostLong);

            virtual uint32_t NToHL(uint32_t ANetLong);

            virtual ssize_t Recv(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags);

            virtual ssize_t RecvFrom(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags, char *VIP,
                    size_t ASize, unsigned short *VPort, LPSOCKADDRIN AFrom, socklen_t *AFromLen);

            virtual CSocket Select(CList *ARead, CList *AWrite, CList *AErrors, int ATimeout);

            virtual ssize_t Send(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags);

            virtual ssize_t SendTo(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags, LPCSTR AIP,
                    unsigned short APort);

            virtual int SetSockOpt(CSocket ASocket, int ALevel, int AOptName, const void *AOptVal, socklen_t AOptLen);

            virtual CSocket Socket(int ADomain, int AType, int AProtocol, unsigned int AFlag);

            virtual int Shutdown(CSocket ASocket, int AHow);

            bool GetHostName(char *AName, size_t ASize);

            virtual int GetSocketError(CSocket ASocket);

            virtual int GetLastError();

            virtual void GetPeerName(CSocket ASocket, sa_family_t *AFamily, char *AIP, size_t ASize,
                    unsigned short *APort);

            virtual void GetSockName(CSocket ASocket, sa_family_t *AFamily, char *AIP, size_t ASize,
                    unsigned short *APort);

            virtual void GetHostByAddr(SOCKADDR_IN *AAddr, char *VHost, size_t ASize);

            virtual void GetHostByName(const char *AName, char *VHost, size_t ASize);

            virtual void GetIPByName(const char *AName, char *VIP, size_t ASize);

            virtual void GetServByPort(SOCKADDR_IN *AName, char *VPort, size_t ASize);

            virtual int GetSockOpt(CSocket ASocket, int Alevel, int AOptname, void *AOptval, socklen_t *AOptlen);

            virtual void SetNonBloking(CSocket ASocket);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketComponent ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CWorkMode {
            wmRead, wmWrite
        };
        //--------------------------------------------------------------------------------------------------------------

        typedef struct tagWORK_INFO {
            ssize_t Current;
            ssize_t Max;
            int Level;
        } CWorkInfo;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketComponent {
        protected:

            CWorkInfo m_WorkInfos[sizeof(CWorkInfo)];

        public:

            CSocketComponent();

            ~CSocketComponent();

            void BeginWork(CWorkMode AWorkMode, ssize_t ASize = 0);

            void DoWork(CWorkMode AWorkMode, ssize_t ACount);

            void EndWork(CWorkMode AWorkMode);

        }; // CSocketComponent

        //--------------------------------------------------------------------------------------------------------------

        //-- CSimpleBuffer ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define GRecvBufferSizeDefault  (64 * 1024)
        #define GSendBufferSizeDefault  (64 * 1024)
        #define MaxLineLengthDefault    (32 * 1024)
        #define InBufCacheSizeDefault   (32 * 1024) //CManagedBuffer.PackReadSize
        //--------------------------------------------------------------------------------------------------------------

        enum CMaxLineAction {
            maException, maSplit
        };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSimpleBuffer : public CMemoryStream {
            typedef CMemoryStream inherited;

        public:

            CSimpleBuffer();

            virtual size_t Extract(void *ABuffer, size_t AByteCount);

            virtual void Remove(size_t AByteCount);

        }; // CSimpleBuffer

        //--------------------------------------------------------------------------------------------------------------

        //-- CManagedBuffer --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CManagedBuffer: public CSimpleBuffer {
            typedef CSimpleBuffer inherited;

        protected:

            size_t m_PackReadSize;
            size_t m_ReadSize;

            void SetPackReadSize(size_t Value);

        public:

            CManagedBuffer();

            void Clear() override;

            size_t Extract(void *ABuffer, size_t AByteCount) override;

            Pointer Memory() const override;

            void PackBuffer();

            void Remove(size_t AByteCount) override;

            size_t Seek(size_t Offset, unsigned short Origin);

            size_t PackReadSize() const { return m_PackReadSize; }

            void PackReadSize(size_t Value) { SetPackReadSize(Value); }

            size_t ReadSize() const { return m_ReadSize; }

        }; // CManagedBuffer

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketHandle ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketHandle: public CSocketComponent, public CCollectionItem {
        private:

            CSocket m_Handle;
#ifdef WITH_SSL
            SSL *m_pSSL;

            CSSLMethod m_SSLMethod;
#endif
            int m_SocketType;

            bool m_HandleAllocated;
            bool m_Nonblocking;

            char m_IP[NI_MAXIP];
            char m_PeerIP[NI_MAXIP];

            unsigned short m_Port;
            unsigned short m_PeerPort;

            unsigned short m_ClientPortMin;
            unsigned short m_ClientPortMax;

            bool TryBind();

            bool BindPortReserved();

        protected:

            SOCKADDR_IN m_From;
            socklen_t m_FromLen;

            virtual void SetDefaultPort(unsigned short Value);

            void SetIP(LPCSTR Value);

            void SetPeerIP(LPCSTR Value);

            static int GetLastError();

        public:

            explicit CSocketHandle(CCollection *ACollection);

            ~CSocketHandle() override;
#ifdef WITH_SSL
            void AllocateSSL();
            void CloseSSL();
            void ShutdownSSL();
            void ClearSSL();
            void ConnectSSL();

            CSSLMethod SSLMethod() const { return m_SSLMethod; }
            void SSLMethod(CSSLMethod Value) { m_SSLMethod = Value; }

            bool UsedSSL() const { return m_SSLMethod != sslNotUsed; }
#endif
            bool Accept(CSocket ASocket, unsigned int AFlag);

            void AllocateSocket(int ASocketType, int AProtocol, unsigned int AFlag);

            void Bind();

            void CloseSocket(bool AResetLocal = true);

            static bool GetHostName(char *AName, size_t ASize);

            static void GetHostByName(const char *AName, char *VHost, size_t ASize);

            static void GetIPByName(const char *AName, char *VIP, size_t ASize);

            static bool GetHostIP(char *AIP, size_t ASize);

            int Connect(sa_family_t AFamily, LPCSTR AHost, unsigned short APort);

            bool CheckConnection();

            int GetSocketError() const;

            void GetSockOpt(int ALevel, int AOptName, void *AOptVal, socklen_t AOptLen) const;

            void Listen(int anQueueCount) const;

            ssize_t Recv(void *ABuffer, size_t ABufferSize, int AFlags = 0) const;

            ssize_t RecvFrom(void *ABuffer, size_t ABufferSize, int AFlags = 0);

            void Reset(bool AResetLocal = true);

            ssize_t Send(void *ABuffer, size_t ABufferSize, int AFlags = 0);

            ssize_t SendTo(LPCSTR AIP, unsigned short APort, void *ABuffer, size_t ABufferSize, int AFlags = 0) const;

            void SetPeer(LPCSTR asIP, unsigned short anPort);

            void SetSockOpt(int ALevel, int AOptName, const void *AOptVal, socklen_t AOptLen) const;

            static bool Select(CSocket ASocket, int ATimeOut);

            static bool SelectWrite(CSocket ASocket, int ATimeOut);

            void UpdateBindingLocal();

            void UpdateBindingPeer();

            void UpdateFromPeer();

            int SocketType() const { return m_SocketType; }

            bool HandleAllocated() const { return m_HandleAllocated; }

            bool Nonblocking() const { return m_Nonblocking; }

            CSocket Handle() const { return m_Handle; }

            char *IP() { return m_IP; }
            void IP(LPCSTR Value) { SetIP(Value); }

            unsigned short Port() const { return m_Port; }
            void Port(unsigned short Value) { m_Port = Value; }

            char *PeerIP() { return m_PeerIP; }
            void PeerIP(LPCSTR Value) { SetPeerIP(Value); }

            unsigned short PeerPort() const { return m_PeerPort; }

            unsigned short ClientPortMin() const { return m_ClientPortMin; }
            void ClientPortMin(unsigned short Value) { m_ClientPortMin = Value; }

            unsigned short ClientPortMax() const { return m_ClientPortMax; }
            void ClientPortMax(unsigned short Value) { m_ClientPortMax = Value; }

            static int LastError() { return GetLastError(); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketHandles --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketHandles : public CCollection {
            typedef CCollection inherited;

        protected:

            CString m_DefaultIP;

            unsigned short m_DefaultPort;

            CSocketHandle *GetItem(int Index) const override;
            void SetItem(int Index, CSocketHandle *Value);

        public:

            CSocketHandles();

#ifdef WITH_SSL
            CSocketHandle *Add(CSSLMethod ASSLMethod = sslNotUsed);
#else
            CSocketHandle *Add();
#endif
            CSocketHandle *Insert(int Index);

            CSocketHandle *BindingByHandle(CSocket AHandle);

            CSocketHandle *Handles(int Index) const { return GetItem(Index); }
            void Handles(int Index, CSocketHandle *Value) { SetItem(Index, Value); }

            CString& DefaultIP() { return m_DefaultIP; }
            const CString& DefaultIP() const { return m_DefaultIP; }

            unsigned short DefaultPort() const { return m_DefaultPort; }
            void DefaultPort(unsigned short Value) { m_DefaultPort = Value; }

            CSocketHandle *operator[] (int Index) const override { return Handles(Index); };
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CIOHandler ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CIOHandlerSocket;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CIOHandler : public CSocketComponent {
            typedef CSocketComponent inherited;

        public:

            CIOHandler() : CSocketComponent() {};

            virtual ~CIOHandler() { CIOHandler::Close(); };

            virtual void AfterAccept() {};
#ifdef WITH_SSL
            virtual void Open(CSSLMethod SSLMethod) abstract;

            virtual CSSLMethod SSLMethod() const abstract;

            virtual bool UsedSSL() const abstract;
#else
            virtual void Open() abstract;
#endif
            virtual void Close() {};

            virtual bool Connected() abstract;

            virtual ssize_t Recv(void *ABuf, size_t ALen) abstract;

            virtual ssize_t Send(void *ABuf, size_t ALen) abstract;

        }; // CIOHandler

        //--------------------------------------------------------------------------------------------------------------

        //-- CIOHandlerSocket ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CIOHandlerSocket: public CIOHandler {
            typedef CIOHandler inherited;

        protected:

            CSocketHandle *m_pBinding;

        public:

            CIOHandlerSocket();

            ~CIOHandlerSocket() override;
#ifdef WITH_SSL
            void Open(CSSLMethod SSLMethod) override;

            bool UsedSSL() const override;

            CSSLMethod SSLMethod() const override;
#else
            void Open() override;
#endif
            void Close() override;

            bool Connected() override;

            ssize_t Recv(void *ABuf, size_t ALen) override;

            ssize_t Send(void *ABuf, size_t ALen) override;

            CSocketHandle *Binding() { return m_pBinding; }

        }; // CIOHandlerSocket

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollManager;
        class LIB_DELPHI CPollEventHandler;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollConnection: public CSocketComponent, public CCollectionItem {
        private:

            CPollConnection *m_pBinding;
            CPollEventHandler *m_pEventHandler;

            void SetTimeOut(CDateTime Value);

        protected:

            CDateTime m_TimeOut;

            bool m_AutoFree;
            bool m_CloseConnection;

            void ClosePoll();

            void SetBinding(CPollConnection *AValue);
            void SetEventHandler(CPollEventHandler *AValue);

        public:

            explicit CPollConnection(CPollManager *AManager);

            ~CPollConnection() override;

            virtual void Close() abstract;

            CPollConnection *Binding() const { return m_pBinding; };
            void Binding(CPollConnection *Value) { SetBinding(Value); };

            CPollEventHandler *EventHandler() const { return m_pEventHandler; }
            void EventHandler(CPollEventHandler *Value) { SetEventHandler(Value); }

            CDateTime TimeOut() const { return m_TimeOut; };
            void TimeOut(CDateTime Value) { SetTimeOut(Value); };

            void UpdateTimeOut(CDateTime DateTime, double Interval, double Period = MSecsPerDay);

            bool AutoFree() const { return m_AutoFree; }
            void AutoFree(bool Value) { m_AutoFree = Value; }

            bool CloseConnection() const { return m_CloseConnection; }
            void CloseConnection(bool Value) { m_CloseConnection = Value; }

        }; // CPollConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollManager ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollManager: public CCollection {
        public:

            explicit CPollManager(): CCollection(this) {

            };

            explicit CPollManager(CCollection *AOwner): CCollection(AOwner) {

            };

            ~CPollManager() override = default;

            void CloseAllConnection() { Clear(); };

            CPollConnection *operator[] (int Index) const override { return dynamic_cast<CPollConnection *> (Items(Index)); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPConnection --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CTCPServer;
        //--------------------------------------------------------------------------------------------------------------

        enum CConnectionStatus { csConnected = 0,
            csWaitRequest, csRequestOk, csRequestReady, csRequestSent, csRequestError,
            csWaitReply, csReplyOk, csReplyReady, csReplySent, csReplyError
        };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CTCPConnection: public CPollConnection {
            friend CListenerThread;

        private:

            CIOHandler *m_pIOHandler;
            CIOHandlerSocket *m_pSocket;

            CSimpleBuffer *m_pWriteBuffer;
            CSimpleBuffer *m_pRecvBuffer;

            CManagedBuffer *m_pInputBuffer;
            CSimpleBuffer *m_pOutputBuffer;

            bool m_ReadLnSplit;
            bool m_ReadLnTimedOut;
            bool m_ClosedGracefully;
            bool m_OEM;

            size_t m_SendBufferSize;
            size_t m_RecvBufferSize;

            ssize_t m_WriteBufferThreshold;

            size_t m_MaxLineLength;
            int m_ReadTimeOut;

            CMaxLineAction m_MaxLineAction;
            CNotifyEvent m_OnDisconnected;

            bool m_FreeIOHandler;

            void SetIOHandler(CIOHandler *AValue, bool AFree);
            void FreeIOHandler();

        protected:

            CDateTime m_Clock;

            void DoDisconnected();

        public:

            explicit CTCPConnection(CPollManager *AManager);

            ~CTCPConnection() override;

            bool CheckForDisconnect(bool ARaiseExceptionIfDisconnected = true);

            CIOHandlerSocket *Socket() { return m_pSocket; }

            CIOHandler *IOHandler() { return m_pIOHandler; }
            void IOHandler(CIOHandler *Value, bool AFree = true) { SetIOHandler(Value, AFree); }

            bool ClosedGracefully() const { return m_ClosedGracefully; }

            bool OEM() const { return m_OEM; }
            void OEM(bool Value) { m_OEM = Value; }

            CDateTime Clock() const { return m_Clock; };
            void UpdateClock() { m_Clock = Now(); };

            void Close() override;

            bool Connected();
            void Disconnect();

            void DisconnectSocket();

            void OpenWriteBuffer(ssize_t AThreshhold = -1);

            void CloseWriteBuffer();

            void FlushWriteBuffer(ssize_t AByteCount = -1);

            void ClearWriteBuffer();

            void FreeWriteBuffer();

            void FlushOutputBuffer(ssize_t AByteCount = -1);

            void CheckWriteResult(ssize_t AByteCount);

            void WriteBuffer(void *ABuffer, size_t AByteCount, bool AWriteNow = false);

            ssize_t WriteBufferAsync(void *ABuffer, size_t AByteCount);

            bool WriteAsync(ssize_t AByteCount = -1);

            void WriteInteger(int AValue, bool AConvert = true);

            virtual void WriteLn(LPCSTR Format, ...);

            virtual void Write(LPCSTR Format, ...);

            void WriteStream(CStream *AStream, bool AAll = true, bool AWriteByteCount = false, size_t ASize = 0);

            void WriteStrings(CStrings *AValue, bool AWriteLinesCount = false);

            virtual ssize_t CheckReadStack(ssize_t AByteCount);

            ssize_t ReadFromStack(bool ARaiseExceptionIfDisconnected = true, bool ARaiseExceptionOnTimeout = true);

            ssize_t ReadAsync(bool ARaiseExceptionIfDisconnected = true);

            void ReadBuffer(void *ABuffer, size_t AByteCount);

            int ReadInteger(bool AConvert = true);

            size_t ReadLn(char *AStr, char ATerminator = INT_LF, int ATimeout = TimeoutDefault, int AMaxLineLength = -1);

            size_t ReadLnWait(char *AStr, size_t ABytes, int AFailCount = MaxInt);

            void ReadString(char *AStr, size_t ABytes);

            void ReadStrings(CStrings *AValue, int AReadLinesCount = -1);

            bool ReadLnTimedOut() const { return m_ReadLnTimedOut; }

            int ReadTimeOut() const { return m_ReadTimeOut; }
            void ReadTimeOut(int Value) { m_ReadTimeOut = Value; }

            size_t RecvBufferSize() const { return m_RecvBufferSize; }
            void RecvBufferSize(size_t Value) { m_RecvBufferSize = Value; }

            size_t MaxLineLength() const { return m_MaxLineLength; }
            void MaxLineLength(size_t Value) { m_MaxLineLength = Value; }

            CMaxLineAction MaxLineAction() { return m_MaxLineAction; }
            void MaxLineAction(CMaxLineAction Value) { m_MaxLineAction = Value; }

            CManagedBuffer *InputBuffer() { return m_pInputBuffer; }
            CSimpleBuffer *OutputBuffer() { return m_pOutputBuffer; }

            const CNotifyEvent& OnDisconnected() { return m_OnDisconnected; }
            void OnDisconnected(CNotifyEvent && Value) { m_OnDisconnected = Value; }

        }; // CTCPConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketEvent ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CSocketEvent;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CSocketEvent *Sender, CTCPConnection *AConnection, LPCTSTR AFormat, va_list args)> COnSocketVerboseEvent;
        typedef std::function<bool (CTCPConnection *AConnection)> COnSocketExecuteEvent;
        typedef std::function<void (CTCPConnection *AConnection)> COnSocketConnectionEvent;
        typedef std::function<void (CSocketEvent *Sender, const Delphi::Exception::Exception &E)> COnSocketListenExceptionEvent;
        typedef std::function<void (CTCPConnection *AConnection, const Delphi::Exception::Exception &E)> COnSocketExceptionEvent;
        typedef std::function<void (CSocketEvent *Sender, const CString &Data, CTCPConnection *AConnection)> COnSocketBeforeCommandHandlerEvent;
        typedef std::function<void (CSocketEvent *Sender, const CString &Data, CTCPConnection *AConnection)> COnSocketNoCommandHandlerEvent;
        typedef std::function<void (CSocketEvent *Sender, CTCPConnection *AConnection)> COnSocketAfterCommandHandlerEvent;
        typedef std::function<void (CCommand *ACommand)> COnSocketCommandEvent;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketEvent {
            friend CPeerThread;

        protected:

            COnSocketVerboseEvent m_OnVerbose;
            COnSocketConnectionEvent m_OnAccessLog;

            COnSocketExecuteEvent m_OnExecute;

            CNotifyEvent m_OnConnected;
            CNotifyEvent m_OnDisconnected;

            COnSocketExceptionEvent m_OnException;
            COnSocketListenExceptionEvent m_OnListenException;

            COnSocketBeforeCommandHandlerEvent m_OnBeforeCommandHandler;
            COnSocketAfterCommandHandlerEvent m_OnAfterCommandHandler;
            COnSocketNoCommandHandlerEvent m_OnNoCommandHandler;

            virtual void DoVerbose(CTCPConnection *AConnection, LPCTSTR AFormat, ...);
            virtual void DoVerbose(CTCPConnection *AConnection, LPCTSTR AFormat, va_list args);

            virtual bool DoCommand(CTCPConnection *AConnection) abstract;

            virtual bool DoExecute(CTCPConnection *AConnection) abstract;

            virtual void DoAccessLog(CTCPConnection *AConnection);

            virtual void DoConnected(CObject *Sender);
            virtual void DoDisconnected(CObject *Sender);

            virtual void DoException(CTCPConnection *AConnection, const Delphi::Exception::Exception &E);
            virtual void DoListenException(const Delphi::Exception::Exception &E);

            virtual void DoBeforeCommandHandler(CTCPConnection *AConnection, const CString &Line);
            virtual void DoAfterCommandHandler(CTCPConnection *AConnection);
            virtual void DoNoCommandHandler(const CString &Data, CTCPConnection *AConnection);

        public:

            CSocketEvent();

            const COnSocketVerboseEvent &OnVerbose() { return m_OnVerbose; }
            void OnVerbose(COnSocketVerboseEvent && Value) { m_OnVerbose = Value; }

            const COnSocketExecuteEvent &OnExecute() { return m_OnExecute; }
            void OnExecute(COnSocketExecuteEvent && Value) { m_OnExecute = Value; }

            const COnSocketConnectionEvent &OnAccessLog() { return m_OnAccessLog; }
            void OnAccessLog(COnSocketConnectionEvent && Value) { m_OnAccessLog = Value; }

            const CNotifyEvent &OnConnected() { return m_OnConnected; }
            void OnConnected(CNotifyEvent && Value) { m_OnConnected = Value; }

            const CNotifyEvent &OnDisconnected() { return m_OnDisconnected; }
            void OnDisconnected(CNotifyEvent && Value) { m_OnDisconnected = Value; }

            const COnSocketExceptionEvent &OnException() { return m_OnException; }
            void OnException(COnSocketExceptionEvent && Value) { m_OnException = Value; }

            const COnSocketListenExceptionEvent &OnListenException() { return m_OnListenException; }
            void OnListenException(COnSocketListenExceptionEvent && Value) { m_OnListenException = Value; }

            COnSocketBeforeCommandHandlerEvent &OnBeforeCommandHandler() { return m_OnBeforeCommandHandler; }
            void OnBeforeCommandHandler(COnSocketBeforeCommandHandlerEvent && Value) { m_OnBeforeCommandHandler = Value; }

            COnSocketAfterCommandHandlerEvent &OnAfterCommandHandler() { return m_OnAfterCommandHandler; }
            void OnAfterCommandHandler(COnSocketAfterCommandHandlerEvent && Value) { m_OnAfterCommandHandler = Value; }

            const COnSocketNoCommandHandlerEvent &OnNoCommandHandler() { return m_OnNoCommandHandler; }
            void OnNoCommandHandler(COnSocketNoCommandHandlerEvent && Value) { m_OnNoCommandHandler = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketServer ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketServer: public CSocketComponent {
        private:

            CSocketHandles *m_pBindings;

            CString m_ServerName;

            CString m_AllowedMethods;

            bool m_FreeBindings;

            void FreeBindings();

        protected:

            CString& GetDefaultIP();
            const CString& GetDefaultIP() const;

            unsigned short GetDefaultPort() const;
            virtual void SetDefaultPort(unsigned short Value);

            CSocketHandles *GetBindings() const;
            void SetBindings(CSocketHandles *Value);

        public:

            CSocketServer();

            ~CSocketServer();

            CString& DefaultIP() { return GetDefaultIP(); }
            const CString& DefaultIP() const { return GetDefaultIP(); }

            unsigned short DefaultPort() const { return GetDefaultPort(); }
            void DefaultPort(unsigned short Value) { SetDefaultPort(Value); }

            CString& ServerName() { return m_ServerName; };
            const CString& ServerName() const { return m_ServerName; };

            CString& AllowedMethods() { return m_AllowedMethods; };
            const CString& AllowedMethods() const { return m_AllowedMethods; };

            CSocketHandles *Bindings() const { return GetBindings(); }
            void Bindings(CSocketHandles *Value) { SetBindings(Value); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketClient ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketClient: public CSocketComponent {
        protected:

            CString m_ClientName;

            CString m_Host;

            unsigned short m_Port;

        public:

            CSocketClient();

            ~CSocketClient() = default;

            CString &ClientName() { return m_ClientName; }
            const CString &ClientName() const { return m_ClientName; }

            CString &Host() { return m_Host; }
            const CString &Host() const { return m_Host; }

            unsigned short Port() const { return m_Port; }
            void Port(unsigned short Value) { m_Port = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CEventSocketServer ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEventSocketServer: public CSocketServer, public CSocketEvent {
        public:

            CEventSocketServer(): CSocketServer(), CSocketEvent() {};

            ~CEventSocketServer() = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CEventSocketClient ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEventSocketClient: public CSocketClient, public CSocketEvent  {
        public:

            CEventSocketClient(): CSocketClient(), CSocketEvent() {};

            ~CEventSocketClient() = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollSocketServer -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollSocketServer: public CEventSocketServer, public CPollManager {
        public:

            CPollSocketServer(): CEventSocketServer(), CPollManager() {};

            ~CPollSocketServer() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollSocketClient -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollSocketClient: public CEventSocketClient, public CPollManager {
        public:

            CPollSocketClient(): CEventSocketClient(), CPollManager() {};

            ~CPollSocketClient() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocket ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        struct CWebSocketFrame {

            unsigned char FIN = WS_FIN;
            unsigned char Opcode = 0xFF;
            unsigned char Mask = 0;
            unsigned char Length = 0;
            unsigned char MaskingKey[4] = { 0, 0, 0, 0 };

            void Clear() {
                FIN = WS_FIN;
                Opcode = 0xFF;
                Mask = 0;
                Length = 0;
                ::SecureZeroMemory(MaskingKey, sizeof(MaskingKey));
            }

            void SetMaskingKey(uint32_t Key) {
                Mask = WS_MASK;
                ::CopyMemory(MaskingKey, &Key, sizeof(Key));
            }

            void SetMaskingKey(unsigned char Key[4]) {
                Mask = WS_MASK;
                ::CopyMemory(MaskingKey, Key, sizeof(MaskingKey));
            }

            //unsigned long long PayloadLength = 0;
        };

        //--------------------------------------------------------------------------------------------------------------

        class CWebSocket {
        private:

            enum CWSParserState {
                frame,
                extended,
                masking_key,
                payload_start,
                payload
            } m_State = frame;

            CWebSocketFrame m_Frame;
            CMemoryStream m_Payload;

            uint64_t m_PayloadSize;

            size_t m_MaskingIndex;

            void LoadHeader(const CMemoryStream &Stream);
            void LoadExtended(const CMemoryStream &Stream);
            void LoadMaskingKey(const CMemoryStream &Stream);

            void Encode(CMemoryStream &Stream);
            void Decode(const CMemoryStream &Stream);

            void PayloadFromStream(const CMemoryStream &Stream);
            void PayloadToStream(CMemoryStream &Stream);

        public:

            CWebSocket();

            void Clear();

            CWebSocketFrame &Frame() { return m_Frame; }
            const CWebSocketFrame &Frame() const { return m_Frame; }

            CMemoryStream &Payload() { return m_Payload; };
            const CMemoryStream &Payload() const { return m_Payload; };

            CWSParserState State() { return m_State; }

            void Close(CMemoryStream &Stream);
            void Ping(CMemoryStream &Stream);
            void Pong(CMemoryStream &Stream);

            void SaveToStream(CMemoryStream &Stream);
            int LoadFromStream(const CMemoryStream &Stream);

            void SetPayload(CMemoryStream &Stream, uint32_t Key = 0);
            void SetPayload(const CString &String, uint32_t Key = 0);

            CWebSocket& operator<< (const CString &String) {
                SetPayload(String);
                return *this;
            }

            CWebSocket& operator>> (CString &String) {
                String.LoadFromStream(m_Payload);
                return *this;
            }

            CWebSocket& operator<< (CMemoryStream &Stream) {
                LoadFromStream(Stream);
                return *this;
            }

            CWebSocket& operator>> (CMemoryStream &Stream) {
                SaveToStream(Stream);
                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocketConnection --------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CHTTPProtocol { pHTTP = 0, pWebSocket };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CWebSocketConnection: public CTCPConnection {
            typedef CTCPConnection inherited;

        private:

            CWebSocket *m_WSRequest;
            CWebSocket *m_WSReply;

            CStringList m_Data;

            CNotifyEvent m_OnWaitRequest;
            CNotifyEvent m_OnWaitReply;

            CNotifyEvent m_OnRequest;
            CNotifyEvent m_OnReply;

            CNotifyEvent m_OnPing;
            CNotifyEvent m_OnPong;

        protected:

            CConnectionStatus m_ConnectionStatus;

            CHTTPProtocol m_Protocol;

            CWebSocket *GetWSRequest();
            CWebSocket *GetWSReply();

            void DoWaitRequest();
            void DoWaitReply();

            void DoRequest();
            void DoReply();

            void DoPing();
            void DoPong();

            virtual void Parse(CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute);

        public:

            explicit CWebSocketConnection(CPollManager *AManager);

            ~CWebSocketConnection() override = default;

            CHTTPProtocol Protocol() const { return m_Protocol; }

            CWebSocket *WSRequest() { return GetWSRequest(); }
            CWebSocket *WSReply() { return GetWSReply(); }

            CConnectionStatus ConnectionStatus() const { return m_ConnectionStatus; }
            void ConnectionStatus(CConnectionStatus Value) { m_ConnectionStatus = Value; }

            void SendWebSocket(bool ASendNow = false);

            void SendWebSocketPing(bool ASendNow = false);
            void SendWebSocketPong(bool ASendNow = false);
            void SendWebSocketClose(bool ASendNow = false);

            virtual void Clear();

            CStringList &Data() { return m_Data; }
            const CStringList &Data() const { return m_Data; }

            const CNotifyEvent &OnWaitRequest() { return m_OnWaitRequest; }
            void OnWaitRequest(CNotifyEvent && Value) { m_OnWaitRequest = Value; }

            const CNotifyEvent &OnWaitReply() { return m_OnWaitReply; }
            void OnWaitReply(CNotifyEvent && Value) { m_OnWaitReply = Value; }

            const CNotifyEvent &OnRequest() { return m_OnRequest; }
            void OnRequest(CNotifyEvent && Value) { m_OnRequest = Value; }

            const CNotifyEvent &OnReply() { return m_OnReply; }
            void OnReply(CNotifyEvent && Value) { m_OnReply = Value; }

            const CNotifyEvent &OnPing() { return m_OnPing; }
            void OnPing(CNotifyEvent && Value) { m_OnPing = Value; }

            const CNotifyEvent &OnPong() { return m_OnPong; }
            void OnPong(CNotifyEvent && Value) { m_OnPong = Value; }

        }; // CWebSocketConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPServerConnection --------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CAsyncServer;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CTCPServerConnection: public CWebSocketConnection {
            typedef CWebSocketConnection inherited;

        private:

            CPollSocketServer *m_pServer;

        public:

            explicit CTCPServerConnection(CPollSocketServer *AServer);

            ~CTCPServerConnection() override;

            virtual CPollSocketServer *Server() { return m_pServer; }

        }; // CTCPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPClientConnection --------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CTCPClientConnection: public CWebSocketConnection {
            typedef CWebSocketConnection inherited;

        protected:

            CPollSocketClient *m_pClient;

        public:

            explicit CTCPClientConnection(CPollSocketClient *AClient);

            ~CTCPClientConnection() override;

            virtual CPollSocketClient *Client() { return m_pClient; }

        }; // CTCPClientConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CServerIOHandler ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CServerIOHandler : public CIOHandlerSocket {
            typedef CIOHandlerSocket inherited;

        public:

            CServerIOHandler() : CIOHandlerSocket() {};

            ~CServerIOHandler() override = default;

            static CIOHandlerSocket *Accept(CSocket ASocket, int AFlags);

        }; // CServerIOHandler

        //--------------------------------------------------------------------------------------------------------------

        //-- CClientIOHandler ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CClientIOHandler : public CIOHandlerSocket {
            typedef CIOHandlerSocket inherited;
        private:

            int m_MaxTries;

            int m_Attempts;

        public:
#ifdef WITH_SSL
            CClientIOHandler(CSSLMethod AMethod = sslNotUsed);
#else
            CClientIOHandler();
#endif
            ~CClientIOHandler() override;

            bool Connect(LPCSTR AHost, unsigned short APort);

        }; // CServerIOHandler

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPServer ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CUDPServer: public CSocketServer {
            typedef CSocketServer inherited;

        private:

            bool m_Active;

        protected:

            void SetActive(bool AValue);

        public:

            CUDPServer();

            ~CUDPServer();

            bool Active() const { return m_Active; }
            void Active(bool Value) { SetActive(Value); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPClient ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CUDPClient: public CSocketClient {
            typedef CSocketHandle inherited;

        private:

            CSocketHandle *m_pSocket;

            bool m_Active;

        protected:

            virtual void SetActive(bool AValue);

        public:

            CUDPClient();

            ~CUDPClient();

            bool Active() const { return m_Active; }
            void Active(bool Value) { SetActive(Value); }

            CSocketHandle *Socket() { return m_pSocket; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketThread ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CThreadStopMode {
            smTerminate, smSuspend
        };
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CSocketThread *AThread)> COnSocketThreadNotifyEvent;
        typedef std::function<void (CSocketThread *AThread, const Delphi::Exception::Exception &E)> COnSocketThreadExceptionEvent;
        typedef std::function<void (CSocketThread *AThread, Pointer AData)> COnSocketThreadSynchronizeEvent;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSocketThread : public CThread {
            typedef CThread inherited;

        protected:

            pthread_mutex_t m_Lock {};
            CThreadStopMode m_StopMode;
            bool m_Stopped;

            COnSocketThreadExceptionEvent m_OnException;
            COnSocketThreadNotifyEvent m_OnStopped;

            virtual void AfterRun() abstract;

            virtual void AfterExecute() abstract;

            virtual void BeforeExecute() abstract;

            virtual void BeforeRun() abstract;

            virtual void Cleanup() {};

            virtual void DoException(const Delphi::Exception::Exception &E);

            virtual void DoStopped();

            void Execute() override;

            virtual void ExecuteTry();

            bool GetStopped();

            virtual void Run() abstract;

        public:

            explicit CSocketThread(bool ACreateSuspended = true);

            ~CSocketThread() override;

            virtual void Start();

            virtual void Stop();

            void Terminate() override;

            virtual void TerminateAndWaitFor();

            CThreadStopMode StopMode() { return m_StopMode; }
            void StopMode(CThreadStopMode Value) { m_StopMode = Value; }

            bool Stopped() { return GetStopped(); }

            const COnSocketThreadExceptionEvent &OnException() { return m_OnException; }
            void OnException(COnSocketThreadExceptionEvent && Value) { m_OnException = Value; }

            const COnSocketThreadNotifyEvent& OnStopped() { return m_OnStopped; }
            void OnStopped(COnSocketThreadNotifyEvent && Value) { m_OnStopped = Value; }

        }; // CSocketThread

        //--------------------------------------------------------------------------------------------------------------

        //-- CPeerThread -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPeerThread : public CSocketThread {
            typedef CSocketThread inherited;

            friend CListenerThread;

        protected:

            CTCPServerConnection *m_pConnection;

            void AfterRun() override;

            void AfterExecute() override {};

            void BeforeRun() override;

            void BeforeExecute() override {};

            void Cleanup() override;

            void Run() override;

        public:

            CTCPServerConnection *Connection() { return m_pConnection; }

        }; // CPeerThread

        //--------------------------------------------------------------------------------------------------------------

        //-- CListenerThread -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CListenerThread : public CThread {
            typedef CThread inherited;

        private:

            CTCPServer *m_pServer;
            CSocketHandle *m_pBinding;

        protected:

            void Execute() override;

        public:

            CListenerThread(CTCPServer *AServer, CSocketHandle *ABinding);

            CTCPServer *Server() { return m_pServer; }

            CSocketHandle *Binding() { return m_pBinding; }

        }; // CListenerThread

        //--------------------------------------------------------------------------------------------------------------

        //-- CThreadMgr ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CThreadMgr {
        protected:

            CThreadList *m_pActiveThreads;
            CThreadPriority m_ThreadPriority;

        public:

            CThreadMgr();

            ~CThreadMgr();

            virtual CPeerThread *GetThread();

            virtual void ReleaseThread(CPeerThread *AThread) abstract;

            virtual void TerminateThreads();

            CThreadList *ActiveThreads() { return m_pActiveThreads; }

            CThreadPriority ThreadPriority() { return m_ThreadPriority; }
            void ThreadPriority(CThreadPriority Value) { m_ThreadPriority = Value; }

        }; // CThreadMgr

        //--------------------------------------------------------------------------------------------------------------

        //-- CThreadMgrDefault -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CThreadMgrDefault : public CThreadMgr {
            typedef CThreadMgr inherited;

        public:

            CPeerThread *GetThread() override {
              CPeerThread *Result = inherited::GetThread();
              ActiveThreads()->Add(Result);
              return Result;
            };

            void ReleaseThread(CPeerThread *AThread) override {
              if (!IsCurrentThread(AThread)) {
                if (!AThread->Suspended())
                  AThread->TerminateAndWaitFor();
                FreeAndNil(AThread);
              } else {
                AThread->FreeOnTerminate(true);
                AThread->Terminate();
              };

              ActiveThreads()->Remove(AThread);
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CThreadSafeList -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CThreadSafeList : public CThreadList {
            typedef CThreadList inherited;

        public:

            bool IsCountLessThan(int AValue) {
              int Count = 0;

              CList *LList = LockList();
              try {
                Count = LList->Count();
              } catch (...) {
              }
              UnlockList();

              return (Count < AValue);
            };
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommandHandler -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCommandHandler : public CCollectionItem {
            typedef CCollectionItem inherited;

            friend CCommand;

        protected:

            char m_CmdDelimiter;

            CString m_Command;
            Pointer m_Data;

            bool m_Disconnect;
            bool m_Enabled;

            char m_ParamDelimiter;
            bool m_ParseParams;

            int m_ReplyExceptionCode;

            COnSocketCommandEvent m_OnCommand;
            COnSocketExceptionEvent m_OnException;

            virtual void DoException(CTCPConnection *AConnection, const Delphi::Exception::Exception &E);

        public:

            explicit CCommandHandler(CCollection *ACollection);

            ~CCommandHandler() override = default;

            virtual bool Check(LPCTSTR AData, size_t ALen, CTCPConnection *AConnection);
            virtual bool Check(const CString &Data, CTCPConnection *AConnection);

            Pointer Data() const { return m_Data; }
            void Data(Pointer Value) { m_Data = Value; }

            char CmdDelimiter() const { return m_CmdDelimiter; }
            void CmdDelimiter(char Value) { m_CmdDelimiter = Value; }

            CString &Command() { return m_Command; }
            const CString &Command() const { return m_Command; }

            bool Disconnect() const { return m_Disconnect; }
            void Disconnect(bool Value) { m_Disconnect = Value; }

            bool Enabled() const { return m_Enabled; }
            void Enabled(bool Value) { m_Enabled = Value; }

            char ParamDelimiter() const { return m_ParamDelimiter; }
            void ParamDelimiter(char Value) { m_ParamDelimiter = Value; }

            bool ParseParams() const { return m_ParseParams; }
            void ParseParams(bool Value) { m_ParseParams = Value; }

            int ReplyExceptionCode() const { return m_ReplyExceptionCode; }
            void ReplyExceptionCode(int Value) { m_ReplyExceptionCode = Value; }

            const COnSocketCommandEvent &OnCommand() const { return m_OnCommand; }
            void OnCommand(COnSocketCommandEvent && Value) { m_OnCommand = Value; }

            const COnSocketExceptionEvent &OnException() const { return m_OnException; }
            void OnException(COnSocketExceptionEvent && Value) { m_OnException = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommandHandlers ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCommandHandlers : public CCollection {
            typedef CCollection inherited;
        private:

            CSocketComponent *m_pOwner;

            bool m_EnabledDefault;
            bool m_ParseParamsDefault;
            bool m_DisconnectDefault;

        protected:

            CCommandHandler *GetItem(int AIndex) const override;
            void SetItem(int AIndex, const CCommandHandler *AValue);

        public:

            CCommandHandlers();

            explicit CCommandHandlers(CSocketServer *AServer);
            explicit CCommandHandlers(CSocketClient *AClient);

            CSocketServer *Server() const { return (CSocketServer *) m_pOwner; }
            CSocketClient *Client() const { return (CSocketClient *) m_pOwner; }

            CCommandHandler *Add();

            bool EnabledDefault() const { return m_EnabledDefault; }
            void EnabledDefault(bool Value) { m_EnabledDefault = Value; }

            bool ParseParamsDefault() const { return m_ParseParamsDefault; }
            void ParseParamsDefault(bool Value) { m_ParseParamsDefault = Value; }

            bool DisconnectDefault() const { return m_DisconnectDefault; }
            void DisconnectDefault(bool Value) { m_DisconnectDefault = Value; }

            CCommandHandler *Commands(int Index) const { return GetItem(Index); }
            void Commands(int Index, CCommandHandler *Value) { SetItem(Index, Value); }

            CCommandHandler *operator[] (int Index) const override { return Commands(Index); };

        }; // CCommandHandlers

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommand --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCommand: public CObject {
            friend CCommandHandler;

        protected:

            CTCPConnection *m_pConnection;
            CCommandHandler *m_pCommandHandler;

            CStringList m_Params;

            bool m_PerformReply;

            CString m_RawLine;
            CString m_UnparsedParams;

            virtual void DoCommand();

        public:

            explicit CCommand(CCommandHandler *ACommandHandler);

            ~CCommand() override = default;

            CTCPConnection *Connection() const { return m_pConnection; }

            CCommandHandler *CommandHandler() const { return m_pCommandHandler; }

            bool PerformReply() const { return m_PerformReply; }
            void PerformReply(bool Value) { m_PerformReply = Value; }

            CStringList &Params() { return m_Params; }
            const CStringList &Params() const { return m_Params; }

            CString &RawLine() { return m_RawLine; }
            const CString &RawLine() const { return m_RawLine; }

            CString &UnparsedParams() { return m_UnparsedParams; }
            const CString &UnparsedParams() const { return m_UnparsedParams; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPServer ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CTCPServer: public CPollSocketServer {
            typedef CSocketServer inherited;

            friend CPeerThread;
            friend CListenerThread;

        private:

            CThreadMgr *m_pThreadMgr;
            CServerIOHandler *m_pIOHandler;
            CThreadSafeList *m_pThreads;
            CThreadList *m_pListenerThreads;
            CCommandHandlers *m_pCommandHandlers;

            bool m_Active;

        protected:

            CThreadMgr *GetThreadMgr();

            void SetActive(bool AValue);

            virtual void SetIOHandler(CServerIOHandler *Value);

            virtual void InitializeCommandHandlers() {};

            void TerminateAllThreads();

            void TerminateListenerThreads();

            bool DoExecute(CTCPConnection *AConnection) override;

        public:

            explicit CTCPServer();

            ~CTCPServer() override;

            bool Active() const { return m_Active; }
            void Active(bool Value) { SetActive(Value); }

            int IndexOfThreadId(pid_t dwThreadId);

            CPeerThread *FindThread(pid_t  dwThreadId);

            CThreadSafeList *Threads() { return m_pThreads; }

            CServerIOHandler *IOHandler() { return m_pIOHandler; }
            void IOHandler(CServerIOHandler *Value) { SetIOHandler(Value); }

            CThreadMgr *ThreadMgr() { return GetThreadMgr(); }

            CCommandHandlers *CommandHandlers() { return m_pCommandHandlers; }
            void CommandHandlers(CCommandHandlers *Value) { m_pCommandHandlers = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollStack ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollEventHandler;
        //--------------------------------------------------------------------------------------------------------------

        typedef int CSocketPoll;
        typedef struct epoll_event CPollEvent;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollStack: public CObject {
        private:

            CSocketPoll m_Handle;

            CPollEvent *m_pEventList;

            int m_EventSize;
            int m_TimeOut;

        protected:

            void Ctl(int AOption, CPollEventHandler *AEventHandler);

            CPollEvent *GetEvent(int Index);

        public:

            CPollStack(int AEventSize = 512);

            CPollStack(const CPollStack &Source): CPollStack() {
                Assign(Source);
            };

            ~CPollStack() override;

            void Assign(const CPollStack &Source);

            CSocketPoll Create(int AFlags = 0);

            void Close();

            void Add(CPollEventHandler *AEventHandler);

            void Mod(CPollEventHandler *AEventHandler);

            void Del(CPollEventHandler *AEventHandler);

            int TimeOut() const { return m_TimeOut; };
            void TimeOut(int Value) { m_TimeOut = Value; };

            int Wait(const sigset_t *ASigMask = nullptr);

            CPollEvent *EventList() { return m_pEventList; };

            int EventSize() const { return m_EventSize; };

            CPollEvent *Events(int Index) { return GetEvent(Index); };

            CPollEvent *operator[] (int Index) { return Events(Index); };

            CPollStack& operator= (const CPollStack &Source) {
                if (this != &Source) {
                    Assign(Source);
                }
                return *this;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollEventHandler -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPollEventHandler *AHandler)> COnPollEventHandlerEvent;
        //--------------------------------------------------------------------------------------------------------------

        enum CPollEventType { etNull, etAccept, etConnect, etIO, etDelete, etTimer };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEPoll;
        class LIB_DELPHI CEPollClient;
        class LIB_DELPHI CPollEventHandlers;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollEventHandler: public CCollectionItem {
            typedef CCollectionItem inherited;

            friend CEPoll;

        private:

            CSocket m_Socket;

            uint32_t m_Events;

            TCHAR m_szTimeStamp[25] = {0};

            CDateTime m_TimeStamp;

            CPollEventType m_EventType;

            CPollConnection *m_pBinding;

            CPollEventHandlers *m_pEventHandlers;

            COnPollEventHandlerEvent m_OnTimerEvent;
            COnPollEventHandlerEvent m_OnTimeOutEvent;

            COnPollEventHandlerEvent m_OnConnectEvent;
            COnPollEventHandlerEvent m_OnReadEvent;
            COnPollEventHandlerEvent m_OnWriteEvent;
            COnPollEventHandlerEvent m_OnErrorEvent;

            void ClearBinding();
            void UpdateTimeOut();

        protected:

            void SetEventType(CPollEventType Value);
            void SetBinding(CPollConnection *Value);
            void SetTimeStamp(CDateTime Value);

            void DoTimerEvent();
            void DoTimeOutEvent();

            void DoConnectEvent();
            void DoReadEvent();
            void DoWriteEvent();
            void DoErrorEvent();

        public:

            explicit CPollEventHandler(CPollEventHandlers *AEventHandlers, CSocket ASocket);

            ~CPollEventHandler() override;

            CSocket Socket() const { return m_Socket; }

            uint32_t Events() const { return m_Events; }

            CPollConnection *Binding() const { return m_pBinding; }
            void Binding(CPollConnection *Value) { SetBinding(Value); }

            void Start(CPollEventType AEventType = etIO);
            void Stop();

            void Fault();

            bool Stopped() const { return m_EventType == etDelete; };

            CPollEventType EventType() const { return m_EventType; }
            void EventType(CPollEventType Value) { SetEventType(Value); }

            CDateTime TimeStamp() const { return m_TimeStamp; }
            void TimeStamp(CDateTime Value) { SetTimeStamp(Value); }

            LPCTSTR TimeStampStr() const { return m_szTimeStamp; }

            const COnPollEventHandlerEvent &OnTimerEvent() const { return m_OnTimerEvent; }
            void OnTimerEvent(COnPollEventHandlerEvent && Value) { m_OnTimerEvent = Value; }

            const COnPollEventHandlerEvent &OnTimeOutEvent() const { return m_OnTimeOutEvent; }
            void OnTimeOutEvent(COnPollEventHandlerEvent && Value) { m_OnTimeOutEvent = Value; }

            const COnPollEventHandlerEvent &OnConnectEvent() const { return m_OnConnectEvent; }
            void OnConnectEvent(COnPollEventHandlerEvent && Value) { m_OnConnectEvent = Value; }

            const COnPollEventHandlerEvent &OnReadEvent() const { return m_OnReadEvent; }
            void OnReadEvent(COnPollEventHandlerEvent && Value) { m_OnReadEvent = Value; }

            const COnPollEventHandlerEvent &OnWriteEvent() const { return m_OnWriteEvent; }
            void OnWriteEvent(COnPollEventHandlerEvent && Value) { m_OnWriteEvent = Value; }

            const COnPollEventHandlerEvent &OnErrorEvent() const { return m_OnErrorEvent; }
            void OnErrorEvent(COnPollEventHandlerEvent && Value) { m_OnErrorEvent = Value; }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollEventHandlers ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CAsyncServer;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CPollEventHandler *AHandler, const Delphi::Exception::Exception &E)> COnPollEventHandlerExceptionEvent;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPollEventHandlers: public CCollection {
            typedef CCollection inherited;

            friend CPollEventHandler;

        private:

            CPollStack m_PollStack;

            COnPollEventHandlerExceptionEvent m_OnException;

        protected:

            CPollEventHandler *GetItem(int AIndex) const override;
            void SetItem(int AIndex, CPollEventHandler *AValue);

            void PollAdd(CPollEventHandler *AHandler);
            void PollMod(CPollEventHandler *AHandler);
            void PollDel(CPollEventHandler *AHandler);

            void DoException(CPollEventHandler *AHandler, const Delphi::Exception::Exception &E);

        public:

            CPollEventHandlers();

            CPollStack &PollStack() { return m_PollStack; };
            const CPollStack &PollStack() const { return m_PollStack; };

            CPollStack *ptrPollStack() { return &m_PollStack; };

            CPollEventHandler *Add(CSocket ASocket);

            CPollEventHandler *FindHandlerBySocket(CSocket ASocket);

            CPollEventHandler *Handlers(int Index) const { return GetItem(Index); }
            void Handlers(int Index, CPollEventHandler *Value) { SetItem(Index, Value); }

            CPollEventHandler *operator[] (int Index) const override { return Handlers(Index); };

            const COnPollEventHandlerExceptionEvent& OnException() const { return m_OnException; }
            void OnException(COnPollEventHandlerExceptionEvent && Value) { m_OnException = Value; }

        }; // CPollEventHandlers

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollTimer -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEPollTimer : public CHandleStream, public CPollConnection {
            typedef CHandleStream inherited;

        private:

            int m_ClockId;
            int m_Flags;

            COnPollEventHandlerEvent m_OnTimer;

        protected:

            virtual void DoTimer(CPollEventHandler *AHandler);

        public:

            CEPollTimer(int AClockId, int AFlags);

            ~CEPollTimer() override;

            inline static class CEPollTimer *CreateTimer(int AClockId, int AFlags) {
                return new CEPollTimer(AClockId, AFlags);
            };

            int Handle() { return m_Handle; };
            int ClockId() const { return m_ClockId; };

            void Open();
            void Close() override;

            void SetTime(int AFlags, const struct itimerspec *AIn, struct itimerspec *AOut = nullptr);
            void GetTime(struct itimerspec *AOut);

            CPollEventHandler *AllocateTimer(CPollEventHandlers *AEventHandlers, long int Value, long int Interval = 0, int Flags = 0);
            void SetTimer(long int Value, long int Interval = 0, int Flags = 0);

            const COnPollEventHandlerEvent &OnTimer() const { return m_OnTimer; }
            void OnTimer(COnPollEventHandlerEvent && Value) { m_OnTimer = Value; }

        }; // CFileStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPoll ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEPoll {
        private:

            COnPollEventHandlerExceptionEvent m_OnEventHandlerException;

            bool m_FreeEventHandlers;

            void CreateEventHandlers();
            void FreeEventHandlers();

            void CheckTimeOut(CPollEventHandler *AHandler, CDateTime DateTime);

        protected:

            CPollEventHandlers *m_pEventHandlers;

            void SetEventHandlers(CPollEventHandlers *Value);

            virtual void PackEventHandlers(CDateTime DateTime);

            virtual void DoTimeOut(CPollEventHandler *AHandler) abstract;
            virtual void DoAccept(CPollEventHandler *AHandler) abstract;
            virtual void DoConnect(CPollEventHandler *AHandler) abstract;
            virtual void DoRead(CPollEventHandler *AHandler) abstract;
            virtual void DoWrite(CPollEventHandler *AHandler) abstract;
            virtual void DoError(CPollEventHandler *AHandler) abstract;

            void DoEventHandlersException(CPollEventHandler *AHandler, const Delphi::Exception::Exception &E);

        public:

            CEPoll();

            ~CEPoll();

            void Wait(const sigset_t *ASigMask = nullptr);

            CPollEventHandlers *EventHandlers() const { return m_pEventHandlers; }
            void EventHandlers(CPollEventHandlers *Value) { SetEventHandlers(Value); }

            void AllocateEventHandlers(const CEPoll &EPoll) { SetEventHandlers(EPoll.EventHandlers()); }
            void AllocateEventHandlers(CEPoll *EPoll) { SetEventHandlers(EPoll->EventHandlers()); }

            bool ExternalEventHandlers() const { return !m_FreeEventHandlers; };

            const COnPollEventHandlerExceptionEvent &OnEventHandlerException() const { return m_OnEventHandlerException; }
            void OnEventHandlerException(COnPollEventHandlerExceptionEvent && Value) { m_OnEventHandlerException = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollServer ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEPollServer: public CPollSocketServer, public CEPoll {
        protected:

            void DoTimeOut(CPollEventHandler *AHandler) override {};
            void DoAccept(CPollEventHandler *AHandler) override {};
            void DoConnect(CPollEventHandler *AHandler) override {};
            void DoError(CPollEventHandler *AHandler) override {};
            bool DoExecute(CTCPConnection *AConnection) override;

        public:

            CEPollServer();

            ~CEPollServer() override = default;

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollClient ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CEPollClient: public CPollSocketClient, public CEPoll {
        protected:

            void DoTimeOut(CPollEventHandler *AHandler) override;
            void DoAccept(CPollEventHandler *AHandler) override {};
            void DoRead(CPollEventHandler *AHandler) override;
            void DoWrite(CPollEventHandler *AHandler) override;
            void DoError(CPollEventHandler *AHandler) override {};
            bool DoExecute(CTCPConnection *AConnection) override;

        public:

            CEPollClient();

            ~CEPollClient() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CAsyncServer ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CActiveLevel { alShutDown, alBinding, alActive };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CAsyncServer: public CEPollServer {
            typedef CEPollServer inherited;

        private:

            CCommandHandlers *m_pCommandHandlers;

            CStringList m_Data;

            bool m_FreeIOHandler;

            void SetIOHandler(CServerIOHandler *Value);

        protected:

            CActiveLevel m_ActiveLevel;

            CServerIOHandler *m_pIOHandler;

            void FreeIOHandler();

            virtual void InitializeCommandHandlers();
            virtual void InitializeBindings() abstract;

            virtual void SetActiveLevel(CActiveLevel AValue) abstract;

            bool DoCommand(CTCPConnection *AConnection) override;

        public:

            CAsyncServer();

            ~CAsyncServer() override;

            CActiveLevel ActiveLevel() const { return m_ActiveLevel; }
            void ActiveLevel(CActiveLevel Value) { SetActiveLevel(Value); }

            CServerIOHandler *IOHandler() const { return m_pIOHandler; }
            void IOHandler(CServerIOHandler *Value) { SetIOHandler(Value); }

            CCommandHandlers *CommandHandlers() const { return m_pCommandHandlers; }
            void CommandHandlers(CCommandHandlers *Value) { m_pCommandHandlers = Value; }

            CStringList& Data() { return m_Data; }
            const CStringList& Data() const { return m_Data; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CAsyncClient ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CAsyncClient: public CEPollClient {
            typedef CEPollClient inherited;

        private:

            CCommandHandlers *m_pCommandHandlers;

        protected:

            bool m_AutoConnect;
            bool m_Active;
#ifdef WITH_SSL
            bool m_UsedSSL;
#endif
            CStringList m_Data;

            void SetActive(bool AValue);

            virtual void Initialize() {};

            virtual void InitializeCommandHandlers() {};

            virtual void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) abstract;

            bool DoCommand(CTCPConnection *AConnection) override;

        public:

            CAsyncClient();

            explicit CAsyncClient(const CString &Host, unsigned short Port);

            ~CAsyncClient() override;

            void ConnectStart();

            void Disconnect() { SetActive(false); };

            bool Active() const { return m_Active; }
            void Active(bool Value) { SetActive(Value); }

            bool AutoConnect() const { return m_AutoConnect; }
            void AutoConnect(bool Value) { m_AutoConnect = Value; }
#ifdef WITH_SSL
            bool UsedSSL() const { return m_UsedSSL; }
            void UsedSSL(bool Value) { m_UsedSSL = Value; }
#endif
            CCommandHandlers *CommandHandlers() const { return m_pCommandHandlers; }
            void CommandHandlers(CCommandHandlers *Value) { m_pCommandHandlers = Value; }

            CStringList& Data() { return m_Data; }
            const CStringList& Data() const { return m_Data; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPAsyncServer -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CUDPAsyncServer;

        typedef std::function<void (CUDPAsyncServer *Server, CSocketHandle *Socket, CManagedBuffer &Buffer)> COnUDPServerReadEvent;
        typedef std::function<void (CUDPAsyncServer *Sender, CSocketHandle *Socket, CSimpleBuffer &Buffer)> COnUDPServerWriteEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CUDPAsyncServer: public CAsyncServer {
        private:

            CManagedBuffer m_InputBuffer;
            CSimpleBuffer m_OutputBuffer;

            COnUDPServerReadEvent m_OnRead;
            COnUDPServerWriteEvent m_OnWrite;

            void DoBufferRead(CSocketHandle *ASocketHandle);
            void DoBufferWrite(CSocketHandle *ASocketHandle);

            void SetActiveLevel(CActiveLevel AValue) override;

        protected:

            void DoRead(CPollEventHandler *AHandler) override;
            void DoWrite(CPollEventHandler *AHandler) override;

        public:

            CUDPAsyncServer();

            explicit CUDPAsyncServer(unsigned short AListen);

            ~CUDPAsyncServer() override = default;

            void InitializeBindings() override;

            ssize_t Receive(CSocketHandle *ASocketHandle);
            ssize_t Send(CSocketHandle *ASocketHandle);

            CManagedBuffer &InputBuffer() { return m_InputBuffer; }
            const CManagedBuffer &InputBuffer() const { return m_InputBuffer; }

            CSimpleBuffer &OutputBuffer() { return m_OutputBuffer; }
            const CSimpleBuffer &OutputBuffer() const { return m_OutputBuffer; }

            const COnUDPServerReadEvent &OnRead() const { return m_OnRead; }
            void OnRead(COnUDPServerReadEvent && Value) { m_OnRead = Value; }

            const COnUDPServerWriteEvent &OnWrite() const { return m_OnWrite; }
            void OnWrite(COnUDPServerWriteEvent && Value) { m_OnWrite = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPAsyncServer -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CTCPAsyncServer: public CAsyncServer {
        private:

            void SetActiveLevel(CActiveLevel AValue) override;

            CTCPServerConnection *GetConnection(int AIndex) const;
            void SetConnection(int AIndex, CTCPServerConnection *AValue);

        protected:

            void DoTimeOut(CPollEventHandler *AHandler) override;
            void DoAccept(CPollEventHandler *AHandler) override;
            void DoRead(CPollEventHandler *AHandler) override;
            void DoWrite(CPollEventHandler *AHandler) override;

        public:

            CTCPAsyncServer();

            explicit CTCPAsyncServer(unsigned short AListen);

            ~CTCPAsyncServer() override = default;

            CTCPServerConnection *Connections(int Index) const { return GetConnection(Index); }
            void Connections(int Index, CTCPServerConnection *Value) { SetConnection(Index, Value); }

            CTCPServerConnection *operator[] (int Index) const override { return Connections(Index); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPAsyncClient -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CTCPAsyncClient: public CAsyncClient {
        protected:

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;

            void DoConnect(CPollEventHandler *AHandler) override;

        public:

            CTCPAsyncClient();

            explicit CTCPAsyncClient(LPCTSTR AHost, unsigned short APort);

            ~CTCPAsyncClient() override = default;

        };

    } // namespace Socket

} // namespace Delphi

using namespace Delphi::Socket;

#endif // DELPHI_SOCKETS_HPP
