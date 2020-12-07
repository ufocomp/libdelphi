/*++

Library name:

  libdelphi

Module Name:

  Sockets.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Sockets.hpp"
//----------------------------------------------------------------------------------------------------------------------

namespace Delphi {

    namespace Socket {

        static int GInstanceCount = 0;

        pthread_mutex_t GSocketCriticalSection;
        LIB_DELPHI CStack *GStack = nullptr;
        //--------------------------------------------------------------------------------------------------------------

        int SO_True = 1;
        int SO_False = 0;
        //--------------------------------------------------------------------------------------------------------------

        LIB_DELPHI char *inet_ntoa2(in_addr_t ulAddr) {
            in_addr addr = {};
            addr.s_addr = ulAddr;
            return inet_ntoa(addr);
        }
        //--------------------------------------------------------------------------------------------------------------

        LIB_DELPHI bool IsCurrentThread(CThread *AThread) {
            return AThread->Handle() == pthread_self();
        }
        //--------------------------------------------------------------------------------------------------------------

        LIB_DELPHI void SplitColumns(LPCTSTR AData, size_t ASize, CStringList *AStrings, char ADelim) {
            unsigned int idx;
            size_t len;

            if (ASize == 0)
                return;

            auto LData = new TCHAR[ASize + 2];
            auto LParam = new TCHAR[ASize + 2];

            try {
                ::SecureZeroMemory(LData, (ASize + 2) * sizeof(TCHAR));
                ::SecureZeroMemory(LParam, (ASize + 2) * sizeof(TCHAR));

                if (StringCchCopy(LData, ASize + 1, AData) == S_OK) {
                    if (LData[ASize - 1] != ADelim) {
                        LData[ASize] = ADelim;
                        LData[ASize + 1] = 0;
                    }

                    idx = 0;
                    for (size_t i = 0; i <= ASize; ++i) {
                        if (LData[i] == ADelim) {
                            chVERIFY(SUCCEEDED(StringCchLength(LParam, idx, &len)));
                            idx = 0;
                            if (len != 0)
                                AStrings->Add(LParam);
                            ::SecureZeroMemory(LParam, (ASize + 2) * sizeof(TCHAR));
                        } else {
                            LParam[idx++] = LData[i];
                        }
                    }
                }
            }
            catch (...) {
                //
            }

            delete[] LParam;
            delete[] LData;
        }
        //--------------------------------------------------------------------------------------------------------------

        LIB_DELPHI void SplitColumns(const CString &Data, CStringList &Strings, char Delimiter) {
            SplitColumns(Data.c_str(), Data.Size(), &Strings, Delimiter);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CStack ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CStack::CStack() {
            m_LastError = 0;
#ifdef WITH_SSL
            m_SSLError = SSL_ERROR_NONE;
#endif
            SecureZeroMemory(m_szBuffer, sizeof(m_szBuffer));
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CStack::CheckForSocketError(ssize_t AResult, CErrorGroup AErrGroup) {
            int Ignore[] = {0};
            return CheckForSocketError(AResult, Ignore, 0, AErrGroup);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CStack::CheckForSocketError(ssize_t AResult, int const AIgnore[], int ACount, CErrorGroup AErrGroup) {
            if (AResult == SOCKET_ERROR) {
                m_LastError = (AErrGroup == egSocket) ? h_errno : errno;
                for (int i = 0; i < ACount; ++i)
                    if (m_LastError == AIgnore[i])
                        return true;
                RaiseSocketError(m_LastError);
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocket CStack::CreateSocketHandle(int ASocketType, int AProtocol, unsigned int AFlag) {
            return Socket(AF_INET, ASocketType, AProtocol, AFlag);
        }
        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_SSL
        bool CStack::CheckForSSLError(ssize_t AResult) {
            unsigned long Ignore[] = { SSL_ERROR_NONE, SSL_ERROR_WANT_READ, SSL_ERROR_WANT_WRITE };
            return CheckForSSLError(AResult, Ignore, 1);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CStack::CheckForSSLError(ssize_t AResult, unsigned long const AIgnore[], int ACount) {
            if (AResult <= 0) {
                m_SSLError = GetSSLError();
                ERR_error_string(m_SSLError, m_szBuffer);
                for (int i = 0; i < ACount; ++i)
                    if (m_SSLError == AIgnore[i])
                        return true;
                RaiseSSLError();
            }
            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::SSLInit() {
            SSL_library_init();
            SSLeay_add_ssl_algorithms();
            SSL_load_error_strings();
        }
        //--------------------------------------------------------------------------------------------------------------

        SSL *CStack::SSLNew(bool IsSever) {
            const SSL_METHOD *method = IsSever ? TLS_server_method() : TLS_client_method();
            SSL_CTX *ctx = ::SSL_CTX_new(method);
            return ::SSL_new(ctx);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::SSLFree(SSL *ssl) {
            if (ssl != nullptr) {
                ::SSL_free(ssl);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocket CStack::SSLGetSocket(SSL *ssl) {
            if (ssl != nullptr) {
                return ::SSL_get_fd(ssl);
            }
            return INVALID_SOCKET;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::SSLAllocate(SSL *ssl, CSocket ASocket) {
            if (ssl != nullptr) {
                ::SSL_set_fd(ssl, ASocket);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::SSLClear(SSL *ssl) {
            return ::SSL_clear(ssl);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::SSLShutdown(SSL *ssl) {
            return ::SSL_shutdown(ssl);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::SSLConnect(SSL *ssl) {
            return ::SSL_connect(ssl);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::SSLRecv(SSL *ssl, void *ABuffer, size_t ABufferLength) {
            return ::SSL_read(ssl, ABuffer, ABufferLength);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::SSLSend(SSL *ssl, void *ABuffer, size_t ABufferLength) {
            return ::SSL_write(ssl, ABuffer, ABufferLength);
        }
        //--------------------------------------------------------------------------------------------------------------

        unsigned long CStack::GetSSLError() {
            return ::ERR_get_error();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::RaiseSSLError() {
            throw EExternalException(m_szBuffer);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::RecvPacket(SSL *ssl, void *ABuffer, size_t ABufferSize) {
            return SSLRecv(ssl, ABuffer, ABufferSize);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::SendPacket(SSL *ssl, void *ABuffer, size_t ABufferSize) {
            return SSLSend(ssl, ABuffer, ABufferSize);
        }
        //--------------------------------------------------------------------------------------------------------------
#endif
        void CStack::RaiseSocketError(int AErr) {
            throw ESocketError(AErr);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocket CStack::Accept(CSocket ASocket, char *VIP, size_t ASize, unsigned short *VPort, unsigned int AFlags) {
            CSocket Socket;

            SOCKADDR_IN addr = {};
            socklen_t addrlen = sizeof(SOCKADDR_IN);

            if (((AFlags & SOCK_NONBLOCK) == SOCK_NONBLOCK) || ((AFlags & SOCK_CLOEXEC) == SOCK_CLOEXEC))
                Socket = ::accept4(ASocket, (struct sockaddr *) &addr, &addrlen, (int) AFlags);
            else
                Socket = ::accept(ASocket, (struct sockaddr *) &addr, &addrlen);

            if (Socket != INVALID_SOCKET) {
                sa_family_t VFamily = 0;
                GetPeerName(Socket, &VFamily, VIP, ASize, VPort);
            }

            return Socket;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::Bind(CSocket ASocket, sa_family_t AFamily, LPCSTR AIP, unsigned short APort) {
            SOCKADDR_IN name = {};
            socklen_t namelen = sizeof(SOCKADDR_IN);

            name.sin_family = AFamily;

            if (AIP[0] == 0)
                name.sin_addr.s_addr = htonl(INADDR_ANY);
            else
                name.sin_addr.s_addr = inet_addr(AIP);

            name.sin_port = htons(APort);

            return ::bind(ASocket, (LPSOCKADDR) &name, namelen);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::CloseSocket(CSocket ASocket) {
            return ::close(ASocket);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::Connect(CSocket ASocket, sa_family_t AFamily, LPCSTR AIP, unsigned short APort) {
            SOCKADDR_IN name = {};
            sa_family_t namelen = sizeof(SOCKADDR_IN);

            name.sin_family = AFamily;
            name.sin_addr.s_addr = inet_addr(AIP);
            name.sin_port = htons(APort);

            return ::connect(ASocket, (LPSOCKADDR) &name, namelen);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::Listen(CSocket ASocket, int ABackLog) {
            return ::listen(ASocket, ABackLog);
        }
        //--------------------------------------------------------------------------------------------------------------

        uint16_t CStack::HToNS(uint16_t AHostShort) {
            return htons(AHostShort);
        }
        //--------------------------------------------------------------------------------------------------------------

        uint16_t CStack::NToHS(uint16_t ANetShort) {
            return ntohs(ANetShort);
        }
        //--------------------------------------------------------------------------------------------------------------

        uint32_t CStack::HToNL(uint32_t AHostLong) {
            return htonl(AHostLong);
        }
        //--------------------------------------------------------------------------------------------------------------

        uint32_t CStack::NToHL(uint32_t ANetLong) {
            return ntohl(ANetLong);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::Recv(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags) {
            return ::recv(ASocket, (char *) ABuffer, ABufferLength, AFlags);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::RecvFrom(const CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags, char *VIP,
                                 size_t ASize, unsigned short *VPort, LPSOCKADDRIN AFrom, socklen_t *AFromLen) {
            ssize_t BytesRecv = 0;

            SOCKADDR_IN from = {};
            socklen_t fromLen = sizeof(SOCKADDR_IN);

            if (AFrom == nullptr) {
                AFrom = &from;
                AFromLen = &fromLen;
            }

            BytesRecv = ::recvfrom(ASocket, ABuffer, ABufferLength, AFlags, (LPSOCKADDR) AFrom, AFromLen);

            if (BytesRecv != -1) {
                chVERIFY(SUCCEEDED(StringCchCopyA(VIP, ASize, inet_ntoa(AFrom->sin_addr))));
                *VPort = NToHS(AFrom->sin_port);
            }

            return BytesRecv;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::Send(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags) {
            return ::send(ASocket, ABuffer, ABufferLength, AFlags);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CStack::SendTo(CSocket ASocket, void *ABuffer, size_t ABufferLength, int AFlags, LPCSTR AIP,
                               unsigned short APort) {

            SOCKADDR_IN to = {};
            socklen_t tolen = sizeof(SOCKADDR_IN);

            to.sin_family = AF_INET;
            to.sin_addr.s_addr = inet_addr(AIP);
            to.sin_port = HToNS(APort);

            return ::sendto(ASocket, ABuffer, ABufferLength, AFlags, (sockaddr * ) &to, tolen);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocket CStack::Select(CList *ARead, CList *AWrite, CList *AErrors, int ATimeout) {
            int nfds = 0;
            SOCKET Socket;

            timeval time_out = {};
            timeval *timeval = &time_out;

            fd_set read_s, write_s, except_s;
            fd_set *readfds = &read_s, *writefds = &write_s, *exceptfds = &except_s;

            FD_ZERO(readfds);
            FD_ZERO(writefds);
            FD_ZERO(exceptfds);

            if (ARead != nullptr) {
                for (int i = 0; i < ARead->Count(); ++i) {
                    Socket = *(CSocket *) ARead->Items(i);
                    FD_SET(Socket, readfds);
                    nfds = Max(nfds, Socket);
                }
            }

            if (AWrite != nullptr) {
                for (int i = 0; i < AWrite->Count(); ++i) {
                    Socket = *(CSocket *) AWrite->Items(i);
                    FD_SET(Socket, writefds);
                    nfds = Max(nfds, Socket);
                }
            }

            if (AErrors != nullptr) {
                for (int i = 0; i < AErrors->Count(); ++i) {
                    Socket = *(CSocket *) AErrors->Items(i);
                    FD_SET(Socket, exceptfds);
                    nfds = Max(nfds, Socket);
                }
            }

            if (ATimeout > 0) {
                time_out.tv_sec = 0;
                time_out.tv_usec = ATimeout;
            }

            return ::select(nfds + 1, readfds, writefds, exceptfds, timeval);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::SetSockOpt(CSocket ASocket, int ALevel, int AOptName, const void *AOptVal, socklen_t AOptLen) {
            return ::setsockopt(ASocket, ALevel, AOptName, AOptVal, AOptLen);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocket CStack::Socket(int ADomain, int AType, int AProtocol, unsigned int AFlag) {
            CSocket Socket;

            Socket = ::socket(ADomain, AType, AProtocol);

            //DebugMessage(_T("create socket: %d - domain: %d, type: %d, protocol: %d, flag: %d \n"), Socket, ADomain, AType, AProtocol, AFlag);

            if (Socket == INVALID_SOCKET)
                throw ESocketError(_T("Cannot allocate socket."));

            if ((AFlag & O_NONBLOCK) == O_NONBLOCK) {
                SetNonBloking(Socket);
            }

            return Socket;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::Shutdown(CSocket ASocket, int AHow) {
            return ::shutdown(ASocket, AHow);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CStack::GetHostName(char *AName, size_t ASize) {
            char name[NI_MAXHOST] = {};

            if (!CheckForSocketError(::gethostname(name, sizeof(name)), egSystem)) {
                chVERIFY(SUCCEEDED(StringCchCopyA(AName, ASize, name)));
                return true;
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::GetSocketError(CSocket ASocket) {
            socklen_t len = sizeof(m_LastError);
            CheckForSocketError(GetSockOpt(ASocket, SOL_SOCKET, SO_ERROR, (void *) &m_LastError, &len), egSystem);
            return m_LastError;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::GetLastError() {
            return errno;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetPeerName(CSocket ASocket, sa_family_t *AFamily, char *AIP, size_t ASize,
                unsigned short *APort) {
            SOCKADDR name = {};
            socklen_t namelen = sizeof(SOCKADDR);

            if (!CheckForSocketError(::getpeername(ASocket, (LPSOCKADDR) &name, &namelen), egSystem)) {
                *AFamily = NToHS(((SOCKADDR_IN * ) & name)->sin_family);
                chVERIFY(SUCCEEDED(StringCchCopyA(AIP, ASize, inet_ntoa(((SOCKADDR_IN *) &name)->sin_addr))));
                *APort = NToHS(((SOCKADDR_IN * ) & name)->sin_port);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetSockName(CSocket ASocket, sa_family_t *AFamily, char *AIP, size_t ASize,
                unsigned short *APort) {
            SOCKADDR name = {};
            socklen_t namelen = sizeof(SOCKADDR);

            if (!CheckForSocketError(::getsockname(ASocket, (LPSOCKADDR) & name, &namelen), egSystem)) {
                *AFamily = NToHS(((SOCKADDR_IN * ) & name)->sin_family);
                chVERIFY(SUCCEEDED(StringCchCopyA(AIP, ASize, inet_ntoa(((SOCKADDR_IN * ) & name)->sin_addr))));
                *APort = NToHS(((SOCKADDR_IN * ) & name)->sin_port);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetHostByAddr(SOCKADDR_IN *AAddr, char *VHost, size_t ASize) {
            struct hostent *host = nullptr;

            host = ::gethostbyaddr((char *) &AAddr->sin_addr, sizeof(AAddr->sin_addr), AF_INET);

            if (host == nullptr) {
                chVERIFY(SUCCEEDED(StringCchCopyA(VHost, ASize, inet_ntoa(AAddr->sin_addr))));
            } else {
                chVERIFY(SUCCEEDED(StringCchCopyA(VHost, ASize, host->h_name)));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetHostByName(const char *AName, char *VHost, size_t ASize) {
            struct hostent *host = nullptr;
            struct in_addr addr = {};

            if (AName[0] == '\0' || isalpha(AName[0])) {
                host = ::gethostbyname(AName);
            } else {
                addr.s_addr = inet_addr(AName);
                if (addr.s_addr != INADDR_NONE)
                    host = ::gethostbyaddr((char *) &addr, 4, AF_INET);
            }

            if (host == nullptr) {
                chVERIFY(SUCCEEDED(StringCchCopyA(VHost, ASize, inet_ntoa(addr))));
            } else {
                chVERIFY(SUCCEEDED(StringCchCopyA(VHost, ASize, host->h_name)));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetIPByName(const char *AName, char *VIP, size_t ASize) {
            struct hostent *host = nullptr;
            struct in_addr addr = {};

            host = ::gethostbyname(AName);
            if (host == nullptr) {
                addr.s_addr = inet_addr(AName);
                if (addr.s_addr != INADDR_NONE)
                    host = ::gethostbyaddr((char *) &addr, 4, AF_INET);
            }

            if (host != nullptr)
                addr.s_addr = *(in_addr_t *) host->h_addr;

            chVERIFY(SUCCEEDED(StringCchCopyA(VIP, ASize, inet_ntoa(addr))));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::GetServByPort(SOCKADDR_IN *AName, char *VPort, size_t ASize) {
            LPSERVENT Serv = nullptr;

            Serv = ::getservbyport(AName->sin_port, "udp");
            if (Serv == nullptr) {
                chVERIFY(SUCCEEDED(StringCchPrintfA(VPort, ASize, "%d", NToHS(AName->sin_port))));
            } else {
                chVERIFY(SUCCEEDED(StringCchCopyA(VPort, ASize, Serv->s_name)));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStack::GetSockOpt(CSocket ASocket, int Alevel, int AOptname, void *AOptval, socklen_t *AOptlen) {
            return ::getsockopt(ASocket, Alevel, AOptname, AOptval, AOptlen);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStack::SetNonBloking(CSocket ASocket) {
            int LFlags = fcntl(ASocket, F_GETFL, 0);

            if (LFlags == -1)
                throw EOSError(errno, _T("fcntl failed (F_GETFL): "));

            if ((LFlags & O_NONBLOCK) == 0) {
                LFlags |= O_NONBLOCK;
                if (fcntl(ASocket, F_SETFL, LFlags) == -1)
                    throw EOSError(errno, _T("fcntl failed (F_SETFL): "));
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketComponent ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        inline void AddSocket() {

            if (GInstanceCount == 0)
                pthread_mutex_init(&GSocketCriticalSection, nullptr);

            pthread_mutex_lock(&GSocketCriticalSection);

            try {
                if (GInstanceCount == 0) {
                    GStack = CStack::CreateSocket();
#ifdef WITH_SSL
                    GStack->SSLInit();
#endif
                }

                GInstanceCount++;
            } catch (...) {
            }

            pthread_mutex_unlock(&GSocketCriticalSection);
        }
        //--------------------------------------------------------------------------------------------------------------

        inline void RemoveSocket() {

            pthread_mutex_lock(&GSocketCriticalSection);

            try {
                GInstanceCount--;

                if (GInstanceCount == 0)
                {
                    Delphi::Socket::CStack::DeleteSocket();
                    GStack = nullptr;
                }
            } catch (...) {
            }

            pthread_mutex_unlock(&GSocketCriticalSection);

            if (GInstanceCount == 0)
                pthread_mutex_destroy(&GSocketCriticalSection);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketComponent::CSocketComponent(): m_WorkInfos{} {
            AddSocket();
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketComponent::~CSocketComponent() {
            RemoveSocket();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketComponent::BeginWork(CWorkMode AWorkMode, ssize_t ASize) {
            m_WorkInfos[AWorkMode].Level++;
            if (m_WorkInfos[AWorkMode].Level == 1) {
                m_WorkInfos[AWorkMode].Max = ASize;
                m_WorkInfos[AWorkMode].Current = 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketComponent::DoWork(CWorkMode AWorkMode, ssize_t ACount) {
            if (m_WorkInfos[AWorkMode].Level > 0)
                m_WorkInfos[AWorkMode].Current += ACount;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketComponent::EndWork(CWorkMode AWorkMode) {
            m_WorkInfos[AWorkMode].Level--;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSimpleBuffer ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSimpleBuffer::CSimpleBuffer(): CMemoryStream() {

        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CSimpleBuffer::Extract(void *ABuffer, size_t AByteCount) {
            if (AByteCount > Size())
                throw ESocketError(_T("Not enough data in buffer."));

            ::MoveMemory(ABuffer, Memory(), AByteCount);
            Remove(AByteCount);

            return AByteCount;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSimpleBuffer::Remove(size_t AByteCount) {
            if (AByteCount > Size())
                throw ESocketError(_T("Not enough data in buffer."));

            if (AByteCount == Size()) {
                Clear();
            } else {
                ::MoveMemory(Memory(), Pointer(size_t(Memory()) + AByteCount), Size() - AByteCount);
                SetSize(Size() - AByteCount);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CManagedBuffer --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CManagedBuffer::CManagedBuffer(): CSimpleBuffer() {
            m_ReadSize = 0;
            m_PackReadSize = InBufCacheSizeDefault;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CManagedBuffer::SetPackReadSize(size_t Value) {
            if (Value > 0)
                m_PackReadSize = Value;
            else
                m_PackReadSize = InBufCacheSizeDefault;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CManagedBuffer::Clear() {
            inherited::Clear();
            m_ReadSize = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CManagedBuffer::Extract(void *ABuffer, size_t AByteCount) {
            if (AByteCount > Size())
                throw ESocketError(_T("Not enough data in buffer."));

            ::MoveMemory(ABuffer, Memory(), AByteCount);
            Remove(AByteCount);

            return AByteCount;
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CManagedBuffer::Memory() const {
            return Pointer(size_t(inherited::Memory()) + m_ReadSize);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CManagedBuffer::PackBuffer() {
            if (m_ReadSize > 0) {
                ::MoveMemory(inherited::Memory(), Memory(), Size());
                SetSize(Size());
                m_ReadSize = 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CManagedBuffer::Remove(const size_t AByteCount) {
            if (AByteCount > Size()) {
                throw ESocketError(_T("Not enough data in buffer."));
            } else if (AByteCount == Size()) {
                Clear();
            } else {
                m_ReadSize += AByteCount;

                if (m_ReadSize >= PackReadSize())
                    PackBuffer();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CManagedBuffer::Seek(size_t Offset, unsigned short Origin) {
            switch (Origin) {
                case soFromBeginning:
                    return inherited::Seek(Offset + m_ReadSize, soFromBeginning) - m_ReadSize;

                default: // soFromCurrent, soFromEnd:
                    return inherited::Seek(Offset, Origin) - m_ReadSize;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketHandle ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketHandle::CSocketHandle(CCollection *ACollection): CSocketComponent(), CCollectionItem(ACollection),
                m_IP{0}, m_PeerIP{0}, m_From{} {
#ifdef WITH_SSL
            m_pSSL = nullptr;
            m_SSLMethod = sslNotUsed;
#endif
            m_Port = 0;
            m_PeerPort = 0;

            m_FromLen = sizeof(SOCKADDR_IN);

            m_HandleAllocated = false;
            m_Nonblocking = false;

            m_Handle = INVALID_SOCKET;
            Reset();

            m_SocketType = SOCK_STREAM;

            m_ClientPortMin = 0;
            m_ClientPortMax = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketHandle::~CSocketHandle() {
            CloseSocket();
#ifdef WITH_SSL
            CloseSSL();
#endif
        }
        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_SSL
        void CSocketHandle::AllocateSSL() {
            if (m_SSLMethod != sslNotUsed) {
                if (m_pSSL == nullptr)
                    m_pSSL = CStack::SSLNew(m_SSLMethod == sslServer);
                CStack::SSLAllocate(m_pSSL, m_Handle);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::ShutdownSSL() {
            if (m_pSSL != nullptr) {
                GStack->CheckForSSLError(CStack::SSLShutdown(m_pSSL));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::CloseSSL() {
            if (m_pSSL != nullptr) {
                CStack::SSLFree(m_pSSL);
                m_pSSL = nullptr;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::ClearSSL() {
            if (m_pSSL != nullptr) {
                GStack->CheckForSSLError(CStack::SSLClear(m_pSSL));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::ConnectSSL() {
            if (m_pSSL != nullptr) {
                GStack->CheckForSSLError(CStack::SSLConnect(m_pSSL));
            }
        }
        //--------------------------------------------------------------------------------------------------------------
#endif
        int CSocketHandle::GetSocketError() const {
            return GStack->GetSocketError(Handle());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::GetSockOpt(int ALevel, int AOptName, void *AOptVal, socklen_t AOptLen) const {
            GStack->CheckForSocketError(GStack->GetSockOpt(Handle(), ALevel, AOptName, AOptVal, &AOptLen), egSystem);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::GetHostName(char *AName, size_t ASize) {
            return GStack->GetHostName(AName, ASize);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::GetHostByName(const char *AName, char *VHost, size_t ASize) {
            GStack->GetHostByName(AName, VHost, ASize);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::GetIPByName(const char *AName, char *VIP, size_t ASize) {
            GStack->GetIPByName(AName, VIP, ASize);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::GetHostIP(char *AIP, size_t ASize) {
            char name[NI_MAXHOST] = {};

            if (GStack->GetHostName(name, chARRAY(name))) {
                GStack->GetIPByName(name, AIP, ASize);
                return true;
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::SetIP(LPCSTR Value) {
            if (Value != nullptr) {
                ::SecureZeroMemory(m_IP, sizeof(m_IP));
                chVERIFY(SUCCEEDED(StringCchCopyA(m_IP, chARRAY(m_IP), Value)));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::SetPeerIP(LPCSTR Value) {
            if (Value != nullptr) {
                ::SecureZeroMemory(m_PeerIP, sizeof(m_PeerIP));
                chVERIFY(SUCCEEDED(StringCchCopyA(m_PeerIP, chARRAY(m_PeerIP), Value)));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::Accept(CSocket ASocket, unsigned int AFlags) {
            int Ignore[] = {EINTR,     // CloseSocket while in Accept
                            ENOTSOCK}; // CloseSocket just prior to Accept;

            Reset();
            CSocket AcceptedSocket = GStack->Accept(ASocket, m_IP, chARRAY(m_IP), &m_Port, AFlags);

            if (!GStack->CheckForSocketError(AcceptedSocket, Ignore, chARRAY(Ignore), egSystem)) {
                m_Handle = AcceptedSocket;
                m_HandleAllocated = true;
                m_Nonblocking = (AFlags & SOCK_NONBLOCK) == SOCK_NONBLOCK;

                UpdateBindingLocal();
                UpdateBindingPeer();

                return true;
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::AllocateSocket(int ASocketType, int AProtocol, unsigned int AFlag) {
            // If we are reallocating a socket - close and destroy the old socket handle
            CloseSocket();
            if (HandleAllocated())
                Reset();

            m_Handle = GStack->CreateSocketHandle(ASocketType, AProtocol, AFlag);
            m_SocketType = ASocketType;
            m_HandleAllocated = true;
            m_Nonblocking = (AFlag == O_NONBLOCK);
#ifdef WITH_SSL
            AllocateSSL();
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::CloseSocket(bool AResetLocal) {
            if (HandleAllocated()) {
#ifdef WITH_SSL
                ShutdownSSL();
#endif
                // Must be first, closing socket will trigger some errors, and they
                // may then check (in other threads) Connected, which checks this.
                GStack->Shutdown(m_Handle, SHUT_RDWR);
                // SO_LINGER is false - socket may take a little while to actually close after this
                GStack->CloseSocket(m_Handle);

                // Reset all data
                Reset(AResetLocal);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::Reset(bool AResetLocal) {
#ifdef WITH_SSL
            ClearSSL();
#endif
            m_HandleAllocated = false;
            m_Nonblocking = false;
            m_Handle = INVALID_SOCKET;

            if (AResetLocal) {
                SetIP("");
                Port(0);
            }

            m_FromLen = sizeof(SOCKADDR_IN);
            ::SecureZeroMemory(&m_From, m_FromLen);

            SetPeer("", 0);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::Bind() {
            if ((Port() == 0) && (m_ClientPortMin != 0) && (m_ClientPortMax != 0)) {
                if (m_ClientPortMin > m_ClientPortMax)
                    throw ESocketError(_T("Invalid Port Range."));
                else if (!BindPortReserved())
                    throw ESocketError(_T("Can not bind in port range."));
            } else {
                if (!TryBind())
                    throw ESocketError(_T("Could not bind socket. Address and port are already in use."));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::TryBind() {
            bool fSuccess;
            int Ignore[] = {EADDRINUSE};
            fSuccess = !GStack->CheckForSocketError(GStack->Bind(m_Handle, AF_INET, m_IP, m_Port), Ignore,
                                                    chARRAY(Ignore), egSystem);
            if (fSuccess)
                UpdateBindingLocal();
            return fSuccess;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::BindPortReserved() {
            for (unsigned short i = m_ClientPortMin; i <= m_ClientPortMax; ++i) {
                m_Port = i;
                if (TryBind())
                    return true;
            }
            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::Listen(int anQueueCount) const {
            GStack->CheckForSocketError(GStack->Listen(Handle(), anQueueCount), egSystem);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSocketHandle::Connect(sa_family_t AFamily, LPCSTR AHost, unsigned short APort) {
            char IP[NI_MAXIP] = {};
            int Ignore[] = {EINPROGRESS, EALREADY};

            GStack->GetIPByName(AHost, IP, sizeof(IP));

            int SocketError = GStack->Connect(Handle(), AFamily, IP, APort);

            if (!GStack->CheckForSocketError(SocketError, Ignore, chARRAY(Ignore), egSystem)) {
                UpdateBindingLocal();
                UpdateBindingPeer();
            }
#ifdef WITH_SSL
            ConnectSSL();
#endif
            return SocketError;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::CheckConnection() {

            int Ignore[] = {EINPROGRESS, EALREADY};
            int SocketError = GetSocketError();

            if ((SocketError == 0) || (SocketError == EISCONN)) {

                UpdateBindingLocal();
                UpdateBindingPeer();

            } else {

                for (int i : Ignore) {
                    if (SocketError == i)
                        return false;
                }
#ifdef WITH_SSL
                CloseSSL();
#endif
                GStack->RaiseSocketError(SocketError);
            }

            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CSocketHandle::Recv(void *ABuffer, size_t ABufferSize, int AFlags) const {
#ifdef WITH_SSL
            if (m_pSSL != nullptr)
                return GStack->RecvPacket(m_pSSL, ABuffer, ABufferSize);
#endif
            return GStack->Recv(Handle(), ABuffer, ABufferSize, AFlags);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CSocketHandle::RecvFrom(void *ABuffer, size_t ABufferSize, int AFlags) {
            m_FromLen = sizeof(SOCKADDR_IN);
            ::SecureZeroMemory(&m_From, m_FromLen);

            return GStack->RecvFrom(Handle(), ABuffer, ABufferSize, AFlags, m_PeerIP, chARRAY(m_PeerIP), &m_PeerPort, &m_From, &m_FromLen);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CSocketHandle::Send(void *ABuffer, size_t ABufferSize, int AFlags) {
#ifdef WITH_SSL
            if (m_pSSL != nullptr)
                return GStack->SendPacket(m_pSSL, ABuffer, ABufferSize);
#endif
            return SendTo(IP(), Port(), ABuffer, ABufferSize, AFlags);
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CSocketHandle::SendTo(LPCSTR AIP, const unsigned short APort, void *ABuffer, size_t ABufferSize, int AFlags) const {
            ssize_t BytesOut;

            if (m_SocketType == SOCK_STREAM || m_SocketType == SOCK_SEQPACKET)
                BytesOut = GStack->Send(Handle(), ABuffer, ABufferSize, AFlags);
            else
                BytesOut = GStack->SendTo(Handle(), ABuffer, ABufferSize, AFlags, AIP, APort);

            if (BytesOut == SOCKET_ERROR) {
                int LastError = GStack->GetLastError();
                if (LastError == EMSGSIZE)
                    throw ESocketError(LastError);
            }

            return BytesOut;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::SetPeer(LPCSTR asIP, unsigned short anPort) {
            SetPeerIP(asIP);
            m_PeerPort = anPort;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::SetSockOpt(int ALevel, int AOptName, const void *AOptVal, socklen_t AOptLen) const {
            GStack->CheckForSocketError(GStack->SetSockOpt(Handle(), ALevel, AOptName, AOptVal, AOptLen), egSystem);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::Select(CSocket ASocket, int ATimeOut) {
            auto ReadList = new CList;
            ReadList->Add(&ASocket);
            bool fSuccess = (GStack->Select(ReadList, nullptr, nullptr, ATimeOut) == 1);
            FreeAndNil(ReadList);

            return fSuccess;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketHandle::SelectWrite(CSocket ASocket, int ATimeOut) {
            auto WriteList = new CList;
            WriteList->Add(&ASocket);
            bool fSuccess = (GStack->Select(nullptr, WriteList, nullptr, ATimeOut) == 1);
            FreeAndNil(WriteList);

            return fSuccess;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::UpdateBindingLocal() {
            sa_family_t LFamily;
            GStack->GetSockName(Handle(), &LFamily, m_IP, chARRAY(m_IP), &m_Port);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::UpdateBindingPeer() {
            sa_family_t LFamily;
            GStack->GetPeerName(Handle(), &LFamily, m_PeerIP, chARRAY(m_PeerIP), &m_PeerPort);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::UpdateFromPeer() {
            chVERIFY(SUCCEEDED(StringCchCopyA(m_PeerIP, chARRAY(m_PeerIP), inet_ntoa(m_From.sin_addr))));
            m_PeerPort = GStack->NToHS(m_From.sin_port);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandle::SetDefaultPort(const unsigned short Value) {
            m_Port = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CSocketHandle::GetLastError() {
            return GStack->LastError();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketHandles --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketHandles::CSocketHandles(): CCollection(this) {
            m_DefaultPort = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketHandle *CSocketHandles::GetItem(int Index) const {
            return (CSocketHandle *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketHandles::SetItem(int Index, CSocketHandle *Value) {
            inherited::SetItem(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

#ifdef WITH_SSL
        CSocketHandle *CSocketHandles::Add(CSSLMethod ASSLMethod) {
            auto Result = new CSocketHandle(this);
            Result->SSLMethod(ASSLMethod);
#else
        CSocketHandle *CSocketHandles::Add() {
            auto Result = new CSocketHandle(this);
#endif
            inherited::Added((CCollectionItem *) Result);
            if (!DefaultIP().IsEmpty())
                Result->IP(DefaultIP().c_str());
            Result->Port(DefaultPort());
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketHandle *CSocketHandles::Insert(int Index) {
            CSocketHandle *Result = Add();
            Result->Index(Index);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketHandle *CSocketHandles::BindingByHandle(const CSocket AHandle) {
            int i;
            CSocketHandle *Result = nullptr;
            i = Count() - 1;
            while ((i >= 0) && (Handles(i)->Handle() != AHandle))
                i--;

            if (i >= 0)
                Result = Handles(i);

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CIOHandlerSocket ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CIOHandlerSocket::CIOHandlerSocket() : CIOHandler() {
            m_pBinding = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CIOHandlerSocket::~CIOHandlerSocket() {
            CIOHandlerSocket::Close();
        }
        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_SSL
        void CIOHandlerSocket::Open(CSSLMethod SSLMethod) {
            if (m_pBinding == nullptr) {
                m_pBinding = new CSocketHandle(nullptr);
                m_pBinding->SSLMethod(SSLMethod);
            } else
                m_pBinding->Reset(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CIOHandlerSocket::UsedSSL() const {
            bool bResult = (m_pBinding != nullptr);
            if (bResult)
                bResult = m_pBinding->UsedSSL();
            return bResult;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSSLMethod CIOHandlerSocket::SSLMethod() const {
            if (m_pBinding != nullptr)
                return m_pBinding->SSLMethod();
            return sslNotUsed;
        }
        //--------------------------------------------------------------------------------------------------------------
#else
        void CIOHandlerSocket::Open() {
            if (m_pBinding == nullptr)
                m_pBinding = new CSocketHandle(nullptr);
            else
                m_pBinding->Reset(true);
        }
        //--------------------------------------------------------------------------------------------------------------
#endif
        void CIOHandlerSocket::Close() {
            if (m_pBinding != nullptr) {
                FreeAndNil(m_pBinding);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CIOHandlerSocket::Connected() {
            bool bResult = (m_pBinding != nullptr);
            if (bResult)
                bResult = m_pBinding->HandleAllocated();
            return bResult;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CIOHandlerSocket::Recv(void *ABuf, size_t ALen) {
            if (Connected())
                return Binding()->Recv(ABuf, ALen);
            else
                throw ESocketError(_T("Disconnected."));
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CIOHandlerSocket::Send(void *ABuf, size_t ALen) {
            if (Connected())
                return Binding()->Send(ABuf, ALen);
            else
                throw ESocketError(_T("Disconnected."));
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollConnection -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPollConnection::CPollConnection(CPollConnection *AOwner, CPollManager *AManager):
                CCollectionItem(AManager) {
            m_pEventHandler = nullptr;
            m_pOwner = AOwner;
            m_CloseConnection = true;
            m_AutoFree = true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollConnection::SetEventHandler(CPollEventHandler *AValue) {
            if (m_pEventHandler != AValue) {
                m_pEventHandler = AValue;
                m_pEventHandler->Binding(m_pOwner);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollConnection::ClosePoll() {
            if (Assigned(m_pEventHandler))
                m_pEventHandler->Stop();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPConnection --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPConnection::CTCPConnection(CPollManager *AManager): CPollConnection(this, AManager) {
            m_Clock = 0;

            m_pIOHandler = nullptr;
            m_pSocket = nullptr;

            m_pWriteBuffer = nullptr;
            m_OnDisconnected = nullptr;

            m_ReadTimeOut = 0;
            m_MaxLineAction = maSplit;
            m_WriteBufferThreshold = 0;

            m_FreeIOHandler = false;
            m_ReadLnSplit = false;
            m_ReadLnTimedOut = false;
            m_ClosedGracefully = false;
            m_OEM = false;

            m_pRecvBuffer = new CSimpleBuffer;
            m_RecvBufferSize = GRecvBufferSizeDefault;

            m_pInputBuffer = new CManagedBuffer;
            m_pOutputBuffer = new CSimpleBuffer;

            m_SendBufferSize = GSendBufferSizeDefault;
            m_MaxLineLength = MaxLineLengthDefault;
        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPConnection::~CTCPConnection() {
            DisconnectSocket();

            FreeIOHandler();
            FreeAndNil(m_pInputBuffer);
            FreeAndNil(m_pOutputBuffer);
            FreeAndNil(m_pRecvBuffer);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CTCPConnection::Connected() {
            CheckForDisconnect(false);
            bool bResult = (IOHandler() != nullptr);
            if (bResult)
                bResult = IOHandler()->Connected();
            return bResult;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::Disconnect() {
            DisconnectSocket();
            FreeIOHandler();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::DisconnectSocket() {
            if (IOHandler() != nullptr) {
                DoDisconnected();
                ClosePoll();
                m_ClosedGracefully = true;
                m_pIOHandler->Close();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::FreeIOHandler() {
            if (m_FreeIOHandler)
                delete m_pIOHandler;
            m_pIOHandler = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::SetIOHandler(CIOHandler *AValue, bool AFree) {
            if (m_pIOHandler != AValue) {

                if (m_pIOHandler != nullptr)
                    FreeIOHandler();

                if (AValue == nullptr)
                    m_pSocket = nullptr;
                else
                    m_pSocket = (CIOHandlerSocket *) AValue;

                m_FreeIOHandler = AFree;
                m_pIOHandler = AValue;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CTCPConnection::CheckForDisconnect(bool ARaiseExceptionIfDisconnected) {
            bool LDisconnected = false;
            if (ClosedGracefully() || (IOHandler() == nullptr)) {
                if (IOHandler() != nullptr) {
                    if (IOHandler()->Connected())
                        DisconnectSocket();
                    LDisconnected = true;
                }
            } else if (IOHandler() != nullptr)
                LDisconnected = !IOHandler()->Connected();

            if (LDisconnected)
                if (ARaiseExceptionIfDisconnected)
                    throw ESocketError(_T("Connection closed gracefully."));
            return !LDisconnected;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::OpenWriteBuffer(ssize_t AThreshhold) {
            m_pWriteBuffer = new CSimpleBuffer;
            m_WriteBufferThreshold = AThreshhold;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::CloseWriteBuffer() {
            try {
                FlushWriteBuffer();
            } catch (...) {
                FreeAndNil(m_pWriteBuffer);
                throw;
            }
            FreeAndNil(m_pWriteBuffer);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::FlushWriteBuffer(ssize_t AByteCount) {
            if (m_pWriteBuffer->Size() > 0) {
                if ((AByteCount == -1) || (m_pWriteBuffer->Size() <= (size_t) AByteCount)) {
                    WriteBuffer(m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), true);
                    ClearWriteBuffer();
                } else {
                    WriteBuffer(m_pWriteBuffer->Memory(), (size_t) AByteCount, true);
                    m_pWriteBuffer->Remove((size_t) AByteCount);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::ClearWriteBuffer() {
            if (m_pWriteBuffer != nullptr)
                m_pWriteBuffer->Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::FreeWriteBuffer() {
            FreeAndNil(m_pWriteBuffer);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::FlushOutputBuffer(ssize_t AByteCount) {
            if (m_pOutputBuffer->Size() > 0) {
                if ((AByteCount == -1) || (m_pOutputBuffer->Size() <= (size_t) AByteCount)) {
                    WriteBuffer(m_pOutputBuffer->Memory(), m_pOutputBuffer->Size(), true);
                    m_pOutputBuffer->Clear();
                } else {
                    WriteBuffer(m_pOutputBuffer->Memory(), (size_t) AByteCount, true);
                    m_pOutputBuffer->Remove((size_t) AByteCount);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::CheckWriteResult(ssize_t AByteCount) {
            m_ClosedGracefully = (AByteCount == 0);
            // Check if other side disconnected
            CheckForDisconnect();
            // Check to see if the error signifies disconnection
#ifdef WITH_SSL
            if (m_pIOHandler->UsedSSL()) {
                unsigned long Ignore[] = { SSL_ERROR_WANT_WRITE, SSL_ERROR_SYSCALL };
                if (GStack->CheckForSSLError(AByteCount, Ignore, chARRAY(Ignore))) {
                    if (GStack->SSLError() == SSL_ERROR_SYSCALL) {
                        DisconnectSocket();
                        GStack->RaiseSocketError(GStack->SSLError());
                    }
                }
            } else {
#endif
                int Ignore[] = { ESHUTDOWN, ECONNABORTED, ECONNRESET };
                if (GStack->CheckForSocketError(AByteCount, Ignore, chARRAY(Ignore), egSystem)) {
                    DisconnectSocket();
                    GStack->RaiseSocketError(GStack->LastError());
                }
#ifdef WITH_SSL
            }
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::WriteBuffer(void *ABuffer, size_t AByteCount, bool AWriteNow) {
            CSimpleBuffer *LBuffer = nullptr;

            ssize_t LByteCount;
            size_t LPos;

            if ((AByteCount > 0) && (ABuffer != nullptr)) {
                CheckForDisconnect(true);

                if (Connected()) {
                    if ((m_pWriteBuffer == nullptr) || AWriteNow) {

                        if (m_pWriteBuffer == nullptr) {
                            LBuffer = new CSimpleBuffer();
                        } else
                            LBuffer = m_pWriteBuffer;

                        try {
                            if (LBuffer->Size() == 0)
                                LBuffer->WriteBuffer(ABuffer, AByteCount);

                            LPos = 0;
                            do {
                                LByteCount = m_pIOHandler->Send(Pointer((size_t) LBuffer->Memory() + LPos), LBuffer->Size() - LPos);
#ifdef WITH_SSL
                                if (m_pIOHandler->UsedSSL()) {
                                    if (LByteCount <= 0) {
                                        unsigned long LastError = GStack->GetSSLError();
                                        if (LastError == SSL_ERROR_WANT_WRITE)
                                            break;
                                    }
                                }
#endif
                                CheckWriteResult(LByteCount);
                                DoWork(wmWrite, LByteCount);
                                LPos += LByteCount;
                            } while (LPos < AByteCount);
                        } catch (...) {
                            LBuffer->Clear();
                            if (LBuffer != m_pWriteBuffer)
                                delete LBuffer;
                            throw;
                        }
                        LBuffer->Clear();
                        if (LBuffer != m_pWriteBuffer)
                            delete LBuffer;
                    } else {
                        m_pWriteBuffer->WriteBuffer(ABuffer, AByteCount);
                        if (m_WriteBufferThreshold > 0 && m_pWriteBuffer->Size() >= (size_t) m_WriteBufferThreshold)
                            FlushWriteBuffer(m_WriteBufferThreshold);
                    }
                } else
                    throw ESocketError(_T("Not Connected."));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CTCPConnection::WriteBufferAsync(void *ABuffer, size_t AByteCount) {
            ssize_t LByteCount = 0;

            if ((AByteCount > 0) && (ABuffer != nullptr)) {
                CheckForDisconnect(true);
                if (Connected()) {
                    LByteCount = m_pIOHandler->Send(ABuffer, AByteCount);
#ifdef WITH_SSL
                    if (m_pIOHandler->UsedSSL()) {
                        unsigned long Ignore[] = { SSL_ERROR_NONE, SSL_ERROR_WANT_WRITE };
                        if (GStack->CheckForSSLError(LByteCount, Ignore, chARRAY(Ignore))) {
                            return 0;
                        }
                    } else {
#endif
                        int Ignore[] = { EAGAIN, EWOULDBLOCK };
                        if (GStack->CheckForSocketError(LByteCount, Ignore, chARRAY(Ignore), egSystem))
                            return 0;
#ifdef WITH_SSL
                    }
#endif
                    CheckWriteResult(LByteCount);
                } else
                    throw ESocketError(_T("Not Connected."));
            }

            return LByteCount;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CTCPConnection::WriteAsync(ssize_t AByteCount) {
            ssize_t LByteCount = AByteCount;

            if (m_pOutputBuffer->Size() > 0) {

                if (AByteCount == -1)
                    AByteCount = m_pOutputBuffer->Size();

                LByteCount = WriteBufferAsync(m_pOutputBuffer->Memory(), (size_t) AByteCount);

                if (LByteCount > 0 )
                    m_pOutputBuffer->Remove((size_t) LByteCount);
            }

            return (LByteCount == AByteCount);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::WriteInteger(int AValue, bool AConvert) {
            if (AConvert)
                AValue = (int) GStack->HToNL((unsigned int) AValue);
            WriteBuffer(&AValue, sizeof(AValue), true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::WriteLn(LPCSTR Format, ...) {
            size_t Len = 0;

            OpenWriteBuffer();
            m_pWriteBuffer->Size(GSendBufferSizeDefault);

            va_list args;
            va_start(args, Format);
            chVERIFY(SUCCEEDED(StringCchVPrintfA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), Format, args)));
            va_end(args);

            chVERIFY(SUCCEEDED(StringCchLengthA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), &Len)));

            if ((((char *) m_pWriteBuffer->Memory())[Len - 2] != INT_CR) && (((char *) m_pWriteBuffer->Memory())[Len - 1] != INT_LF)) {
                chVERIFY(SUCCEEDED(StringCchCatA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), EOLA)));
            } else  if ((((char *) m_pWriteBuffer->Memory())[Len - 1] != INT_LF)) {
                chVERIFY(SUCCEEDED(StringCchCatA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), "\n\0")));
            }

            chVERIFY(SUCCEEDED(StringCchLengthA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), &Len)));
            m_pWriteBuffer->Size(Len);

            CloseWriteBuffer();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::WriteStrings(CStrings *AValue, bool AWriteLinesCount) {
            int LWriteLinesCount;
            if (Assigned(AValue)) {
                LWriteLinesCount = AValue->Count();
                if (AWriteLinesCount)
                    WriteInteger(LWriteLinesCount);
                for (int i = 0; i < LWriteLinesCount; ++i) {
                    WriteLn(AValue->Strings(i).c_str());
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::Write(LPCSTR Format, ...) {
            size_t Len = 0;

            OpenWriteBuffer();
            m_pWriteBuffer->Size(GSendBufferSizeDefault);

            va_list args;
            va_start(args, Format);
            chVERIFY(SUCCEEDED(StringCchVPrintfA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), Format, args)));
            va_end(args);

            chVERIFY(SUCCEEDED(StringCchLengthA((char *) m_pWriteBuffer->Memory(), m_pWriteBuffer->Size(), &Len)));
            m_pWriteBuffer->Size(Len);

            CloseWriteBuffer();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::WriteStream(CStream *AStream, bool AAll, bool AWriteByteCount, const size_t ASize) {

            CMemoryStream *LBuffer;
            size_t LSize, LStreamEnd;

            if (AAll)
                AStream->Position(0);

            // This is copied to a local var because accessing .Size is very inefficient
            if (ASize == 0)
                LStreamEnd = AStream->Size();
            else
                LStreamEnd = ASize + AStream->Position();

            LSize = LStreamEnd - AStream->Position();
            if (AWriteByteCount)
                WriteInteger((int) LSize);

            BeginWork(wmWrite, LSize);
            try {
                LBuffer = CMemoryStream::Create();
                try {
                    LBuffer->SetSize(m_SendBufferSize);
                    while (true) {
                        LSize = Min(LStreamEnd - AStream->Position(), m_SendBufferSize);
                        if (LSize == 0)
                            break;

                        // Do not use ReadBuffer. Some source streams are real time and will not
                        // return as much data as we request. Kind of like recv()
                        // NOTE: We use .Size - size must be supported even if real time
                        LSize = AStream->Read(LBuffer->Memory(), LSize);
                        if (LSize == 0)
                            throw Exception::Exception(_T("No data to read."));
                        WriteBuffer(LBuffer->Memory(), LSize);
                    }
                } catch (...) {
                    FreeAndNil(LBuffer);
                    throw;
                }
                FreeAndNil(LBuffer);
            } catch (...) {
                EndWork(wmWrite);
                throw;
            }
            EndWork(wmWrite);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::ReadBuffer(void *ABuffer, size_t AByteCount) {
            if ((AByteCount > 0) && (ABuffer != nullptr)) {
                // Read from stack until we have enough data
                while (InputBuffer()->Size() < AByteCount) {
                    ReadFromStack();
                    CheckForDisconnect(true);
                }
                // Copy it to the callers buffer
                ::MoveMemory(ABuffer, InputBuffer()->Memory(), AByteCount);
                // Remove used data from buffer
                InputBuffer()->Remove(AByteCount);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CTCPConnection::ReadInteger(bool AConvert) {
            int Result = 0;
            ReadBuffer(&Result, sizeof(Result));
            if (AConvert)
                Result = (int) GStack->NToHL((uint32_t) Result);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CTCPConnection::ReadLn(char *AStr, char ATerminator, int ATimeout, int AMaxLineLength) {

            size_t LInputBufferSize;
            size_t LSize, LTermPos;

            char LTerminator[2] = {};

            LTerminator[0] = ATerminator;

            if (AMaxLineLength == -1)
                AMaxLineLength = (int) MaxLineLength();

            // User may pass '' if they need to pass arguments beyond the first.
            if (ATerminator == 0)
                ATerminator = INT_LF;

            m_ReadLnSplit = false;
            m_ReadLnTimedOut = false;

            LTermPos = 0;
            LSize = 0;

            do {
                LInputBufferSize = InputBuffer()->Size();
                if (LInputBufferSize > 0) {
                    LTermPos = MemoryPos(LTerminator, ((char *) InputBuffer()->Memory()) + LSize, LInputBufferSize - LSize);
                    if (LTermPos > 0)
                        LTermPos = LTermPos + LSize;
                    LSize = LInputBufferSize;
                }

                if ((LTermPos != 0) && ((LTermPos - 1) > AMaxLineLength) && (AMaxLineLength != 0)) {
                    if (MaxLineAction() == maException)
                        throw ESocketError(_T("ReadLn Max Line Length Exceeded."));
                    else {
                        m_ReadLnSplit = true;
                        return InputBuffer()->Extract(AStr, (size_t) AMaxLineLength);
                    }
                    // ReadFromStack blocks - do not call unless we need to
                } else if (LTermPos == 0) {
                    if ((LSize > AMaxLineLength) && (AMaxLineLength != 0)) {
                        if (MaxLineAction() == maException)
                            throw ESocketError(_T("ReadLn Max Line Length Exceeded."));
                        else {
                            m_ReadLnSplit = true;
                            return InputBuffer()->Extract(AStr, (size_t) AMaxLineLength);
                        }
                    }
                    // ReadLn needs to call this as data may exist in the buffer, but no EOL yet disconnected
                    CheckForDisconnect(true);

                    // Can only return 0 if error or timeout
                    m_ReadLnTimedOut = (ReadFromStack(true, ATimeout == TimeoutDefault) == 0);
                    if (ReadLnTimedOut())
                        return 0;
                }
            } while (LTermPos == 0);

            // Extract actual data
            LTermPos = InputBuffer()->Extract(AStr, (size_t) LTermPos) - 1;
            if ((ATerminator == INT_LF) && (LTermPos > 0) && (AStr[LTermPos - 1] == INT_CR))
                LTermPos--;

            AStr[LTermPos] = 0;

            return LTermPos;
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CTCPConnection::ReadLnWait(char *AStr, size_t ABytes, int AFailCount) {
            size_t RStr = 0;
            size_t LStr = 0;
            int LAttempts = 0;

            auto *LStream = new CMemoryStream();
            LStream->SetSize(ABytes);

            while ((LStr == 0) && (LAttempts < AFailCount)) {
                LAttempts++;
                LStr = ReadLn((char *) LStream->Memory());
                LStream->SetSize(RStr + LStr + 1);
                LStream->ReadBuffer(AStr + RStr, Min(LStr, ABytes));
                RStr += LStr;
            }

            delete LStream;

            return RStr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::ReadString(char *AStr, size_t ABytes) {
            auto *LStream = new CMemoryStream();
            LStream->SetSize(ABytes + 1);

            if (ABytes > 0)
                ReadBuffer(LStream->Memory(), ABytes);

            LStream->ReadBuffer(AStr, ABytes);
            AStr[ABytes] = 0;

            delete LStream;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::ReadStrings(CStrings *AValue, int AReadLinesCount) {
            CMemoryStream *LStream = nullptr;

            try {
                LStream = new CMemoryStream();
                LStream->SetSize(MaxLineLength());

                if (AReadLinesCount <= 0)
                    AReadLinesCount = ReadInteger();

                for (int i = 0; i < AReadLinesCount; ++i) {
                    LStream->SetSize(ReadLn((char *) LStream->Memory()));
                    AValue->Add((char *) LStream->Memory());
                }
            } catch (...) {
            }

            delete LStream;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CTCPConnection::CheckReadStack(ssize_t AByteCount) {
            m_ClosedGracefully = (AByteCount == 0);

            if (!ClosedGracefully()) {
#ifdef WITH_SSL
                if (m_pIOHandler != nullptr && m_pIOHandler->UsedSSL()) {
                    unsigned long Ignore[] = { SSL_ERROR_NONE, SSL_ERROR_WANT_READ, SSL_ERROR_SYSCALL };
                    if (GStack->CheckForSSLError(AByteCount, Ignore, chARRAY(Ignore))) {
                        AByteCount = 0;
                        if (GStack->SSLError() == SSL_ERROR_SYSCALL) {
                            if (m_pIOHandler != nullptr)
                                DisconnectSocket();
                            if (InputBuffer()->Size() == 0)
                                GStack->RaiseSocketError(GStack->SSLError());
                        }
                    }
                } else {
#endif
                    int Ignore[] = { ESHUTDOWN, ECONNABORTED, ECONNRESET };
                    if (GStack->CheckForSocketError(AByteCount, Ignore, chARRAY(Ignore), egSystem)) {
                        AByteCount = 0;
                        if (m_pIOHandler != nullptr)
                            DisconnectSocket();
                        if (InputBuffer()->Size() == 0)
                            GStack->RaiseSocketError(GStack->LastError());
                    }
#ifdef WITH_SSL
                }
#endif
                if (AByteCount > 0) {
                    m_pRecvBuffer->Size((size_t) AByteCount);
                    m_pInputBuffer->Seek(0, soEnd);
                    m_pInputBuffer->WriteBuffer(m_pRecvBuffer->Memory(), m_pRecvBuffer->Size());
                }
            }

            return AByteCount;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CTCPConnection::ReadFromStack(bool ARaiseExceptionIfDisconnected, bool ARaiseExceptionOnTimeout) {

            ssize_t LByteCount = 0;
            ssize_t Result = 0;

            CheckForDisconnect(ARaiseExceptionIfDisconnected);
            if (Connected()) {
                do {
                    if (Connected() && (m_pRecvBuffer != nullptr) && (IOHandler() != nullptr)) { //APR: disconnect from other thread

                        m_pRecvBuffer->Size(RecvBufferSize());
                        LByteCount = m_pIOHandler->Recv(m_pRecvBuffer->Memory(), m_pRecvBuffer->Size());
#ifdef WITH_SSL
                        if (m_pIOHandler->UsedSSL()) {
                            if (LByteCount <= 0) {
                                Result = LByteCount;
                                unsigned long LastError = GStack->GetSSLError();
                                if (LastError == SSL_CTRL_SESS_TIMEOUTS) {
                                    if (ARaiseExceptionOnTimeout)
                                        throw ESocketError(_T("Read Timeout"));
                                    return 0;
                                } else if (LastError == SSL_ERROR_NONE) {
                                    continue;
                                } else {
                                    break;
                                }
                            }
                        } else {
#endif
                            if (LByteCount == SOCKET_ERROR) {
                                Result = LByteCount;
                                int LastError = GStack->GetLastError();
                                if (LastError == ETIMEDOUT) {
                                    if (ARaiseExceptionOnTimeout)
                                        throw ESocketError(_T("Read Timeout"));
                                    return 0;
                                }

                                break;
                            }
#ifdef WITH_SSL
                        }
#endif
                    } else {
                        LByteCount = 0;
                        if (ARaiseExceptionIfDisconnected)
                            throw ESocketError(_T("Not Connected"));
                    }

                    Result = CheckReadStack(LByteCount);
                    CheckForDisconnect(ARaiseExceptionIfDisconnected);

                } while ((LByteCount == 0) && Connected());
            } else {
                if (ARaiseExceptionIfDisconnected)
                    throw ESocketError(_T("Not Connected"));
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CTCPConnection::ReadAsync(bool ARaiseExceptionIfDisconnected) {
            ssize_t LByteCount = 0;
            ssize_t LByteRecv = 0;

            CheckForDisconnect(ARaiseExceptionIfDisconnected);

            if (Connected() && (m_pRecvBuffer != nullptr) && (IOHandler() != nullptr)) { //APR: disconnect from other thread
                m_pRecvBuffer->Size(RecvBufferSize());
                do {
                    LByteRecv = IOHandler()->Recv(m_pRecvBuffer->Memory(), m_pRecvBuffer->Size());
#ifdef WITH_SSL
                    if (m_pIOHandler->UsedSSL()) {
                        unsigned long Ignore[] = { SSL_ERROR_NONE, SSL_ERROR_WANT_READ };
                        if (GStack->CheckForSSLError(LByteRecv, Ignore, chARRAY(Ignore)))
                            return LByteCount;
                    } else {
#endif
                        int Ignore[] = { EAGAIN, EWOULDBLOCK };
                        if (GStack->CheckForSocketError(LByteRecv, Ignore, chARRAY(Ignore), egSystem))
                            return LByteCount;
#ifdef WITH_SSL
                    }
#endif
                    LByteCount += CheckReadStack(LByteRecv);
                } while (LByteRecv > 0);
            } else {
                if (ARaiseExceptionIfDisconnected)
                    throw ESocketError(_T("Not Connected."));
            }

            return LByteCount;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPConnection::DoDisconnected() {
            if (m_OnDisconnected != nullptr)
                m_OnDisconnected(this);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPServerConnection --------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPServerConnection::CTCPServerConnection(CPollSocketServer *AServer):
                CTCPConnection(AServer) {
            m_pServer = AServer;
        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPServerConnection::~CTCPServerConnection() {
            m_pServer = nullptr;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPClientConnection --------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPClientConnection::CTCPClientConnection(CPollSocketClient *AClient):
                CTCPConnection(AClient) {
            m_pClient = AClient;
            if (Assigned(m_pClient))
                m_pClient->IncConnection();
        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPClientConnection::~CTCPClientConnection() {
            if (Assigned(m_pClient))
                m_pClient->DecConnection();
            m_pClient = nullptr;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CServerIOHandler ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CIOHandlerSocket *CServerIOHandler::Accept(CSocket ASocket, int AFlags) {
            CIOHandlerSocket *Result = nullptr;

            auto pIOHandler = new CIOHandlerSocket();
#ifdef WITH_SSL
            pIOHandler->Open(sslNotUsed);
#else
            pIOHandler->Open();
#endif
            if (pIOHandler->Binding()->Accept(ASocket, AFlags))
                return pIOHandler;
            else {
                FreeAndNil(pIOHandler);
                Result = nullptr;
            }

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CClientIOHandler ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_SSL
        CClientIOHandler::CClientIOHandler(CSSLMethod AMethod): CIOHandlerSocket() {
            m_MaxTries = 5;
            m_Attempts = 0;
            CClientIOHandler::Open(AMethod);
        }
        //--------------------------------------------------------------------------------------------------------------
#else
        CClientIOHandler::CClientIOHandler(): CIOHandlerSocket() {
            m_MaxTries = 5;
            m_Attempts = 0;
            CClientIOHandler::Open();
        }
        //--------------------------------------------------------------------------------------------------------------
#endif
        CClientIOHandler::~CClientIOHandler() {
            CClientIOHandler::Close();
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CClientIOHandler::Connect(LPCSTR AHost, unsigned short APort) {

            int ErrorCode = m_pBinding->Connect(AF_INET, AHost, APort);

            if (ErrorCode == -1) {
                m_MaxTries++;
                if (m_MaxTries == m_Attempts) {
                    m_Attempts = 0;
                    throw ESocketError(ENOTCONN);
                }
            }

            return (ErrorCode == 0) || (CSocketHandle::LastError() == EISCONN);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketEvent ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketEvent::CSocketEvent() {
            m_OnVerbose = nullptr;
            m_OnAccessLog = nullptr;
            m_OnExecute = nullptr;
            m_OnException = nullptr;
            m_OnListenException = nullptr;
            m_OnConnected = nullptr;
            m_OnDisconnected = nullptr;
            m_OnAfterCommandHandler = nullptr;
            m_OnBeforeCommandHandler = nullptr;
            m_OnNoCommandHandler = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoVerbose(CTCPConnection *AConnection, LPCTSTR AFormat, ...) {
            va_list args;
            if (m_OnVerbose != nullptr) {
                va_start(args, AFormat);
                m_OnVerbose(this, AConnection, AFormat, args);
                va_end(args);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoVerbose(CTCPConnection *AConnection, LPCTSTR AFormat, va_list args) {
            if (m_OnVerbose != nullptr) {
                m_OnVerbose(this, AConnection, AFormat, args);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoAfterCommandHandler(CTCPConnection *AConnection) {
            if (m_OnAfterCommandHandler != nullptr)
                m_OnAfterCommandHandler(this, AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoBeforeCommandHandler(CTCPConnection *AConnection, const CString &Line) {
            if (m_OnBeforeCommandHandler != nullptr)
                m_OnBeforeCommandHandler(this, Line, AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoAccessLog(CTCPConnection *AConnection) {
            if (m_OnAccessLog != nullptr)
                m_OnAccessLog(AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoConnected(CObject *Sender) {
            if (m_OnConnected != nullptr)
                m_OnConnected(Sender);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoDisconnected(CObject *Sender) {
            if (m_OnDisconnected != nullptr)
                m_OnDisconnected(Sender);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoException(CTCPConnection *AConnection, const Delphi::Exception::Exception &E) {
            if (m_OnException != nullptr)
                m_OnException(AConnection, E);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoListenException(const Delphi::Exception::Exception &E) {
            if (m_OnListenException != nullptr)
                m_OnListenException(this, E);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketEvent::DoNoCommandHandler(const CString &Data, CTCPConnection *AConnection) {
            if (m_OnNoCommandHandler != nullptr)
                m_OnNoCommandHandler(this, Data, AConnection);
            else
                throw ETCPServerError(_T("No command handler found."));
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketServer ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketServer::CSocketServer(): CSocketComponent() {
            m_ServerName = WWWServerName;
            m_AllowedMethods = WWWAllowedMethods;
            m_pBindings = new CSocketHandles();
            m_FreeBindings = true;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketServer::~CSocketServer() {
            FreeBindings();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketServer::FreeBindings() {
            if (m_FreeBindings)
                FreeAndNil(m_pBindings);
        }
        //--------------------------------------------------------------------------------------------------------------

        CString &CSocketServer::GetDefaultIP() {
            return m_pBindings->DefaultIP();
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CSocketServer::GetDefaultIP() const {
            return m_pBindings->DefaultIP();
        }
        //--------------------------------------------------------------------------------------------------------------

        unsigned short CSocketServer::GetDefaultPort() const {
            return m_pBindings->DefaultPort();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketServer::SetDefaultPort(unsigned short Value) {
            m_pBindings->DefaultPort(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketHandles *CSocketServer::GetBindings() const {
            return m_pBindings;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketServer::SetBindings(CSocketHandles *Value) {
            if (m_pBindings != Value) {
                FreeBindings();
                m_pBindings = Value;
                m_FreeBindings = false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketClient ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketClient::CSocketClient(): CSocketComponent() {
            m_Port = 0;
            m_ConnectionCount = 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPServer ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CUDPServer::CUDPServer(): CSocketServer() {
            m_Active = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CUDPServer::~CUDPServer() {
            SetActive(false);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPServer::SetActive(bool AValue) {
            CSocketHandle *pSocket = nullptr;

            if (AValue) {
                if (Bindings()->Count() == 0)
                    Bindings()->Add();

                for (int i = 0; i < Bindings()->Count(); ++i) {
                    pSocket = Bindings()->Handles(i);

                    pSocket->AllocateSocket(SOCK_DGRAM, IPPROTO_UDP, 0); // O_NONBLOCK
                    pSocket->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (void *) &SO_True, sizeof(SO_True));
                    pSocket->SetSockOpt(SOL_SOCKET, SO_BROADCAST, (void *) &SO_True, sizeof(SO_True));

                    pSocket->Bind();
                }
            } else {
                for (int i = 0; i < Bindings()->Count(); ++i)
                    Bindings()->Handles(i)->CloseSocket();
            }

            m_Active = AValue;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPClient ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CUDPClient::CUDPClient(): CSocketClient() {
            m_pSocket = new CSocketHandle(nullptr);
            m_Active = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CUDPClient::~CUDPClient() {
            CUDPClient::SetActive(false);
            FreeAndNil(m_pSocket);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPClient::SetActive(bool AValue) {
            if (AValue) {
                m_pSocket->AllocateSocket(SOCK_DGRAM, IPPROTO_UDP, 0); // O_NONBLOCK
                m_pSocket->SetSockOpt(SOL_SOCKET, SO_BROADCAST, (void *) &SO_True, sizeof(SO_True));
            } else {
                m_pSocket->CloseSocket();
            }

            m_Active = AValue;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CSocketThread ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CSocketThread::CSocketThread(bool ACreateSuspended): CThread(ACreateSuspended) {
            //pthread_mutex_init(&m_Lock, nullptr);
            m_Lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

            m_StopMode = smTerminate;

            m_OnException = nullptr;
            m_OnStopped = nullptr;

            m_Stopped = ACreateSuspended;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketThread::~CSocketThread() {
            FreeOnTerminate(false); //prevent destroy between Terminate & WaitFor
            CSocketThread::Cleanup();
            pthread_mutex_destroy(&m_Lock);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::DoException(const Delphi::Exception::Exception &E) {
            if (m_OnException)
                m_OnException(this, E);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::DoStopped() {
            if (m_OnStopped)
                m_OnStopped(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::ExecuteTry() {
            BeforeExecute();
            try {
                while (!Terminated())
                {
                    if (Stopped()) {
                        DoStopped();
                        if (Stopped()) {
                            if (Terminated())
                                break;
                            Suspended(true);
                            if (Terminated())
                                break;
                        }
                    }

                    try {
                        BeforeRun();
                        try {
                            while (!Stopped())
                                Run();
                        } catch (...) {
                            AfterRun();
                            throw;
                        }
                        AfterRun();
                    } catch (...) {
                        Cleanup();
                        throw;
                    }
                    Cleanup();
                }
            } catch (...) {
                AfterExecute();
                throw;
            }
            AfterExecute();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::Execute() {
            try {
                ExecuteTry();
            } catch (Delphi::Exception::Exception &E) {
                DoException(E);
                Terminate();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CSocketThread::GetStopped() {
            bool fSuccess = false;

            pthread_mutex_lock(&m_Lock);
            try
            {
                fSuccess = Terminated() || m_Stopped || Suspended();
            } catch (...) {
            }
            pthread_mutex_unlock(&m_Lock);

            return fSuccess;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::Start() {
            pthread_mutex_lock(&m_Lock);
            try
            {
                if ( Stopped() )
                {
                    // Resume is also called for smTerminate as .Start can be used to initially start a thread that is created suspended
                    m_Stopped = false;
                    Suspended(false);
                }
            } catch (...) {
            }
            pthread_mutex_unlock(&m_Lock);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::Stop() {
            pthread_mutex_lock(&m_Lock);
            try {
                if ( !Stopped() ) {
                    switch (m_StopMode) {
                        case smTerminate:
                            Terminate();
                            break;

                        case smSuspend:
                            m_Stopped = true;
                            break;
                    }
                }
            } catch (...) {
                pthread_mutex_unlock(&m_Lock);
                throw;
            }
            pthread_mutex_unlock(&m_Lock);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::Terminate() {
            m_Stopped = true;
            inherited::Terminate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CSocketThread::TerminateAndWaitFor() {
            if (FreeOnTerminate())
                throw Exception::Exception(_T("Cannot call TerminateAndWaitFor on FreeAndTerminate threads."));

            Terminate();
            if (Suspended())
                Resume();

            WaitFor();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPeerThread -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        void CPeerThread::AfterRun() {
            m_pConnection->Server()->DoDisconnected(m_pConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPeerThread::BeforeRun() {
            try {
                if (Assigned(m_pConnection->IOHandler()))
                    m_pConnection->IOHandler()->AfterAccept();
                else
                    throw ETCPServerError(_T("TCP Server Error..."));
            }
            catch (...) {
                FreeOnTerminate(true);
                throw;
            }

            m_pConnection->Server()->DoConnected(m_pConnection);

            // Stop this thread if we were disconnected
            if (!m_pConnection->Connected())
                Stop();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPeerThread::Cleanup() {
            inherited::Cleanup();
            if (Assigned(m_pConnection)) {
                if (Assigned(m_pConnection->Server())) {

                    auto LServer = (CTCPServer *) m_pConnection->Server();

                    if (Assigned(LServer->Threads()))
                        LServer->Threads()->Remove(this);

                    if (Assigned(LServer->ThreadMgr()))
                        LServer->ThreadMgr()->ReleaseThread(this);
                }
                FreeAndNil(m_pConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPeerThread::Run() {
            try {
                try {
                    if (!m_pConnection->Server()->DoExecute(m_pConnection))
                        throw Exception::Exception(_T("No recv execute handler found."));
                }
                catch (ESocketError &E) {
                    m_pConnection->Server()->DoException(m_pConnection, E);
                    switch (E.GetLastError()) {
                        case ECONNABORTED:
                        case ECONNRESET:
                            Connection()->Disconnect();
                            break;
                        default:
                            break;
                    }
                }

                if (!Connection()->Connected())
                    Stop();

            } catch (Delphi::Exception::Exception &E) {
                m_pConnection->Server()->DoException(m_pConnection, E);
                throw;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CThreadMgr ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CThreadMgr::CThreadMgr() {
            m_pActiveThreads = new CThreadList;
            m_ThreadPriority = tpNormal;
        }
        //--------------------------------------------------------------------------------------------------------------

        CThreadMgr::~CThreadMgr() {
            FreeAndNil(m_pActiveThreads);
        }
        //--------------------------------------------------------------------------------------------------------------

        CPeerThread *CThreadMgr::GetThread() {
            auto Result = new CPeerThread();
            //SetThreadPriority(Result->Handle, ThreadPriority);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CThreadMgr::TerminateThreads() {
            //
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CListenerThread -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CListenerThread::CListenerThread(CTCPServer *AServer, CSocketHandle *ABinding) : CThread(true) {
            m_pServer = AServer;
            m_pBinding = ABinding;

            FreeOnTerminate(false);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CListenerThread::Execute() {

            CIOHandler *pIOHandler = nullptr;
            CTCPServerConnection *pPeer = nullptr;
            CPeerThread *pThread = nullptr;

            while (!Terminated()) {
                try {
                    if (Assigned(m_pServer)) {
                        pThread = nullptr;
                        pPeer = new CTCPServerConnection(m_pServer);
                        pIOHandler = Server()->IOHandler()->Accept(Binding()->Handle(), 0); // SOCK_NONBLOCK

                        if (pIOHandler == nullptr) {
                            FreeAndNil(pPeer);
                            Terminate();
                        } else {
                            pThread = Server()->ThreadMgr()->GetThread();
                            pThread->m_pConnection = pPeer;
                            pThread->m_pConnection->IOHandler(pIOHandler);

                            m_pServer->Threads()->Add(pThread);
                            pThread->Start();
                        }
                    }
                }
                catch (Delphi::Exception::Exception &E) {
                    if (Assigned(pThread))
                        FreeAndNil(pThread);
                    m_pServer->DoListenException(E);
                }
            }

            m_pBinding->CloseSocket();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommandHandler -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCommandHandler::CCommandHandler(CCollection *ACollection): CCollectionItem(ACollection) {
            m_Command = nullptr;
            m_Data = nullptr;
            m_Disconnect = false;

            m_CmdDelimiter = 32;
            m_ParamDelimiter = 32;

            m_Enabled = true;
            m_ParseParams = true;

            m_ReplyExceptionCode = 0;

            m_OnCommand = nullptr;
            m_OnException = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CCommandHandler::Check(LPCTSTR AData, size_t ASize, CTCPConnection *AConnection) {
            CString sCommand;
            sCommand.Write(AData, ASize);
            return Check(sCommand, AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CCommandHandler::Check(const CString &Data, CTCPConnection *AConnection) {
            bool Result = false;

            CString sCommand;
            CString sUnparsedParams;

            try {
                Result = m_Command == Data;

                if (!Result) {
                    size_t Len = m_Command.Length();

                    if (Len >= Data.Length())
                        return false;

                    if (CmdDelimiter() != 0) {
                        sCommand = Data;
                        Result = (sCommand[Len] == m_CmdDelimiter);
                        if (Result) {
                            sUnparsedParams = sCommand.SubString(Len);
                            sCommand.SetLength(Len);
                            sCommand.Truncate();
                            Result = m_Command == sCommand;
                        }
                    } else {
                        // Dont strip any part of the params out.. - just remove the command purely on length and no delimiter
                        sCommand = Data;
                        sUnparsedParams = sCommand.SubString(Len);
                        sCommand.SetLength(Len);
                        sCommand.Truncate();
                        Result = m_Command == sCommand;
                    }
                }

                if (Result) {
                    auto pCommand = new CCommand(this);

                    pCommand->m_pConnection = AConnection;
                    pCommand->m_RawLine = Data;
                    pCommand->m_UnparsedParams = sUnparsedParams;

                    if (ParseParams()) {
                        pCommand->m_Params.Clear();
                        SplitColumns(sUnparsedParams, pCommand->m_Params, m_ParamDelimiter);
                    }

                    pCommand->PerformReply(true);

                    try {
                        pCommand->DoCommand();
                    } catch (Delphi::Exception::Exception &E) {
                        DoException(AConnection, E);
                    }

                    delete pCommand;

                    if (Disconnect())
                        AConnection->Disconnect();
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(AConnection, E);
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCommandHandler::DoException(CTCPConnection *AConnection, const Delphi::Exception::Exception &E) {
            if (m_OnException)
                m_OnException(AConnection, E);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommandHandlers ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCommandHandlers::CCommandHandlers(): CCollection(this) {
            m_pOwner = nullptr;
            m_EnabledDefault = true;
            m_ParseParamsDefault = true;
            m_DisconnectDefault = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CCommandHandlers::CCommandHandlers(CSocketServer *AServer): CCommandHandlers() {
            m_pOwner = AServer;
        }
        //--------------------------------------------------------------------------------------------------------------

        CCommandHandlers::CCommandHandlers(CSocketClient *AClient): CCommandHandlers() {
            m_pOwner = AClient;
        }
        //--------------------------------------------------------------------------------------------------------------

        CCommandHandler *CCommandHandlers::GetItem(int AIndex) const {
            return (CCommandHandler *) inherited::GetItem(AIndex);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCommandHandlers::SetItem(int AIndex, const CCommandHandler *AValue) {
            inherited::SetItem(AIndex, (CCollectionItem *) AValue);
        }
        //--------------------------------------------------------------------------------------------------------------

        CCommandHandler *CCommandHandlers::Add() {
            auto Result = new CCommandHandler(this);

            Result->Enabled(m_EnabledDefault);
            Result->ParseParams(m_ParseParamsDefault);
            Result->Disconnect(m_DisconnectDefault);

            inherited::Added((CCollectionItem *) Result);
            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCommand --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCommand::CCommand(CCommandHandler *ACommandHandler): CObject() {
            m_pConnection = nullptr;
            m_pCommandHandler = ACommandHandler;

            m_PerformReply = false;
            m_RawLine = nullptr;
            m_UnparsedParams = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCommand::DoCommand() {
            if (m_pCommandHandler->m_OnCommand != nullptr)
                m_pCommandHandler->m_OnCommand(this);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPServer ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPServer::CTCPServer(): CPollSocketServer() {
            m_pThreadMgr = nullptr;
            m_pListenerThreads = nullptr;
            m_pIOHandler = nullptr;

            m_pThreads = new CThreadSafeList();
            m_pCommandHandlers = new CCommandHandlers(this);

            m_Active = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPServer::~CTCPServer() {
            SetActive(false);

            FreeAndNil(m_pThreads);
            FreeAndNil(m_pCommandHandlers);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPServer::SetActive(bool AValue) {
            CListenerThread *LListenerThread;

            if (m_Active != AValue ) {

                if (AValue) {
                    InitializeCommandHandlers();

                    m_pIOHandler = new CServerIOHandler();
                    m_pListenerThreads = new CThreadList;

                    if (Bindings()->Count() == 0)
                        Bindings()->Add();

                    for (int i = 0; i < Bindings()->Count(); ++i) {
                        Bindings()->Handles(i)->AllocateSocket(SOCK_STREAM, IPPROTO_IP, 0);
                        Bindings()->Handles(i)->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (void *) &SO_True, sizeof(SO_True));
                        Bindings()->Handles(i)->Bind();
                        Bindings()->Handles(i)->Listen(SOMAXCONN);

                        LListenerThread = new CListenerThread(this, Bindings()->Handles(i));
                        m_pListenerThreads->Add(LListenerThread);
                        LListenerThread->Resume();
                    }
                } else {
                    TerminateListenerThreads();

                    try {
                        TerminateAllThreads();
                    } catch (...) {
                    }

                    FreeAndNil(m_pListenerThreads);
                    FreeAndNil(m_pIOHandler);
                }

                m_Active = AValue;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPServer::SetIOHandler(CServerIOHandler *Value) {
            if (Assigned(m_pIOHandler))
                FreeAndNil(m_pIOHandler);
            m_pIOHandler = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        CThreadMgr *CTCPServer::GetThreadMgr() {
            if (m_pThreadMgr == nullptr)
                m_pThreadMgr = new CThreadMgrDefault;
            return m_pThreadMgr;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CTCPServer::IndexOfThreadId(pid_t dwThreadId) {
            int Index = 0;
            CList *pThreads;

            if (Assigned(Threads())) {
                pThreads = Threads()->LockList();
                try {
                    while ((Index < pThreads->Count()) && (static_cast<CPeerThread *> (pThreads->Items(Index))->ThreadId() != dwThreadId))
                        Index++;
                    if ( Index == pThreads->Count() )
                        Index = -1;
                } catch (...) {
                }
                Threads()->UnlockList();

                return Index;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPeerThread *CTCPServer::FindThread(pid_t dwThreadId) {
            CPeerThread *pThread = nullptr;
            CList *pThreads;

            int Index = IndexOfThreadId(dwThreadId);

            if (Index != -1 && Assigned(Threads())) {
                pThreads = Threads()->LockList();
                try {
                    pThread = static_cast<CPeerThread *> (pThreads->Items(Index));
                } catch (...) {
                }
                Threads()->UnlockList();
            }

            return pThread;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPServer::TerminateAllThreads() {
            useconds_t LSleepTime = 250;
            useconds_t LTerminateWaitTime = 5000;

            CList *pThreads;
            bool LTimedOut;

            if (Assigned(Threads())) {
                pThreads = Threads()->LockList();
                try {
                    for (int i = 0; i < pThreads->Count(); ++i )
                        static_cast<CPeerThread *>(pThreads->Items(i))->Connection()->DisconnectSocket();
                } catch (...) {
                }
                Threads()->UnlockList();

                LTimedOut = true;
                for (useconds_t i = 1; i < (LTerminateWaitTime / LSleepTime); ++i) {
                    usleep(LSleepTime);
                    if (Threads()->IsCountLessThan(1)) {
                        LTimedOut = false;
                        break;
                    }
                }
                if (LTimedOut)
                    throw ETCPServerError(_T("Terminate Thread Timeout."));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPServer::TerminateListenerThreads() {
            CListenerThread *pListenerThread;
            CList *pListenerThreads;

            if (Assigned(m_pListenerThreads)) {
                pListenerThreads = m_pListenerThreads->LockList();
                try {
                    for (int i = 0; i < pListenerThreads->Count(); ++i ) {
                        pListenerThread = (CListenerThread *) pListenerThreads->Items(i);
                        pListenerThread->Terminate();
                        pListenerThread->Binding()->CloseSocket(false);
                        pListenerThread->WaitFor();
                        delete pListenerThread;
                    }
                } catch (...) {
                }
                m_pListenerThreads->UnlockList();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CTCPServer::DoExecute(CTCPConnection *AConnection) {
            int i;
            bool Result = false;

            CMemoryStream LStream;

            if (CommandHandlers()->Count() > 0) {
                Result = true;

                if (AConnection->Connected()) {
                    LStream.SetSize(MaxLineLengthDefault);
                    LStream.SetSize(AConnection->ReadLn((char *) LStream.Memory()));

                    if (LStream.Size() > 0 && LStream.Size() < MaxLineLengthDefault) {
#ifdef _UNICODE
                        wchar_t *WLine = new wchar_t[LStream->Size() + 1];

                        if ( CharToWide((char *) LStream->Memory(), WLine, LStream->Size() + 1) )
                        {
                            DoBeforeCommandHandler(AThread, WLine);
#else
                        DoBeforeCommandHandler(AConnection, (char *) LStream.Memory());
#endif
                        try {
                            for (i = 0; i < CommandHandlers()->Count(); ++i) {
                                if (CommandHandlers()->Commands(i)->Enabled()) {
#ifdef _UNICODE
                                    if (CommandHandlers()->Items(i)->Check(WLine, LStream.Size(), AThread))
#else
                                    if (CommandHandlers()->Commands(i)->Check((char *) LStream.Memory(), LStream.Size(), AConnection))
#endif
                                        break;
                                }
                            }

                            if (i == CommandHandlers()->Count())
#ifdef _UNICODE
                                DoNoCommandHandler(WLine, AThread);
#else
                                DoNoCommandHandler((char *) LStream.Memory(), AConnection);
#endif
                        }
                        catch (...) {
                        }

                        DoAfterCommandHandler(AConnection);
#ifdef _UNICODE
                        }

          delete [] WLine;
#endif
                    }
                }
            } else {
                if (m_OnExecute != nullptr) {
                    m_OnExecute(AConnection);
                    Result = true;
                }
            }

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollStack ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPollStack::CPollStack(size_t AEventSize): CObject() {
            m_Handle = INVALID_SOCKET;
            m_TimeOut = INFINITE;
            m_pEventList = nullptr;
            m_EventSize = AEventSize;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollStack::~CPollStack() {
            Close();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Assign(CPollStack *Source) {
            Close();

            m_Handle = Source->m_Handle;
            m_TimeOut = Source->m_TimeOut;
            m_EventSize = Source->m_EventSize;
        }
        //--------------------------------------------------------------------------------------------------------------

        CSocketPoll CPollStack::Create(int AFlags) {
            m_Handle = ::epoll_create1(AFlags);

            if (m_Handle == INVALID_SOCKET)
                throw ESocketError(errno, _T("epoll: create failure: "));

            return m_Handle;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Close() {
            if (m_Handle != INVALID_SOCKET) {
                ::close(m_Handle);
                m_Handle = INVALID_SOCKET;
            }

            delete [] m_pEventList;
            m_pEventList = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Ctl(int AOption, CPollEventHandler *AEventHandler) {
            CSocket Socket;
            struct epoll_event ee = {};

            if (!Assigned(AEventHandler))
                throw Exception::Exception(_T("Event handler cannot be empty."));

            if (m_Handle == INVALID_SOCKET)
                Create(0);

            Socket = AEventHandler->Socket();

            ee.events = AEventHandler->Events();
            ee.data.fd = Socket;

            if (AOption == EPOLL_CTL_DEL) {
                ee.data.ptr = nullptr;
            } else {
                ee.data.ptr = AEventHandler;
            }

            if (epoll_ctl(m_Handle, AOption, Socket, &ee) != 0)
                throw ESocketError(errno, _T("epoll: control operations (%d) for socket (%d) failure: "), AOption, Socket);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Add(CPollEventHandler *AEventHandler) {
            Ctl(EPOLL_CTL_ADD, AEventHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Mod(CPollEventHandler *AEventHandler) {
            Ctl(EPOLL_CTL_MOD, AEventHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollStack::Del(CPollEventHandler *AEventHandler) {
            Ctl(EPOLL_CTL_DEL, AEventHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CPollStack::Wait(const sigset_t *ASigMask) {
            int Result;

            if (m_Handle == INVALID_SOCKET)
                Create(0);

            if (!Assigned(m_pEventList))
                m_pEventList = new CPollEvent[m_EventSize];

            if (ASigMask == nullptr)
                Result = epoll_wait(m_Handle, m_pEventList, m_EventSize, m_TimeOut);
            else
                Result = epoll_pwait(m_Handle, m_pEventList, m_EventSize, m_TimeOut, ASigMask);

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEvent *CPollStack::GetEvent(int Index) {
            if ((Index < 0) || (m_pEventList == nullptr) || (Index >= m_EventSize))
                throw ExceptionFrm(SListIndexError, Index);
            return &m_pEventList[Index];
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollEventHandler -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler::CPollEventHandler(CPollEventHandlers *AEventHandlers, CSocket ASocket):
            CCollectionItem(AEventHandlers) {
            m_Socket = ASocket;
            m_Events = 0;
            m_EventType = etNull;
            m_pBinding = nullptr;
            m_pEventHandlers = AEventHandlers;
            m_OnTimeOutEvent = nullptr;
            m_OnConnectEvent = nullptr;
            m_OnReadEvent = nullptr;
            m_OnWriteEvent = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler::~CPollEventHandler() {
            Stop();
            ClearBinding();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::ClearBinding() {
            CPollConnection *Temp;
            if (Assigned(m_pBinding)) {
                Temp = m_pBinding;
                try {
                    m_pBinding->Disconnect();
                } catch (...) {

                }
                m_pBinding = nullptr;
                if (Temp->AutoFree())
                    delete Temp;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::SetEventType(CPollEventType Value) {
            if (m_EventType != Value) {
                switch (Value) {
                    case etNull:
                        m_Events = 0;
                        break;
                    case etTimer:
                    case etAccept:
                        m_Events = EPOLLIN;
                        m_pEventHandlers->PollAdd(this);
                        break;
                    case etConnect:
                        m_Events = EPOLLOUT;
                        m_pEventHandlers->PollAdd(this);
                        break;
                    case etIO:
                        m_Events = EPOLLIN | EPOLLOUT | EPOLLET;
                        if (m_EventType == etNull)
                            m_pEventHandlers->PollAdd(this);
                        else
                            m_pEventHandlers->PollMod(this);
                        break;
                    case etDelete:
                        m_Events = 0;
                        if (m_EventType != etNull)
                            m_pEventHandlers->PollDel(this);
                        break;
                }
                m_EventType = Value;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::SetBinding(CPollConnection *Value) {
            if (m_pBinding != Value) {
                m_pBinding = Value;
                if (Value != nullptr) {
                    Value->EventHandler(this);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::Start(CPollEventType AEventType) {
            SetEventType(AEventType);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::Stop() {
            SetEventType(etDelete);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::DoTimerEvent() {
            if (m_OnTimerEvent != nullptr)
                m_OnTimerEvent(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::DoTimeOutEvent() {
            if (m_OnTimeOutEvent != nullptr)
                m_OnTimeOutEvent(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::DoConnectEvent() {
            if (m_OnConnectEvent != nullptr)
                m_OnConnectEvent(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::DoReadEvent() {
            if (m_OnReadEvent != nullptr)
                m_OnReadEvent(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandler::DoWriteEvent() {
            if (m_OnWriteEvent != nullptr)
                m_OnWriteEvent(this);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CPollEventHandlers ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandlers::CPollEventHandlers(CPollStack *APollStack): CCollection(this) {
            m_pPollStack = APollStack;
            m_OnException = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CPollEventHandlers::GetItem(int AIndex) const {
            return (CPollEventHandler *) inherited::GetItem(AIndex);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandlers::SetItem(int AIndex, CPollEventHandler *AValue) {
            inherited::SetItem(AIndex, (CCollectionItem *) AValue);
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CPollEventHandlers::Add(CSocket ASocket) {
            auto Result = new CPollEventHandler(this, ASocket);
            inherited::Added(Result);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandlers::PollAdd(CPollEventHandler *AHandler) {
            try {
                m_pPollStack->Add(AHandler);
            } catch (Delphi::Exception::Exception &E) {
                DoException(AHandler, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandlers::PollMod(CPollEventHandler *AHandler) {
            try {
                m_pPollStack->Mod(AHandler);
            } catch (Delphi::Exception::Exception &E) {
                DoException(AHandler, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandlers::PollDel(CPollEventHandler *AHandler) {
            try {
                m_pPollStack->Del(AHandler);
            } catch (Delphi::Exception::Exception &E) {
                DoException(AHandler, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CPollEventHandlers::FindHandlerBySocket(CSocket ASocket) {
            CPollEventHandler *pHandler = nullptr;
            for (int I = 0; I < Count(); ++I) {
                pHandler = Handlers(I);
                if (pHandler->Socket() == ASocket)
                    return pHandler;
            }
            return pHandler;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPollEventHandlers::DoException(CPollEventHandler *AHandler, const Delphi::Exception::Exception &E) {
            if (m_OnException != nullptr)
                m_OnException(AHandler, E);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollTimer ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CEPollTimer::CEPollTimer(int AClockId, int AFlags): CHandleStream(INVALID_HANDLE_VALUE),
            CPollConnection(this) {

            m_AutoFree = true;

            m_ClockId = AClockId;
            m_Flags = AFlags;
            m_OnTimer = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CEPollTimer::~CEPollTimer() {
            ClosePoll();
            Close();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::Open() {
            CreateHandle(::timerfd_create(m_ClockId, m_Flags));
            if (m_Handle == INVALID_HANDLE_VALUE)
                throw EOSError(errno, _T("Could not create file timer. Error: "));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::Close() {
            if (m_Handle != INVALID_HANDLE_VALUE) {
                ::close(m_Handle);
                m_Handle = INVALID_HANDLE_VALUE;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::SetTime(int AFlags, const struct itimerspec *AIn, struct itimerspec *AOut) {
            if (m_Handle == INVALID_HANDLE_VALUE)
                Open();

            if (::timerfd_settime(m_Handle, AFlags, AIn, AOut) == -1) {
                Close();
                throw EOSError(errno, _T("Could not set file time. Error: "));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::GetTime(struct itimerspec *AOut) {
            if (m_Handle == INVALID_HANDLE_VALUE)
                Open();

            if (timerfd_gettime(m_Handle, AOut) == -1) {
                Close();
                throw EOSError(errno, _T("Could not get file time error: "));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CEPollTimer::AllocateTimer(CPollEventHandlers *AEventHandlers, long int Value, long int Interval, int Flags) {
            CPollEventHandler *Handler;

            SetTimer(Value, Interval, Flags);

            Handler = AEventHandlers->Add(m_Handle);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            Handler->OnTimerEvent([this](auto && AHandler) { DoTimer(AHandler); });
#else
            Handler->OnTimerEvent(std::bind(&CEPollTimer::DoTimer, this, _1));
#endif
            Handler->Binding(this);
            Handler->Start(etTimer);

            return Handler;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::SetTimer(long int Value, long int Interval, int Flags) {
            struct itimerspec ts = {0, 0};
            struct timespec now = {0, 0};

            if (Flags == TFD_TIMER_ABSTIME) {
                if (::clock_gettime(ClockId(), &now) == -1)
                    throw EOSError(errno, _T("Could not get clock time. Error: "));
            }

            ts.it_value.tv_sec = now.tv_sec + Value / 1000;
            ts.it_value.tv_nsec = now.tv_nsec;

            ts.it_interval.tv_sec = Interval / 1000;
            ts.it_interval.tv_nsec = 0;

            SetTime(Flags, &ts);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollTimer::DoTimer(CPollEventHandler *AHandler) {
            if (m_OnTimer != nullptr)
                m_OnTimer(AHandler);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPoll ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CEPoll::CEPoll() {
            m_Timer = 0;
            m_pEventHandlers = nullptr;
            m_pPollStack = new CPollStack();
            m_FreeEventHandlers = true;
            m_FreePollStack = true;
            m_OnEventHandlerException = nullptr;

            CreateEventHandlers();
            UpdateTimer();
        }
        //--------------------------------------------------------------------------------------------------------------

        CEPoll::~CEPoll() {
            FreePollStack();
            FreeEventHandlers();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::CreateEventHandlers() {
            m_pEventHandlers = new CPollEventHandlers(m_pPollStack);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            m_pEventHandlers->OnException([this](auto && AHandler, auto && AException) { DoEventHandlersException(AHandler, AException); });
#else
            m_pEventHandlers->OnException(std::bind(&CEPoll::DoEventHandlersException, this, _1, _2));
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::FreeEventHandlers() {
            if (m_FreeEventHandlers) {
                delete m_pEventHandlers;
            }
            m_pEventHandlers = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::FreePollStack() {
            if (m_FreePollStack) {
                delete m_pPollStack;
            }
            m_pPollStack = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::SetEventHandlers(CPollEventHandlers *Value) {
            if (m_pEventHandlers != Value) {
                FreeEventHandlers();
                m_pEventHandlers = Value;
                m_FreeEventHandlers = false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::SetPollStack(CPollStack *Value) {
            if (m_pPollStack != Value) {
                FreePollStack();
                m_pPollStack = Value;
                m_pEventHandlers->PollStack(Value);
                m_FreePollStack = false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CEPoll::GetTimeOut() {
            return m_pPollStack->TimeOut();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::SetTimeOut(int Value) {
            m_pPollStack->TimeOut(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::UpdateTimer() {
            if (m_pPollStack->TimeOut() != INFINITE) {
                m_Timer = Now() + (CDateTime) m_pPollStack->TimeOut() / MSecsPerDay;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::CheckHandler(CPollEventHandler *AHandler) {
            if (AHandler->Stopped()) {
                delete AHandler;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::PackEventHandlers() {
            for (int i = m_pEventHandlers->Count() - 1; i >= 0; i--) {
                CheckHandler(m_pEventHandlers->Handlers(i));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::Wait() {

            int events, err;
            uint32_t uEvents;

            CPollEventHandler *pHandler = nullptr;
            CPollEvent *pPollEvent = nullptr;

            events = m_pPollStack->Wait();

            err = (events == -1) ? errno : 0;

            if (err) {
                throw EOSError(err, _T("epoll: call waits for events failure: "));
            }

            for (int i = 0; i < events; ++i) {

                pPollEvent = m_pPollStack->Events(i);

                pHandler = static_cast<CPollEventHandler *> (pPollEvent->data.ptr);
                uEvents = pPollEvent->events;

                if (uEvents & (EPOLLERR | EPOLLHUP)) {
                    /*
                     * if the error events were returned, add EPOLLIN and EPOLLOUT
                     * to handle the events at least in one active handler
                     */

                    uEvents |= EPOLLIN | EPOLLOUT;
                }

                if (pHandler->EventType() == etAccept) {

                    if (uEvents & EPOLLIN) {
                        if (pHandler->OnReadEvent() != nullptr) {
                            pHandler->DoReadEvent();
                        } else {
                            DoAccept(pHandler);
                        }
                    }

                } else if (pHandler->EventType() == etConnect) {

                    if (uEvents & EPOLLOUT) {
                        if (pHandler->OnConnectEvent() != nullptr) {
                            pHandler->DoConnectEvent();
                        } else {
                            DoConnect(pHandler);
                        }
                    }

                } else if (pHandler->EventType() == etIO) {

                    if (uEvents & EPOLLIN) {
                        if (pHandler->OnReadEvent() != nullptr) {
                            pHandler->DoReadEvent();
                        } else {
                            DoRead(pHandler);
                        }
                    }

                    if (uEvents & EPOLLOUT) {
                        if (pHandler->OnWriteEvent() != nullptr) {
                            pHandler->DoWriteEvent();
                        } else {
                            DoWrite(pHandler);
                        }
                    }

                } else if (pHandler->EventType() == etTimer) {

                    if (uEvents & EPOLLIN) {
                        if (pHandler->OnTimerEvent() != nullptr) {
                            pHandler->DoTimerEvent();
                        }
                    }
                }

                if (pHandler->EventType() != etTimer) {
                    UpdateTimer();
                }

                if ((events == 0) || (m_Timer != 0 && Now() >= m_Timer)) {

                    if (m_pPollStack->TimeOut() != INFINITE) {
                        for (int I = 0; I < m_pEventHandlers->Count(); ++I) {
                            pHandler = m_pEventHandlers->Handlers(I);
                            if (pHandler->EventType() == etIO) {
                                if (pHandler->OnTimeOutEvent() != nullptr) {
                                    pHandler->DoTimeOutEvent();
                                } else {
                                    DoTimeOut(pHandler);
                                }
                            }
                        }

                        PackEventHandlers();
                        UpdateTimer();
                    } else {
                        throw EOSError(err, _T("epoll_wait() returned no events without timeout"));
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPoll::DoEventHandlersException(CPollEventHandler *AHandler, const Delphi::Exception::Exception &E) {
            if (m_OnEventHandlerException != nullptr)
                m_OnEventHandlerException(AHandler, E);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollServer ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CEPollServer::CEPollServer() : CPollSocketServer(), CEPoll() {

        }
        //--------------------------------------------------------------------------------------------------------------

        bool CEPollServer::DoExecute(CTCPConnection *AConnection) {
            if (m_OnExecute != nullptr) {
                return m_OnExecute(AConnection);
            }
            return DoCommand(AConnection);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CEPollClient ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CEPollClient::CEPollClient(): CPollSocketClient(), CEPoll() {

        }
        //--------------------------------------------------------------------------------------------------------------

        bool CEPollClient::DoExecute(CTCPConnection *AConnection) {
            if (m_OnExecute != nullptr) {
                return m_OnExecute(AConnection);
            }
            return DoCommand(AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollClient::DoTimeOut(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                pConnection->Disconnect();
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollClient::DoRead(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                DoExecute(pConnection);
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CEPollClient::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                pConnection->CheckForDisconnect(true);
                pConnection->FreeWriteBuffer();
                pConnection->FlushOutputBuffer();
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CAsyncServer ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CAsyncServer::CAsyncServer(): CEPollServer() {
            m_ActiveLevel = alShutDown;
            m_pIOHandler = nullptr;
            m_FreeIOHandler = true;
            m_pCommandHandlers = new CCommandHandlers(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        CAsyncServer::~CAsyncServer() {
            FreeAndNil(m_pCommandHandlers);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CAsyncServer::FreeIOHandler() {
            if (m_FreeIOHandler) {
                FreeAndNil(m_pIOHandler);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CAsyncServer::SetIOHandler(CServerIOHandler *Value) {
            if (m_pIOHandler != Value) {
                FreeIOHandler();
                m_pIOHandler = Value;
                m_FreeIOHandler = false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CAsyncServer::InitializeCommandHandlers() {

        }
        //--------------------------------------------------------------------------------------------------------------

        bool CAsyncServer::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *pHandler;

            bool Result = m_pCommandHandlers->Count() > 0;

            if (Result) {
                if (AConnection->Connected()) {

                    CMemoryStream LStream;

                    LStream.SetSize(MaxLineLengthDefault);
                    LStream.SetSize(AConnection->ReadLn((char *) LStream.Memory()));

                    if (LStream.Size() > 0 && LStream.Size() < MaxLineLengthDefault) {
                        DoBeforeCommandHandler(AConnection, (char *) LStream.Memory());
                        try {
                            int Index;
                            for (Index = 0; Index < m_pCommandHandlers->Count(); ++Index) {
                                pHandler = m_pCommandHandlers->Commands(Index);
                                if (pHandler->Enabled()) {
                                    if (pHandler->Check((char *) LStream.Memory(), LStream.Size(), AConnection))
                                        break;
                                }
                            }
                            if (Index == m_pCommandHandlers->Count())
                                DoNoCommandHandler((char *) LStream.Memory(), AConnection);
                        } catch (Delphi::Exception::Exception &E) {
                            DoException(AConnection, E);
                        }
                        DoAfterCommandHandler(AConnection);
                    }
                }
            }

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CAsyncClient ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CAsyncClient::CAsyncClient(): CEPollClient() {
            m_Active = false;
            m_AutoConnect = true;
#ifdef WITH_SSL
            m_UsedSSL = false;
#endif
            m_pCommandHandlers = new CCommandHandlers(this);
        }

        CAsyncClient::CAsyncClient(const CString &Host, unsigned short Port): CAsyncClient() {
            m_Host = Host;
            m_Port = Port;
#ifdef WITH_SSL
            m_UsedSSL = Port == 443;
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        CAsyncClient::~CAsyncClient() {
            SetActive(false);
            FreeAndNil(m_pCommandHandlers);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CAsyncClient::SetActive(bool AValue) {
            if (m_Active != AValue ) {

                if (AValue) {
                    Initialize();

                    if (CommandHandlers()->Count() == 0)
                        InitializeCommandHandlers();

                    if (m_AutoConnect)
                        ConnectStart();

                } else {
                    m_pEventHandlers->Clear();
                    CloseAllConnection();
                }

                m_Active = AValue;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CAsyncClient::ConnectStart() {
            auto pIOHandler = new CIOHandlerSocket();
#ifdef WITH_SSL
            pIOHandler->Open(m_UsedSSL ? sslClient : sslNotUsed);
#else
            pIOHandler->Open();
#endif
            pIOHandler->Binding()->AllocateSocket(SOCK_STREAM, IPPROTO_IP, O_NONBLOCK);
            pIOHandler->Binding()->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (void *) &SO_True, sizeof(SO_True));

            auto pEventHandler = m_pEventHandlers->Add(pIOHandler->Binding()->Handle());

            if (ExternalPollStack()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                pEventHandler->OnTimeOutEvent([this](auto && AHandler) { DoTimeOut(AHandler); });
                pEventHandler->OnConnectEvent([this](auto && AHandler) { DoConnect(AHandler); });
                pEventHandler->OnReadEvent([this](auto && AHandler) { DoRead(AHandler); });
                pEventHandler->OnWriteEvent([this](auto && AHandler) { DoWrite(AHandler); });
#else
                pEventHandler->OnTimeOutEvent(std::bind(&CAsyncClient::DoTimeOut, this, _1));
                pEventHandler->OnConnectEvent(std::bind(&CAsyncClient::DoConnect, this, _1));
                pEventHandler->OnReadEvent(std::bind(&CAsyncClient::DoRead, this, _1));
                pEventHandler->OnWriteEvent(std::bind(&CAsyncClient::DoWrite, this, _1));
#endif
            }

            pEventHandler->Start(etConnect);

            int ErrorCode = pIOHandler->Binding()->Connect(AF_INET, m_Host.c_str(), m_Port);

            DoConnectStart(pIOHandler, pEventHandler);

            if (ErrorCode == 0)
                DoConnect(pEventHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CAsyncClient::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *pHandler;

            bool Result = m_pCommandHandlers->Count() > 0;

            if (Result) {
                if (AConnection->Connected()) {

                    CMemoryStream LStream;

                    LStream.SetSize(MaxLineLengthDefault);
                    LStream.SetSize(AConnection->ReadLn((char *) LStream.Memory()));

                    if (LStream.Size() > 0 && LStream.Size() < MaxLineLengthDefault) {
                        DoBeforeCommandHandler(AConnection, (char *) LStream.Memory());
                        try {
                            int Index;
                            for (Index = 0; Index < m_pCommandHandlers->Count(); ++Index) {
                                pHandler = m_pCommandHandlers->Commands(Index);
                                if (pHandler->Enabled()) {
                                    if (pHandler->Check((char *) LStream.Memory(), LStream.Size(), AConnection))
                                        break;
                                }
                            }
                            if (Index == m_pCommandHandlers->Count())
                                DoNoCommandHandler((char *) LStream.Memory(), AConnection);
                        } catch (Delphi::Exception::Exception &E) {
                            DoException(AConnection, E);
                        }
                        DoAfterCommandHandler(AConnection);
                    }
                }
            }

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CUDPAsyncServer -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CUDPAsyncServer::CUDPAsyncServer(): CAsyncServer() {
            m_OnRead = nullptr;
            m_OnWrite = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CUDPAsyncServer::CUDPAsyncServer(unsigned short AListen): CUDPAsyncServer() {
            DefaultPort(AListen);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::InitializeBindings() {
            CSocketHandle* pBinding = Bindings()->Add();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::SetActiveLevel(CActiveLevel AValue) {

            CPollEventHandler *pEventHandler = nullptr;

            if (m_ActiveLevel != AValue ) {

                if (m_ActiveLevel < AValue) {

                    if (CommandHandlers()->Count() == 0)
                        InitializeCommandHandlers();

                    if (Bindings()->Count() == 0)
                        InitializeBindings();

                    for (int i = 0; i < Bindings()->Count(); ++i) {
                        auto SocketHandle = Bindings()->Handles(i);
                        if (AValue >= alBinding && !SocketHandle->HandleAllocated()) {
                            SocketHandle->AllocateSocket(SOCK_DGRAM, IPPROTO_UDP, O_NONBLOCK);
                            SocketHandle->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (void *) &SO_True, sizeof(SO_True));
                            SocketHandle->SetSockOpt(SOL_SOCKET, SO_BROADCAST, (void *) &SO_True, sizeof(SO_True));

                            SocketHandle->Bind();
                        }

                        if (AValue == alActive) {
                            pEventHandler = m_pEventHandlers->Add(SocketHandle->Handle());
                            pEventHandler->Start(etIO);
                        }
                    }

                } else {

                    if (AValue <= alBinding) {
                        m_pEventHandlers->Clear();
                        CloseAllConnection();
                    }

                    if (AValue == alShutDown) {
                        for (int i = 0; i < Bindings()->Count(); ++i) {
                            Bindings()->Handles(i)->CloseSocket();
                        }
                    }
                }

                m_ActiveLevel = AValue;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::DoRead(CPollEventHandler *AHandler) {
            auto SocketHandle = Bindings()->BindingByHandle(AHandler->Socket());
            if (SocketHandle != nullptr) {
                Receive(SocketHandle);
                DoBufferRead(SocketHandle);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::DoWrite(CPollEventHandler *AHandler) {
            auto SocketHandle = Bindings()->BindingByHandle(AHandler->Socket());
            if (SocketHandle != nullptr) {
                DoBufferWrite(SocketHandle);
                Send(SocketHandle);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::DoBufferRead(CSocketHandle *ASocketHandle) {
            if (m_OnRead != nullptr) {
                m_OnRead(this, ASocketHandle, m_InputBuffer);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CUDPAsyncServer::DoBufferWrite(CSocketHandle *ASocketHandle) {
            if (m_OnWrite != nullptr) {
                m_OnWrite(this, ASocketHandle, m_OutputBuffer);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CUDPAsyncServer::Receive(CSocketHandle *ASocketHandle) {
            ssize_t LByteCount;
            ssize_t LByteRecv = 0;

            if (ASocketHandle != nullptr && ASocketHandle->HandleAllocated()) {
                CMemoryStream LStream;
                do {
                    LStream.SetSize(GRecvBufferSizeDefault);
                    LByteCount = ASocketHandle->RecvFrom(LStream.Memory(), LStream.Size());

                    int Ignore[] = {EAGAIN, EWOULDBLOCK};
                    if (GStack->CheckForSocketError(LByteCount, Ignore, chARRAY(Ignore), egSystem))
                        break;

                    LStream.SetSize((size_t) LByteCount);
                    m_InputBuffer.Seek(0, soEnd);
                    m_InputBuffer.WriteBuffer(LStream.Memory(), LStream.Size());

                    LByteRecv += LByteCount;
                } while (LByteCount > 0);

                return LByteRecv;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        ssize_t CUDPAsyncServer::Send(CSocketHandle *ASocketHandle) {
            ssize_t LByteCount;
            ssize_t LByteSend = 0;

            if (ASocketHandle != nullptr && ASocketHandle->HandleAllocated()) {
                while (m_OutputBuffer.Size() != 0) {
                    LByteCount = ASocketHandle->SendTo(ASocketHandle->PeerIP(), ASocketHandle->PeerPort(),
                                                       m_OutputBuffer.Memory(), m_OutputBuffer.Size());

                    int Ignore[] = {EAGAIN, EWOULDBLOCK};
                    if (GStack->CheckForSocketError(LByteCount, Ignore, chARRAY(Ignore), egSystem))
                        break;

                    m_OutputBuffer.Remove((size_t) LByteCount);
                    LByteSend += LByteCount;
                }

                return LByteSend;
            }

            return -1;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPAsyncServer -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPAsyncServer::CTCPAsyncServer(): CAsyncServer() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPAsyncServer::CTCPAsyncServer(unsigned short AListen): CTCPAsyncServer() {
            DefaultPort(AListen);
        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPServerConnection *CTCPAsyncServer::GetConnection(int AIndex) const {
            return (CTCPServerConnection *) CCollection::GetItem(AIndex);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::SetConnection(int AIndex, CTCPServerConnection *AValue) {
            CCollection::SetItem(AIndex, AValue);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::SetActiveLevel(CActiveLevel AValue) {

            CPollEventHandler *pEventHandler = nullptr;

            if (m_ActiveLevel != AValue ) {

                if (m_ActiveLevel < AValue) {

                    if (CommandHandlers()->Count() == 0)
                        InitializeCommandHandlers();

                    if (m_pIOHandler == nullptr)
                        m_pIOHandler = new CServerIOHandler();

                    if (Bindings()->Count() == 0)
                        InitializeBindings();

                    for (int i = 0; i < Bindings()->Count(); ++i) {
                        auto SocketHandle = Bindings()->Handles(i);
                        if (AValue >= alBinding && !SocketHandle->HandleAllocated()) {
                            SocketHandle->AllocateSocket(SOCK_STREAM, IPPROTO_IP, O_NONBLOCK);
                            SocketHandle->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (void *) &SO_True, sizeof(SO_True));

                            SocketHandle->Bind();
                            SocketHandle->Listen(SOMAXCONN);
                        }

                        if (AValue == alActive) {
                            pEventHandler = m_pEventHandlers->Add(SocketHandle->Handle());
                            pEventHandler->Start(etAccept);
                        }
                    }

                } else {

                    if (AValue <= alBinding) {
                        m_pEventHandlers->Clear();
                        CloseAllConnection();
                    }

                    if (AValue == alShutDown) {
                        for (int i = 0; i < Bindings()->Count(); ++i) {
                            Bindings()->Handles(i)->CloseSocket();
                        }
                        FreeIOHandler();
                    }
                }

                m_ActiveLevel = AValue;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::DoTimeOut(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                pConnection->Disconnect();
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::DoRead(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                DoExecute(pConnection);
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPConnection *> (AHandler->Binding());
            try {
                pConnection->CheckForDisconnect(true);
                pConnection->FreeWriteBuffer();
                pConnection->FlushOutputBuffer();
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncServer::DoAccept(CPollEventHandler *AHandler) {
            CIOHandlerSocket *pIOHandler = nullptr;
            CPollEventHandler *pEventHandler = nullptr;
            CTCPServerConnection *pConnection = nullptr;

            try {
                pIOHandler = (CIOHandlerSocket *) IOHandler()->Accept(AHandler->Socket(), SOCK_NONBLOCK);

                if (Assigned(pIOHandler)) {
                    pConnection = new CTCPServerConnection(this);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    pConnection->OnDisconnected(std::bind(&CTCPAsyncServer::DoDisconnected, this, _1));
#endif
                    pConnection->IOHandler(pIOHandler);

                    pIOHandler->AfterAccept();

                    pEventHandler = m_pEventHandlers->Add(pIOHandler->Binding()->Handle());
                    pEventHandler->Binding(pConnection);
                    pEventHandler->Start(etIO);

                    DoConnected(pConnection);
                } else {
                    throw ETCPServerError(_T("TCP Server Error..."));
                }
            } catch (Delphi::Exception::Exception &E) {
                delete pConnection;
                DoListenException(E);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CTCPAsyncClient -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CTCPAsyncClient::CTCPAsyncClient(): CAsyncClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CTCPAsyncClient::CTCPAsyncClient(LPCTSTR AHost, unsigned short APort): CTCPAsyncClient() {
            m_Host = AHost;
            m_Port = APort;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncClient::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto pConnection = new CTCPClientConnection(this);
            pConnection->IOHandler(AIOHandler);
            pConnection->AutoFree(true);
            AHandler->Binding(pConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CTCPAsyncClient::DoConnect(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CTCPClientConnection *> (AHandler->Binding());

            if (pConnection == nullptr) {
                AHandler->Stop();
                return;
            }

            try {
                auto pIOHandler = (CIOHandlerSocket *) pConnection->IOHandler();

                if (pIOHandler->Binding()->CheckConnection()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    pConnection->OnDisconnected(std::bind(&CTCPAsyncClient::DoDisconnected, this, _1));
#endif
                    AHandler->Start(etIO);
                    DoConnected(pConnection);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

    } // namespace Socket

} // namespace Delphi
