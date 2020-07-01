/*++

Library name:

  libdelphi

Module Name:

  Server.hpp

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

#define WS_FIN                  0x80u
#define WS_MASK                 0x80u

#define WS_OPCODE_CONTINUATION  0x00u
#define WS_OPCODE_TEXT          0x01u
#define WS_OPCODE_BINARY        0x02u
#define WS_OPCODE_CLOSE         0x08u
#define WS_OPCODE_PING          0x09u
#define WS_OPCODE_PONG          0x0Au

#define WS_PAYLOAD_LENGTH_16    126u
#define WS_PAYLOAD_LENGTH_63    127u

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

                    if (protocol == HTTPS_PREFIX) {
                        port = 443;
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
                    if (port != 80 && port != 443) {
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

            enum CTokenType {
                attUnknown = -1,
                attAccess,
                attRefresh
            } TokenType;

            CString Token;

            CAuthorization(): Schema(asUnknown), Type(atUnknown), TokenType(attUnknown) {

            }

            explicit CAuthorization(const CString& String): CAuthorization() {
                Parse(String);
            }

            void Parse(const CString& String) {
                if (String.IsEmpty())
                    throw CAuthorizationError("Data has not be empty.");

                if (String.SubString(0, 5) == "Basic") {

                    Schema = asBasic;

                    const CString LPassphrase(base64_decode(String.SubString(6)));

                    const size_t LPos = LPassphrase.Find(':');
                    if (LPos == CString::npos)
                        throw CAuthorizationError("Incorrect passphrase.");

                    Type = atOwner;

                    Username = LPassphrase.SubString(0, LPos);
                    Password = LPassphrase.SubString(LPos + 1);

                    if (Username.IsEmpty() || Password.IsEmpty())
                        throw CAuthorizationError("Username and password has not be empty.");

                } else if (String.SubString(0, 6) == "Bearer") {

                    Schema = asBearer;
                    Token = String.SubString(7);

                    if (Token.IsEmpty())
                        throw CAuthorizationError("Token has not be empty.");

                } else {
                    throw CAuthorizationError("Unknown schema.");
                }
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

        //-- CRequest --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        struct CRequest {

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
            size_t ContentLength = 0;

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

            bool CloseConnection = false;

            /// The Location interface represents a location (URL) as an object.
            CLocation Location;

            /// Clear content and headers.
            void Clear();

            void ToText();
            void ToJSON();

            /// Convert the request into a vector of buffers. The buffers do not own the
            /// underlying memory blocks, therefore the request object must remain valid and
            /// not be changed until the write operation has completed.
            void ToBuffers(CMemoryStream *AStream);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, const CString& Value);

            /// Add header to headers.
            void AddHeader(const CString& Name, const CString& Value);

            bool BuildLocation();
            void BuildCookies();

            /// Get a prepare request.
            static CRequest *Prepare(CRequest *ARequest, LPCTSTR AMethod, LPCTSTR AURI,
                    LPCTSTR AContentType = nullptr);

            /// Add Authorization header to headers
            static CRequest *Authorization(CRequest *ARequest, LPCTSTR AMethod, LPCTSTR ALogin, LPCTSTR APassword);

            CRequest &operator=(const CRequest &Value) {
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

                return *this;
            };

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

        class CWebSocketParser;

        class CWebSocket {
            friend CWebSocketParser;

        private:

            CWebSocketFrame m_Frame;

            CMemoryStream *m_Payload;

            size_t m_Size;

            void Encode(CMemoryStream *Stream);
            void Decode(CMemoryStream *Stream);

            void PayloadFromStream(CMemoryStream *Stream);
            void PayloadToStream(CMemoryStream *Stream);

        public:

            CWebSocket();

            ~CWebSocket();

            void Clear();

            size_t Size() const { return m_Size; };

            CWebSocketFrame &Frame() { return m_Frame; }
            const CWebSocketFrame &Frame() const { return m_Frame; }

            CMemoryStream *Payload() { return m_Payload; };

            void Close(CMemoryStream *Stream);
            void Ping(CMemoryStream *Stream);
            void Pong(CMemoryStream *Stream);

            void SaveToStream(CMemoryStream *Stream);
            void LoadFromStream(CMemoryStream *Stream);

            void SetPayload(CMemoryStream *Stream);
            void SetPayload(const CString &String);

            CWebSocket& operator<< (const CString &String) {
                SetPayload(String);
                return *this;
            }

            CWebSocket& operator>> (CString &String) {
                String.LoadFromStream(m_Payload);
                return *this;
            }

            CWebSocket& operator<< (CMemoryStream *Stream) {
                LoadFromStream(Stream);
                return *this;
            }

            CWebSocket& operator>> (CMemoryStream *Stream) {
                SaveToStream(Stream);
                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CRequestParser --------------------------------------------------------------------------------------------

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
            LPCTSTR Begin;
            LPCTSTR End;
            size_t Size;
            int Result;
            Request::CParserState State;
            size_t ContentLength;
            TCHAR MIME[3] = {};
            size_t MimeIndex;

            CHTTPContext(LPCTSTR ABegin, size_t ASize, Request::CParserState AState = Request::method_start) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = 0;
                MimeIndex = 0;
            };

        };
        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming HTTP requests.
        class CRequestParser {
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
            static int Consume(CRequest *ARequest, CHTTPContext &Context);

            /// Parse some data. The int return value is "1" when a complete request
            /// has been parsed, "0" if the data is invalid, "-1" when more
            /// data is required.
            static int Parse(CRequest *ARequest, CHTTPContext &Context);

            static int ParseFormData(CRequest *ARequest, CFormData &FormData);

        };

        /// Parser for incoming WebSocket requests.
        class CWebSocketParser {
        public:

            static void Parse(CWebSocket *ARequest, CMemoryStream *AStream);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CReply ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServerConnection;
        //--------------------------------------------------------------------------------------------------------------

        struct CReply
        {
            int VMajor;
            int VMinor;

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
            } Status;

            CString StatusString;
            CString StatusText;

            /// The content type of the reply.
            enum CContentType
            {
                html = 0,
                json,
                xml,
                text,
                sbin
            } ContentType = html;

            CString ServerName;

            CString AllowedMethods;

            bool CloseConnection = true;

            /// The headers to be included in the reply.
            CHeaders Headers;

            /// The content length to be receive in the reply.
            size_t ContentLength = 0;

            /// The content to be receive in the reply.
            CString Content;

            /// The cache file.
            CString CacheFile;

            /// Clear content and headers.
            void Clear();

            void ToText();
            void ToJSON();

            void StringToStatus();

            /// Convert the reply into a vector of buffers. The buffers do not own the
            /// underlying memory blocks, therefore the reply object must remain valid and
            /// not be changed until the write operation has completed.
            void ToBuffers(CMemoryStream *AStream);

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
            static CReply *GetReply(CReply *AReply, CStatusType AStatus, LPCTSTR AContentType = nullptr);

            /// Get a stock reply.
            static CReply *GetStockReply(CReply *AReply, CStatusType AStatus);

            static void AddUnauthorized(CReply *AReply, bool ABearer = false, LPCTSTR AError = nullptr, LPCTSTR AMessage = nullptr);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CReplyParser ----------------------------------------------------------------------------------------------

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

        struct CReplyContext {
            LPCTSTR Begin;
            LPCTSTR End;
            size_t Size;
            int Result;
            Reply::CParserState State;
            size_t ContentLength;
            size_t ChunkedLength;
            CString Chunked;
            TCHAR MIME[3] = {};
            size_t MimeIndex;

            CReplyContext(LPCTSTR ABegin, size_t ASize, Reply::CParserState AState = Reply::http_version_h) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = 0;
                ChunkedLength = 0;
                MimeIndex = 0;
            };

        };
        //--------------------------------------------------------------------------------------------------------------

        /// Parser for incoming requests.
        class CReplyParser {
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
            static int Consume(CReply *AReply, CReplyContext& Context);

            /// Parse some data. The int return value is "1" when a complete request
            /// has been parsed, "0" if the data is invalid, "-1" when more
            /// data is required.
            static int Parse(CReply *AReply, CReplyContext& Context);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServerConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServer;
        //--------------------------------------------------------------------------------------------------------------

        enum CHTTPConnectionStatus { csConnected = 0,
                csWaitRequest, csRequestOk, csRequestReady, csRequestSent, csRequestError,
                csWaitReply, csReplyOk, csReplyReady, csReplySent, csReplyError
        };
        //--------------------------------------------------------------------------------------------------------------

        enum CHTTPProtocol { pHTTP = 0, pWebSocket };

        class CHTTPServerConnection: public CTCPServerConnection {
            typedef CTCPServerConnection inherited;

        private:

            CRequest *m_Request;
            CReply *m_Reply;

            CWebSocket *m_WSRequest;
            CWebSocket *m_WSReply;

            /// The current state of the parser.
            Request::CParserState m_State;

            CHTTPConnectionStatus m_ConnectionStatus;

            CHTTPProtocol m_Protocol;

            bool m_CloseConnection;

            CNotifyEvent m_OnRequest;
            CNotifyEvent m_OnReply;

            void ParseHTTP(CMemoryStream *Stream);
            void ParseWebSocket(CMemoryStream *Stream);

        protected:

            CRequest *GetRequest();
            CReply *GetReply();

            CWebSocket *GetWSRequest();
            CWebSocket *GetWSReply();

            void DoRequest();
            void DoReply();

        public:

            explicit CHTTPServerConnection(CPollSocketServer *AServer);

            ~CHTTPServerConnection() override;

            void Clear();

            bool ParseInput();

            CHTTPServer *HTTPServer() { return (CHTTPServer *) Server(); }

            CRequest *Request() { return GetRequest(); }
            CReply *Reply() { return GetReply(); }

            CWebSocket *WSRequest() { return GetWSRequest(); }
            CWebSocket *WSReply() { return GetWSReply(); }

            CHTTPProtocol Protocol() const { return m_Protocol; }

            bool CloseConnection() const { return m_CloseConnection; }
            void CloseConnection(bool Value) { m_CloseConnection = Value; }

            CHTTPConnectionStatus ConnectionStatus() { return m_ConnectionStatus; }
            void ConnectionStatus(CHTTPConnectionStatus Value) { m_ConnectionStatus = Value; }

            void SendStockReply(CReply::CStatusType AStatus, bool ASendNow = false);
            void SendReply(CReply::CStatusType AStatus, LPCTSTR AContentType = nullptr, bool ASendNow = false);
            void SendReply(bool ASendNow = false);

            void SwitchingProtocols(const CString &Accept, const CString &Protocol);
            void SendWebSocket(bool ASendNow = false);

            void SendWebSocketPing(bool ASendNow = false);
            void SendWebSocketPong(bool ASendNow = false);
            void SendWebSocketClose(bool ASendNow = false);

            const CNotifyEvent &OnRequest() { return m_OnRequest; }
            void OnRequest(CNotifyEvent && Value) { m_OnRequest = Value; }

            const CNotifyEvent &OnReply() { return m_OnReply; }
            void OnReply(CNotifyEvent && Value) { m_OnReply = Value; }

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPClientConnection: public CTCPClientConnection {
            typedef CTCPClientConnection inherited;

        private:

            CRequest *m_Request;

            CReply *m_Reply;

            /// The current state of the parser.
            Reply::CParserState m_State;

            CHTTPConnectionStatus m_ConnectionStatus;

            bool m_CloseConnection;

            CNotifyEvent m_OnRequest;
            CNotifyEvent m_OnReply;

        protected:

            CRequest *GetRequest();

            CReply *GetReply();

            void DoRequest();
            void DoReply();

        public:

            explicit CHTTPClientConnection(CPollSocketClient *AClient);

            ~CHTTPClientConnection() override;

            void Clear();

            bool ParseInput();

            CRequest *Request() { return GetRequest(); }

            CReply *Reply() { return GetReply(); };

            bool CloseConnection() const { return m_CloseConnection; };
            void CloseConnection(bool Value) { m_CloseConnection = Value; };

            CHTTPConnectionStatus ConnectionStatus() { return m_ConnectionStatus; };
            void ConnectionStatus(CHTTPConnectionStatus Value) { m_ConnectionStatus = Value; };

            void SendRequest(bool ASendNow = false);

            const CNotifyEvent &OnRequest() { return m_OnRequest; }
            void OnRequest(CNotifyEvent && Value) { m_OnRequest = Value; }

            const CNotifyEvent &OnReply() { return m_OnReply; }
            void OnReply(CNotifyEvent && Value) { m_OnReply = Value; }

        }; // CHTTPServerConnection

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServer -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPairs<CJSON> CSites;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPServer: public CAsyncServer {
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

            static CString URLEncode(const CString& In, char Space = '+');
            static CString URLDecode(const CString& In, char Space = '+');
            static bool URLDecode(const CString& In, CString& Out, char Space = '+');

            CProviders& Providers() { return m_Providers; };
            const CProviders& Providers() const { return m_Providers; };

            CSites& Sites() { return m_Sites; };
            const CSites& Sites() const { return m_Sites; };

            CHTTPServer &operator = (const CHTTPServer &Server) {
                if (&Server != this) {
                    SetIOHandler(Server.IOHandler());
                    SetBindings(Server.Bindings());

                    m_Providers = Server.m_Providers;
                    m_Sites = Server.m_Sites;

                    m_ActiveLevel = Server.m_ActiveLevel;
                }

                return *this;
            }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------
        class CHTTPClient;

        typedef std::function<void (CHTTPClient *Sender, CRequest *Request)> COnHTTPClientRequestEvent;
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

            explicit CHTTPClient(LPCTSTR AHost, unsigned short APort);

            ~CHTTPClient() override = default;

            const COnHTTPClientRequestEvent &OnRequest() { return m_OnRequest; }
            void OnRequest(COnHTTPClientRequestEvent && Value) { m_OnRequest = Value; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxy ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxyManager;
        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxy: public CCollectionItem, public CHTTPClient {
        private:

            CHTTPServerConnection *m_pConnection;

            CRequest *m_Request;

        protected:

            CRequest *GetRequest();

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;
            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRequest(CHTTPClientConnection *AConnection) override;

        public:

            explicit CHTTPProxy(CHTTPServerConnection *AConnection, CHTTPProxyManager *AManager);

            CHTTPServerConnection *Connection() { return m_pConnection; }

            CHTTPServer *Server() { return dynamic_cast<CHTTPServer *> (m_pConnection->Server()); }

            CRequest *Request() { return GetRequest(); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxyManager -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHTTPProxyManager: public CCollection {
        public:

            explicit CHTTPProxyManager(): CCollection(this) {

            };

            ~CHTTPProxyManager() override = default;

            CHTTPProxy* Add(CHTTPServerConnection *AConnection);

        };

    }
}

using namespace Delphi::HTTP;
}

#endif //DELPHI_HTTP_HPP
