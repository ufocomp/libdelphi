/*++

Library name:

  libdelphi

Module Name:

  HTTP.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_HTTP_HPP
#define DELPHI_HTTP_HPP
//----------------------------------------------------------------------------------------------------------------------

#define DefaultServerName  DELPHI_LIB_VER
#define DefaultAllowedMethods  _T("HEAD, OPTIONS, GET")
//----------------------------------------------------------------------------------------------------------------------

#ifndef WWWServerName
#define WWWServerName DefaultServerName
#endif

#ifndef WWWAllowedMethods
#define WWWAllowedMethods DefaultAllowedMethods
#endif
//----------------------------------------------------------------------------------------------------------------------

#define HTTP_PREFIX "http"
#define HTTP_PREFIX_SIZE 4

#define HTTPS_PREFIX "https"
#define HTTPS_PREFIX_SIZE 5

#define WS_PREFIX "ws"
#define WS_PREFIX_SIZE 2

#define WSS_PREFIX "wss"
#define WSS_PREFIX_SIZE 3

#define HTTP_PORT 80
#define HTTP_SSL_PORT 443
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace HTTP {

        namespace Mapping {
            LPCTSTR ExtToType(LPCTSTR Ext);
            bool IsText(LPCTSTR MimeType);
        }
        //--------------------------------------------------------------------------------------------------------------

        struct CLocation {
        private:

            enum {
                url_hostname = 0,
                url_port,
                url_pathname,
                url_search,
                url_hash
            } flag;

            CString portStr;

            void ProtocolInit() {
                struct servent *sptr;
                if ((sptr = getservbyport(port, "tcp")) != nullptr) {
                    protocol = sptr->s_name;
                } else {
                    protocol = HTTP_PREFIX;
                }
            }

        public:

            CString protocol;
            CString hostname;
            int port;
            CString pathname;
            CString search;
            CString hash;

            CLocation(): flag(url_hostname), port(80) {

            }

            CLocation(const CLocation& URL): CLocation() {
                Assign(URL);
            }

            explicit CLocation(const CString& URL): CLocation() {
                Parse(URL);
            }

            void Assign(const CLocation& URL) {
                protocol = URL.protocol;
                hostname = URL.hostname;
                portStr = URL.portStr;
                port = URL.port;
                pathname = URL.pathname;
                search = URL.search;
                hash = URL.hash;
            };

            void Clear() {
                flag = url_hostname;
                protocol.Clear();
                hostname.Clear();
                portStr.Clear();
                pathname.Clear();
                search.Clear();
                hash.Clear();
                port = 80;
            };

            void Parse(const CString &URL) {
                Clear();

                size_t pos = 0;
                size_t end = URL.Find("://", pos);

                size_t findSearch = URL.Find( '?', pos);
                size_t findHash = URL.Find( '#', pos);

                if (end > findSearch || end > findHash)
                    end = CString::npos;

                if (end != CString::npos) {
                    protocol = URL.SubString(pos, end);
                    pos = end + 3;

                    if (protocol == HTTPS_PREFIX || protocol == WSS_PREFIX) {
                        port = HTTP_SSL_PORT;
                    }
                }

                TCHAR ch = URL.at(pos++);
                while (!IsCtl(ch)) {
                    if (ch == ':' && flag < url_port)
                        flag = url_port;
                    if (ch == '/' && flag < url_pathname)
                        flag = url_pathname;
                    if (ch == '?' && flag < url_search)
                        flag = url_search;
                    if (ch == '#' && flag < url_hash)
                        flag = url_hash;

                    switch (flag) {
                        case url_hostname:
                            hostname.Append(ch);
                            break;
                        case url_port:
                            if (ch != ':')
                                portStr.Append(ch);
                            break;
                        case url_pathname:
                            pathname.Append(ch);
                            break;
                        case url_search:
                            search.Append(ch);
                            break;
                        case url_hash:
                            hash.Append(ch);
                            break;
                    }

                    ch = URL.at(pos++);
                }

                if (!portStr.IsEmpty())
                    port = StrToIntDef(portStr.c_str(), 0);

                if (protocol.IsEmpty())
                    ProtocolInit();
            };

            CString Host() const {
                CString Result;

                Result = hostname;

                if (!portStr.IsEmpty()) {
                    Result << ":";
                    Result << portStr;
                } else {
                    if (port != HTTP_PORT && port != HTTP_SSL_PORT) {
                        Result << ":";
                        Result << port;
                    }
                }

                return Result;
            };

            CString Origin() const {
                CString Result;

                Result = protocol;
                Result << "://";
                Result << Host();

                return Result;
            };

            CString toString() const {
                CString Result;

                Result = Origin();
                Result << pathname;
                Result << search;
                Result << hash;

                return Result;
            };

            CString href() const {
                return toString();
            };

            CLocation& operator= (const CLocation &URL) {
                if (this != &URL) {
                    Assign(URL);
                }
                return *this;
            };

            CLocation& operator= (const CString &URL) {
                Parse(URL);
                return *this;
            };

            CLocation& operator<< (const CString &Value) {
                Parse(href() + Value);
                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CAuthorization --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CAuthorizationError : public ExceptionFrm {
            typedef ExceptionFrm inherited;

        public:

            CAuthorizationError() : inherited() {};

            explicit CAuthorizationError(LPCTSTR AFormat, ...) : inherited() {
                CString Format("Authorization error: ");
                Format << AFormat;
                va_list argList;
                va_start(argList, AFormat);
                FormatMessage(Format.c_str(), argList);
                va_end(argList);
            };

            ~CAuthorizationError() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        struct CCleanToken {

            const CString Header;
            const CString Payload;

            const bool Valid;

            explicit CCleanToken(const CString& Header, const CString& Payload, bool Valid):
                    Header(Header), Payload(Payload), Valid(Valid) {

            }

            template<typename T>
            CString Sign(const T& algorithm) const {
                if (!Valid)
                    throw CAuthorizationError("Clean Token has not valid.");
                const auto& header = base64urlEncoding(Header);
                const auto& payload = base64urlEncoding(Payload);
                CString token = header + "." + payload;
                return token + "." + base64urlEncoding(algorithm.sign(token));
            }

        };
        //--------------------------------------------------------------------------------------------------------------

        struct CAuthorization {

            enum CScheme {
                asUnknown = -1,
                asBasic,
                asBearer
            } Schema;

            CString Username;
            CString Password;

            enum CAuthorizationType {
                atUnknown = -1,
                atOwner,
                atSession,
                atClient
            } Type;

            CString Token;

            CAuthorization(): Schema(asUnknown), Type(atUnknown) {

            }

            CAuthorization(const CAuthorization &Value): CAuthorization() {
                if (this != &Value) {
                    Assign(Value);
                }
            }

            explicit CAuthorization(const CString& String): CAuthorization() {
                Parse(String);
            }

            void Assign(const CAuthorization &Value) {
                Schema = Value.Schema;
                Username = Value.Username;
                Password = Value.Password;
                Type = Value.Type;
                Token = Value.Token;
            }

            void Clear() {
                Schema = asUnknown;
                Username.Clear();
                Password.Clear();
                Type = atUnknown;
                Token.Clear();
            }

            void Parse(const CString& String) {
                if (String.IsEmpty())
                    throw CAuthorizationError("Data has not be empty.");

                if (String.SubString(0, 5).Lower() == "basic") {

                    Schema = asBasic;

                    const CString Passphrase(base64_decode(String.SubString(6)));

                    const size_t pos = Passphrase.Find(':');
                    if (pos == CString::npos)
                        throw CAuthorizationError("Incorrect passphrase.");

                    Type = atOwner;

                    Username = Passphrase.SubString(0, pos);
                    Password = Passphrase.SubString(pos + 1);

                } else if (String.SubString(0, 6).Lower() == "bearer") {

                    Schema = asBearer;
                    Token = String.SubString(7);

                    if (Token.IsEmpty())
                        throw CAuthorizationError("Token has not be empty.");

                } else {
                    throw CAuthorizationError("Unknown schema.");
                }
            }

            CAuthorization &operator = (const CAuthorization& Authorization) {
                Assign(Authorization);
                return *this;
            }

            CAuthorization &operator << (const CString& String) {
                Parse(String);
                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHeaders --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPair<CString> CHeader;
        typedef TPairs<CString> CHeaders;

        //--------------------------------------------------------------------------------------------------------------

        //-- CFormData -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        struct CFormDataItem {

            CString Name;
            CString File;
            CString Data;

            CFormDataItem& operator= (const CFormDataItem& Value) {
                if (this != &Value) {
                    Name = Value.Name;
                    File = Value.File;
                    Data = Value.Data;
                }
                return *this;
            };

            inline bool operator!= (const CFormDataItem& Value) { return Name != Value.Name; };
            inline bool operator== (const CFormDataItem& Value) { return Name == Value.Name; };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CFormData -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CFormData {
        private:

            TList<CFormDataItem> m_pList;

            CString m_NullData;

            void Put(int Index, const CFormDataItem& Item);

            CFormDataItem& Get(int Index);
            const CFormDataItem& Get(int Index) const;

        protected:

            int GetCount() const;

            const CString &GetData(const CString &Name) const;
            const CString &GetData(LPCTSTR Name) const;

        public:

            CFormData() = default;

            ~CFormData();

            void Clear();

            int IndexOfName(const CString &Name) const;
            int IndexOfName(LPCTSTR Name) const;

            void Insert(int Index, const CFormDataItem& Item);

            int Add(const CFormDataItem& Item);

            void Delete(int Index);

            void SetCount(int NewCount);

            CFormDataItem& First();

            CFormDataItem& Last();

            int Count() const { return GetCount(); }

            void Assign(const CFormData& Value);

            const CString &Data(const CString& Name) const { return GetData(Name); };
            const CString &Data(LPCTSTR Name) const { return GetData(Name); };

            CFormDataItem& Items(int Index) { return Get(Index); }
            const CFormDataItem& Items(int Index) const { return Get(Index); }

            void Items(int Index, const CFormDataItem& Value) { Put(Index, Value); }

            CFormData& operator= (const CFormData& Value) {
                if (this != &Value)
                    Assign(Value);
                return *this;
            };

            CFormDataItem& operator[](int Index) { return Get(Index); }
            const CFormDataItem& operator[](int Index) const { return Get(Index); }

            CFormDataItem& operator[](LPCTSTR Name) { return Items(IndexOfName(Name)); }
            const CFormDataItem& operator[](LPCTSTR Name) const { return Items(IndexOfName(Name)); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPRequest ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPRequest {
        public:

            CString Method;

            CString URI;

            int VMajor;
            int VMinor;

            /// The headers to be included in the request.
            CHeaders Headers;

            /// The uri parameters to be included in the request.
            CStringList Params;

            /// The Cookies to be included in the request.
            CStringList Cookies;

            /// The content length to be sent in the request.
            size_t ContentLength;

            /// The content type of the reply.
            enum CContentType
            {
                html = 0,
                json,
                xml,
                text,
                sbin
            } ContentType = html;

            /// The content to be sent in the request.
            CString Content;

            /// The form data to be included in the request.
            CStringList FormData;

            /// The value of the "User-Agent" header.
            CString UserAgent;

            bool CloseConnection = true;

            /// The Location interface represents a location (URL) as an object.
            CLocation Location;

            /// Clear content and headers.
            void Clear();

            void ToText();
            void ToJSON();

            /// Convert the request into a vector of buffers. The buffers do not own the
            /// underlying memory blocks, therefore the request object must remain valid and
            /// not be changed until the write operation has completed.
            void ToBuffers(CMemoryStream &Stream);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, const CString& Value);

            /// Add header to headers.
            void AddHeader(const CString& Name, const CString& Value);

            bool BuildLocation();
            void BuildCookies();

            void Assign(const CHTTPRequest &Value) {
                if (this != &Value) {
                    Method = Value.Method;
                    URI = Value.URI;
                    VMajor = Value.VMajor;
                    VMinor = Value.VMinor;
                    Headers = Value.Headers;
                    Params = Value.Params;
                    Cookies = Value.Cookies;
                    ContentLength = Value.ContentLength;
                    ContentType = Value.ContentType;
                    Content = Value.Content;
                    UserAgent = Value.UserAgent;
                    CloseConnection = Value.CloseConnection;
                    Location = Value.Location;
                }
            };

            CHTTPRequest(): VMajor(1), VMinor(1), ContentLength(0) {
                Params.LineBreak("&");
                Params.Delimiter('&');
            }

            CHTTPRequest(const CHTTPRequest &Request): CHTTPRequest() {
                Assign(Request);
            }

            CHTTPRequest &operator=(const CHTTPRequest &Request) {
                Assign(Request);
                return *this;
            };

            /// Get a prepare request.
            static CHTTPRequest *Prepare(CHTTPRequest *ARequest, LPCTSTR AMethod, LPCTSTR AURI,
                LPCTSTR AContentType = nullptr, LPCTSTR AConnection = nullptr);

            /// Add Authorization header to headers
            static CHTTPRequest *Authorization(CHTTPRequest *ARequest, LPCTSTR AMethod, LPCTSTR ALogin, LPCTSTR APassword);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPRequestParser ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace Request {

            enum CParserState {
                method_start,
                method,
                uri_start,
                uri,
                uri_param_start,
                uri_param,
                uri_param_mime,
                http_version_h,
                http_version_t_1,
                http_version_t_2,
                http_version_p,
                http_version_slash,
                http_version_major_start,
                http_version_major,
                http_version_minor_start,
                http_version_minor,
                expecting_newline_1,
                header_line_start,
                header_lws,
                header_name,
                space_before_header_value,
                header_value,
                header_value_options_start,
                header_value_options,
                expecting_newline_2,
                expecting_newline_3,
                content,
                form_data_start,
                form_data,
                form_mime
            };

        }
        //--------------------------------------------------------------------------------------------------------------

        struct CHTTPContext {
            LPCBYTE Begin {};
            LPCBYTE End {};
            size_t Size {};
            int Result;
            Request::CParserState State;
            size_t ContentLength {};
            TCHAR MIME[3] = {};
            size_t MimeIndex {};

            CHTTPContext(LPCBYTE ABegin, size_t ASize, Request::CParserState AState = Request::method_start,
                         size_t AContentLength = 0) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = AContentLength;
                MimeIndex = 0;
            };

        };
        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming HTTP requests.
        class CHTTPRequestParser {
        public:

            /// Check if a byte is an HTTP character.
            static bool IsChar(int c);

            /// Check if a byte is an HTTP control character.
            static bool IsCtl(int c);

            /// Check if a byte is defined as an HTTP tspecial character.
            static bool IsTSpecial(int c);

            /// Check if a byte is a digit.
            static bool IsDigit(int c);

            /// Handle the next character of input.
            static int Consume(CHTTPRequest *ARequest, CHTTPContext &Context);

            /// Parse some data. The int return value is "1" when a complete request
            /// has been parsed, "0" if the data is invalid, "-1" when more
            /// data is required.
            static int Parse(CHTTPRequest *ARequest, CHTTPContext &Context);

            static int ParseFormData(CHTTPRequest *ARequest, CFormData &FormData);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocketParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming WebSocket requests.
        class CWebSocketParser {
        public:

            static int Parse(CWebSocket *ARequest, const CMemoryStream &AStream);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPReply ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServerConnection;
        //--------------------------------------------------------------------------------------------------------------

        struct CHTTPReply
        {
            int VMajor {};
            int VMinor {};

            /// The status of the reply.
            enum CStatusType
            {
                switching_protocols = 101,
                ok = 200,
                created = 201,
                accepted = 202,
                non_authoritative = 203,
                no_content = 204,
                multiple_choices = 300,
                moved_permanently = 301,
                moved_temporarily = 302,
                see_other = 303,
                not_modified = 304,
                bad_request = 400,
                unauthorized = 401,
                forbidden = 403,
                not_found = 404,
                not_allowed = 405,
                internal_server_error = 500,
                not_implemented = 501,
                bad_gateway = 502,
                service_unavailable = 503,
                gateway_timeout = 504
            } Status = internal_server_error;

            CString StatusString{};
            CString StatusText{};

            /// The content type of the reply.
            enum CContentType
            {
                html = 0,
                json,
                xml,
                text,
                sbin
            } ContentType = html;

            CString ServerName{};

            CString AllowedMethods{};

            bool CloseConnection = true;

            /// The headers to be included in the reply.
            CHeaders Headers{};

            /// The content length to be receive in the reply.
            size_t ContentLength = 0;

            /// The content to be receive in the reply.
            CString Content{};

            /// The cache file.
            CString CacheFile{};

            /// Clear content and headers.
            void Clear();

            void ToText();
            void ToJSON();

            void StringToStatus();

            /// Convert the reply into a vector of buffers. The buffers do not own the
            /// underlying memory blocks, therefore the reply object must remain valid and
            /// not be changed until the write operation has completed.
            void ToBuffers(CMemoryStream &Stream);

            static LPCTSTR GetGMT(LPTSTR lpszBuffer, size_t Size, time_t Delta = 0);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, const CString& Value);

            /// Add header to headers.
            void AddHeader(const CString& Name, const CString& Value);

            /// Set cookie.
            void SetCookie(LPCTSTR lpszName, LPCTSTR lpszValue, LPCTSTR lpszPath = nullptr, time_t Expires = 0,
                    bool HttpOnly = true, LPCTSTR lpszSameSite = _T("Lax"));

            /// Get a prepare reply.
            static CHTTPReply *GetReply(CHTTPReply *AReply, CStatusType AStatus, LPCTSTR AContentType = nullptr,
                LPCTSTR ATransferEncoding = nullptr);

            /// Get a stock reply.
            static CHTTPReply *GetStockReply(CHTTPReply *AReply, CStatusType AStatus);

            static void AddUnauthorized(CHTTPReply *AReply, bool ABearer = false, LPCTSTR AError = nullptr, LPCTSTR AMessage = nullptr);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPReplyParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace Reply {

            enum CParserState {
                http_version_h,
                http_version_t_1,
                http_version_t_2,
                http_version_p,
                http_version_slash,
                http_version_major_start,
                http_version_major,
                http_version_minor_start,
                http_version_minor,
                http_status_start,
                http_status,
                http_status_text_start,
                http_status_text,
                expecting_newline_1,
                header_line_start,
                header_lws,
                header_name,
                space_before_header_value,
                header_value,
                header_value_options_start,
                header_value_options,
                expecting_newline_2,
                expecting_newline_3,
                content,
                content_checking_length,
                content_checking_newline,
                content_checking_data,
            };
            //----------------------------------------------------------------------------------------------------------
        }

        struct CHTTPReplyContext {
            LPCBYTE Begin;
            LPCBYTE End;
            size_t Size;
            int Result;
            Reply::CParserState State;
            size_t ContentLength;
            size_t ChunkedLength;
            CString Chunked;
            TCHAR MIME[3] = {};
            size_t MimeIndex;

            CHTTPReplyContext(LPCBYTE ABegin, size_t ASize, Reply::CParserState AState = Reply::http_version_h,
                          size_t AContentLength = 0, size_t AChunkedLength = 0) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = AContentLength;
                ChunkedLength = AChunkedLength;
                MimeIndex = 0;
            };

        };
        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming requests.
        class CHTTPReplyParser {
        public:
            /// Check if a byte is an HTTP character.
            static bool IsChar(int c);

            /// Check if a byte is an HTTP control character.
            static bool IsCtl(int c);

            /// Check if a byte is defined as an HTTP tspecial character.
            static bool IsTSpecial(int c);

            /// Check if a byte is a digit.
            static bool IsDigit(int c);

            /// Check if a byte is a hex.
            static bool IsHex(int c);

            /// Handle the next character of input.
            static int Consume(CHTTPReply *AReply, CHTTPReplyContext& Context);

            /// Parse some data. The int return value is "1" when a complete request
            /// has been parsed, "0" if the data is invalid, "-1" when more
            /// data is required.
            static int Parse(CHTTPReply *AReply, CHTTPReplyContext& Context);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServerConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServer;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServerConnection: public CTCPServerConnection {
            typedef CTCPServerConnection inherited;

        private:

            CHTTPRequest *m_Request;
            CHTTPReply *m_Reply;

            /// The current state of the parser.
            Request::CParserState m_State;

            size_t m_ContentLength;

            void Parse(CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) override;

        protected:

            CHTTPRequest *GetRequest();
            CHTTPReply *GetReply();

        public:

            explicit CHTTPServerConnection(CPollSocketServer *AServer);

            ~CHTTPServerConnection() override;

            void Clear() override;

            bool ParseInput(COnSocketExecuteEvent && OnExecute);

            CHTTPRequest *Request() { return GetRequest(); }
            CHTTPReply *Reply() { return GetReply(); }

            void SendStockReply(CHTTPReply::CStatusType AStatus, bool ASendNow = false);
            void SendReply(CHTTPReply::CStatusType AStatus, LPCTSTR AContentType = nullptr, bool ASendNow = false);
            void SendReply(bool ASendNow = false);

            void SwitchingProtocols(const CString &Accept, const CString &Protocol);

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClientConnection: public CTCPClientConnection {
            typedef CTCPClientConnection inherited;

        private:

            CHTTPRequest *m_Request;
            CHTTPReply *m_Reply;

            /// The current state of the parser.
            Reply::CParserState m_State;

            size_t m_ContentLength;
            size_t m_ChunkedLength;

            void Parse(CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) override;

        protected:

            CHTTPRequest *GetRequest();
            CHTTPReply *GetReply();

        public:

            explicit CHTTPClientConnection(CPollSocketClient *AClient);

            ~CHTTPClientConnection() override;

            void Clear() override;

            bool ParseInput(COnSocketExecuteEvent && OnExecute);

            CHTTPRequest *Request() { return GetRequest(); }
            CHTTPReply *Reply() { return GetReply(); };

            void SwitchingProtocols(CHTTPProtocol Protocol);

            void SendRequest(bool ASendNow = false);

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServer -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPairs<CJSON> CSites;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServer: public CTCPAsyncServer {
        protected:

            CProviders m_Providers;

            CSites m_Sites;

            void DoTimeOut(CPollEventHandler *AHandler) override;

            void DoAccept(CPollEventHandler *AHandler) override;

            void DoRead(CPollEventHandler *AHandler) override;

            void DoWrite(CPollEventHandler *AHandler) override;

            bool DoCommand(CTCPConnection *AConnection) override;

            bool DoExecute(CTCPConnection *AConnection) override;

            void DoReply(CObject *Sender);

        public:

            CHTTPServer();

            explicit CHTTPServer(const CString &IP, unsigned short Port);

            ~CHTTPServer() override = default;

            void InitializeBindings() override;

            void Assign(const CHTTPServer &Server);

            static CString URLEncode(const CString& In);
            static CString URLDecode(const CString& In);
            static bool URLDecode(const CString& In, CString& Out);

            CProviders& Providers() { return m_Providers; };
            const CProviders& Providers() const { return m_Providers; };

            CSites& Sites() { return m_Sites; };
            const CSites& Sites() const { return m_Sites; };

            CHTTPServer &operator = (const CHTTPServer &Server) {
                Assign(Server);
                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClient;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CHTTPClient *Sender, CHTTPRequest *Request)> COnHTTPClientRequestEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClient: public CAsyncClient {
        private:

            COnHTTPClientRequestEvent m_OnRequest;

        protected:

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;

            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRead(CPollEventHandler *AHandler) override;

            void DoWrite(CPollEventHandler *AHandler) override;

            bool DoCommand(CTCPConnection *AConnection) override;

            bool DoExecute(CTCPConnection *AConnection) override;

            virtual void DoRequest(CHTTPClientConnection *AConnection);

        public:

            CHTTPClient();

            explicit CHTTPClient(const CString &Host, unsigned short Port);

            ~CHTTPClient() override = default;

            const COnHTTPClientRequestEvent &OnRequest() const { return m_OnRequest; }
            void OnRequest(COnHTTPClientRequestEvent && Value) { m_OnRequest = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientItem -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClientManager;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClientItem: public CCollectionItem, public CHTTPClient {
        public:

            explicit CHTTPClientItem(CHTTPClientManager *AManager);

            explicit CHTTPClientItem(CHTTPClientManager *AManager, const CString &Host, unsigned short Port);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientManager ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClientManager: public CCollection {
            typedef CCollection inherited;

        protected:

            CHTTPClientItem *GetItem(int Index) const override;

        public:

            CHTTPClientManager(): CCollection(this) {

            };

            ~CHTTPClientManager() override = default;

            CHTTPClientItem *Add(const CString &Host, unsigned short Port);

            CHTTPClientItem *Items(int Index) const override { return GetItem(Index); };

            CHTTPClientItem *operator[] (int Index) const override { return Items(Index); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxy ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxyManager;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxy: public CHTTPClientItem, public CPollConnection {
        private:

            CHTTPServerConnection *m_pConnection;

            CHTTPRequest *m_Request;

        protected:

            CHTTPRequest *GetRequest();

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;
            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRequest(CHTTPClientConnection *AConnection) override;

        public:

            explicit CHTTPProxy(CHTTPProxyManager *AManager, CHTTPServerConnection *AConnection);

            CHTTPServerConnection *Connection() { return m_pConnection; }

            CHTTPServer *Server() { return dynamic_cast<CHTTPServer *> (m_pConnection->Server()); }

            CHTTPRequest *Request() { return GetRequest(); }

            void Close() override {
                m_pConnection = nullptr;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxyManager -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxyManager: public CHTTPClientManager {
            typedef CHTTPClientManager inherited;

        protected:

            CHTTPProxy *GetItem(int Index) const override;

        public:

            CHTTPProxyManager(): CHTTPClientManager() {

            };

            ~CHTTPProxyManager() override = default;

            CHTTPProxy *Add(CHTTPServerConnection *AConnection);

            CHTTPProxy *Items(int Index) const override { return GetItem(Index); };

            CHTTPProxy *operator[] (int Index) const override { return Items(Index); };
        };

    }
}

using namespace Delphi::HTTP;
}

#endif //DELPHI_HTTP_HPP
