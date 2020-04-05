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
#define DefaultAllowedMethods  _T("OPTIONS, GET")
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

        typedef struct uri_parser {

            enum {
                fl_hostname = 0,
                fl_port,
                fl_pathname,
                fl_search,
                fl_hash
            } flag;

            CString protocol;
            CString hostname;
            CString port;
            CString pathname;
            CString search;
            CString hash;

            uri_parser(): flag(fl_hostname) {

            }

            uri_parser(const uri_parser& uri): uri_parser() {
                assign(uri);
            }

            explicit uri_parser(const CString& uri): uri_parser() {
                parse(uri);
            }

            void assign(const uri_parser& uri) {
                protocol = uri.protocol;
                hostname = uri.hostname;
                port = uri.port;
                pathname = uri.pathname;
                search = uri.search;
                hash = uri.hash;
            };

            void clear() {
                flag = fl_hostname;
                protocol.Clear();
                hostname.Clear();
                port.Clear();
                pathname.Clear();
                search.Clear();
                hash.Clear();
            };

            void parse(const CString &uri) {
                clear();

                size_t pos = 0;
                size_t end = uri.Find("://", pos);

                if (end != CString::npos) {
                    protocol = uri.SubString(pos, end);
                    pos = end + 3;
                }

                TCHAR ch = uri.at(pos++);
                while (!IsCtl(ch)) {
                    if (ch == ':')
                        flag = fl_port;
                    if (ch == '/')
                        flag = fl_pathname;
                    if (ch == '?')
                        flag = fl_search;
                    if (ch == '#')
                        flag = fl_hash;

                    switch (flag) {
                        case fl_hostname:
                            hostname.Append(ch);
                            break;
                        case fl_port:
                            if (ch != ':')
                                port.Append(ch);
                            break;
                        case fl_pathname:
                            pathname.Append(ch);
                            break;
                        case fl_search:
                            search.Append(ch);
                            break;
                        case fl_hash:
                            hash.Append(ch);
                            break;
                    }

                    ch = uri.at(pos++);
                }
            };

            CString host() const {
                CString Result;

                Result = hostname;

                if (!port.IsEmpty()) {
                    Result << ":";
                    Result << port;
                }

                return Result;
            };

            CString origin() const {
                CString Result;

                Result = protocol;
                Result << "://";
                Result << host();

                return Result;
            };

            CString toString() const {
                CString Result;

                Result = origin();
                Result << pathname;
                Result << search;
                Result << hash;

                return Result;
            };

            CString href() const {
                return toString();
            };

            uri_parser& operator= (const uri_parser &uri) {
                if (this != &uri) {
                    assign(uri);
                }
                return *this;
            };

            uri_parser& operator= (const CString &uri) {
                parse(uri);
                return *this;
            };

            uri_parser& operator<< (const CString &url) {
                parse(href() + url);
                return *this;
            }

        } CLocation, *PLocation;
        //--------------------------------------------------------------------------------------------------------------

        typedef struct http_header {

            CString Name;
            CString Value;
            CStringList Options;

            http_header& operator= (const http_header& H) {
                if (this != &H) {
                    Name = H.Name;
                    Value = H.Value;
                    Options = H.Options;
                }
                return *this;
            };

            inline bool operator!= (const http_header& AValue) { return Name != AValue.Name; };
            inline bool operator== (const http_header& AValue) { return Name == AValue.Name; };

        } CHeader, *PHeader;

        //--------------------------------------------------------------------------------------------------------------

        //-- CHeaders --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHeaders {
        private:

            TList<CHeader> m_pList;

            CString m_NullValue;

            void Put(int Index, const CHeader& Header);

            CHeader& Get(int Index);
            const CHeader& Get(int Index) const;

        protected:

            int GetCount() const;

            const CString &GetValue(const CString &Name) const;
            const CString &GetValue(LPCTSTR Name) const;

        public:

            CHeaders() = default;

            ~CHeaders();

            void Clear();

            int IndexOfName(const CString &Name) const;
            int IndexOfName(LPCTSTR Name) const;

            void Insert(int Index, const CHeader& Header);

            int Add(const CHeader& Header);

            int AddPair(LPCTSTR lpszName, LPCTSTR lpszValue);
            int AddPair(LPCTSTR lpszName, const CString& Value);
            int AddPair(const CString& Name, const CString& Value);

            void Delete(int Index);

            void SetCount(int NewCount);

            CHeader& First();

            CHeader& Last();

            int Count() const { return GetCount(); }

            void Assign(const CHeaders& Headers);

            const CString &Values(const CString& Name) const { return GetValue(Name); };
            const CString &Values(LPCTSTR Name) const { return GetValue(Name); };

            CHeader& Headers(int Index) { return Get(Index); }
            const CHeader& Headers(int Index) const { return Get(Index); }

            void Headers(int Index, const CHeader& Header) { Put(Index, Header); }

            CHeaders& operator= (const CHeaders& H) {
                if (this != &H)
                    Assign(H);
                return *this;
            };

            CHeader& operator[](int Index) { return Get(Index); }
            const CHeader& operator[](int Index) const { return Get(Index); }

            CHeader& operator[](LPCTSTR Name) { return Headers(IndexOfName(Name)); }
            const CHeader& operator[](LPCTSTR Name) const { return Headers(IndexOfName(Name)); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CFormData -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef struct http_form_data {

            CHeaders Headers;

            CString Name;
            CString File;
            CString Data;

            http_form_data& operator= (const http_form_data& Value) {
                if (this != &Value) {
                    Name = Value.Name;
                    File = Value.File;
                    Data = Value.Data;
                }
                return *this;
            };

            inline bool operator!= (const http_form_data& Value) { return Name != Value.Name; };
            inline bool operator== (const http_form_data& Value) { return Name == Value.Name; };

        } CFormDataItem, *PFormDataItem;

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

        typedef struct http_request {

            CString Method;

            CString Uri;

            /// The uri parameters to be included in the request.
            CStringList Params;

            int VMajor;
            int VMinor;

            /// The headers to be included in the request.
            CHeaders Headers;

            /// The content length to be sent in the request.
            size_t ContentLength = 0;

            /// The content type of the reply.
            enum content_type
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

            CString Host;
            unsigned short Port;

            CString UserAgent;

            bool CloseConnection = false;

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

            /// Get a prepare request.
            static http_request *Prepare(http_request *ARequest, LPCTSTR AMethod, LPCTSTR AUri,
                                         LPCTSTR AContentType = nullptr);

            /// Add Authorization header to headers
            static http_request *Authorization(http_request *ARequest, LPCTSTR AMethod, LPCTSTR ALogin, LPCTSTR APassword);

            http_request &operator=(const http_request &Value) {
                if (this != &Value) {
                    Method = Value.Method;
                    Uri = Value.Uri;
                    Params = Value.Params;
                    VMajor = Value.VMajor;
                    VMinor = Value.VMinor;
                    Headers = Value.Headers;
                    ContentLength = Value.ContentLength;
                    ContentType = Value.ContentType;
                    Content = Value.Content;
                    Host = Value.Host;
                    Port = Value.Port;
                    UserAgent = Value.UserAgent;
                    CloseConnection = Value.CloseConnection;
                }

                return *this;
            };

        } CRequest, *PRequest;

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocket ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef struct web_socket_frame {

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
        } CWebSocketFrame, *PWebSocketFrame;

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

            typedef enum parser_state {
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
            } CParserState;

        }
        //--------------------------------------------------------------------------------------------------------------

        typedef struct http_context {
            LPCTSTR Begin;
            LPCTSTR End;
            size_t Size;
            int Result;
            Request::CParserState State;
            size_t ContentLength;
            TCHAR MIME[3] = {};
            size_t MimeIndex;

            http_context(LPCTSTR ABegin, size_t ASize, Request::CParserState AState = Request::method_start) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = 0;
                MimeIndex = 0;
            };

        } CHTTPContext, *PRequestContext;
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

        typedef struct http_reply
        {
            int VMajor;
            int VMinor;

            /// The status of the reply.
            enum status_type
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
            enum content_type
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

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue);

            /// Add header to headers.
            void AddHeader(LPCTSTR lpszName, const CString& Value);

            /// Add header to headers.
            void AddHeader(const CString& Name, const CString& Value);

            /// Get a prepare reply.
            static http_reply *GetReply(http_reply *AReply, status_type AStatus, LPCTSTR AContentType = nullptr);

            /// Get a stock reply.
            static http_reply *GetStockReply(http_reply *AReply, status_type AStatus);

        } CReply, *PReply;

        //--------------------------------------------------------------------------------------------------------------

        //-- CReplyParser ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace Reply {

            typedef enum parser_state {
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
                content
            } CParserState;
            //----------------------------------------------------------------------------------------------------------
        }

        typedef struct reply_parser_context {
            LPCTSTR Begin;
            LPCTSTR End;
            size_t Size;
            int Result;
            Reply::CParserState State;
            size_t ContentLength;
            TCHAR MIME[3] = {};
            size_t MimeIndex;

            reply_parser_context(LPCTSTR ABegin, size_t ASize, Reply::CParserState AState = Reply::http_version_h) {
                Begin = ABegin;
                End = ABegin + ASize;
                Size = ASize;
                Result = -1;
                State = AState;
                ContentLength = 0;
                MimeIndex = 0;
            };

        } CReplyContext, *PReplyContext;
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

            void SendStockReply(CReply::status_type AStatus, bool ASendNow = false);
            void SendReply(CReply::status_type AStatus, LPCTSTR AContentType = nullptr, bool ASendNow = false);
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

            bool CloseConnection() { return m_CloseConnection; };
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

        class CHTTPServer: public CAsyncServer {
        private:

            CString m_sDocRoot;

            void SetDocRoot(LPCTSTR AValue);

        protected:

            void DoTimeOut(CPollEventHandler *AHandler) override;

            void DoAccept(CPollEventHandler *AHandler) override;

            void DoRead(CPollEventHandler *AHandler) override;

            void DoWrite(CPollEventHandler *AHandler) override;

            bool DoCommand(CTCPConnection *AConnection) override;

            bool DoExecute(CTCPConnection *AConnection) override;

            void DoReply(CObject *Sender);

        public:

            explicit CHTTPServer(unsigned short AListen, LPCTSTR lpDocRoot);

            ~CHTTPServer() override = default;

            bool URLDecode(const CString& In, CString& Out);

            CString& DocRoot() { return m_sDocRoot; };
            void DocRoot(LPCTSTR AValue) { SetDocRoot(AValue); };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CRequest *ARequest)> COnHTTPClientRequestEvent;
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

            CHTTPServerConnection *m_Connection;

            CRequest *m_Request;

        protected:

            CRequest *GetRequest();

            void DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) override;
            void DoConnect(CPollEventHandler *AHandler) override;

            void DoRequest(CHTTPClientConnection *AConnection) override;

        public:

            explicit CHTTPProxy(CHTTPServerConnection *AConnection, CHTTPProxyManager *AManager);

            CHTTPServerConnection *Connection() { return m_Connection; }

            CHTTPServer *Server() { return dynamic_cast<CHTTPServer *> (m_Connection->Server()); }

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
