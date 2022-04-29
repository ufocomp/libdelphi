/*++

Library name:

  libdelphi

Module Name:

  HTTP.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/HTTP.hpp"
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace HTTP {

        #define StringArrayToStream(Stream, Buf) (Stream).Write((Buf), sizeof((Buf)) - sizeof(TCHAR))

        namespace Mapping {

            struct CMapping {
                LPCTSTR Ext;
                LPCTSTR MimeType;
                bool IsText;
            } CMappings[] = {

                    { _T(".htm"),       _T("text/html"),                        true  },
                    { _T(".html"),      _T("text/html"),                        true  },
                    { _T(".css"),       _T("text/css"),                         true  },
                    { _T(".js"),        _T("text/javascript"),                  true  },
                    { _T(".php"),       _T("text/php"),                         true  },

                    { _T(".png"),       _T("image/png"),                        false },
                    { _T(".jpg"),       _T("image/jpeg"),                       false },
                    { _T(".jpeg"),      _T("image/jpeg"),                       false },
                    { _T(".gif"),       _T("image/gif"),                        false },
                    { _T(".tif"),       _T("image/tiff"),                       false },
                    { _T(".ico"),       _T("image/vnd.microsoft.icon"),         false },

                    { _T(".jpe"),       _T("image/jpeg"),                       false },
                    { _T(".jfif"),      _T("image/jpeg"),                       false },

                    { _T(".txt"),       _T("text/plain"),                       true  },
                    { _T(".md"),        _T("text/markdown"),                    true  },
                    { _T(".markdown"),  _T("text/markdown"),                    true  },
                    { _T(".cmd"),       _T("text/cmd"),                         true  },
                    { _T(".appcache"),  _T("text/cache-manifest"),              true  },

                    { _T(".svg"),       _T("image/svg+xml"),                    true  },
                    { _T(".svgz"),      _T("image/svg+xml"),                    true  },

                    { _T(".ttf"),       _T("application/x-font-ttf"),           false },
                    { _T(".otf"),       _T("application/x-font-opentype"),      false },
                    { _T(".woff"),      _T("application/x-font-woff"),          false },
                    { _T(".woff2"),     _T("application/x-font-woff2"),         false },
                    { _T(".eot"),       _T("application/vnd.ms-fontobject"),    false },
                    { _T(".sfnt"),      _T("application/font-sfnt"),            false },

                    { _T(".xml"),       _T("application/xml"),                  true  },
                    { _T(".json"),      _T("application/json"),                 true  },

                    { _T(".pdf"),       _T("application/pdf"),                  false },

                    { _T(".zip"),       _T("application/zip"),                  false },
                    { _T(".gz"),        _T("application/gzip"),                 false },
                    { _T(".tgz"),       _T("application/gzip"),                 false },

                    { _T(".tgz"),       _T("application/gzip"),                 false },
                    { _T(".torrent"),   _T("application/x-bittorrent"),         false },

                    { _T(".bin"),       _T("application/octet-stream"),         false },
                    { _T(".exe"),       _T("application/octet-stream"),         false },

                    {nullptr, nullptr, false} // Marks end of list.
            };

            LPCTSTR ExtToType(LPCTSTR Ext)
            {
                if (Assigned(Ext)) {
                    for (CMapping *m = CMappings; m->Ext; ++m) {
                        if (SameText(m->Ext, Ext)) {
                            return m->MimeType;
                        }
                    }
                }

                return _T("text/plain");
            }

            bool IsText(LPCTSTR MimeType)
            {
                if (Assigned(MimeType)) {
                    for (CMapping *m = CMappings; m->MimeType; ++m) {
                        if (SameText(m->MimeType, MimeType)) {
                            return m->IsText;
                        }
                    }
                }

                return false;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CFormData -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CFormData::~CFormData() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::Clear() {
            m_pList.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::Put(int Index, const CFormDataItem &Item) {
            m_pList.Insert(Index, Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        CFormDataItem &CFormData::Get(int Index) {
            return m_pList.Items(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CFormDataItem &CFormData::Get(int Index) const {
            return m_pList.Items(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CFormData::GetCount() const {
            return m_pList.GetCount();
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CFormData::GetData(const CString &Name) const {
            int Index = IndexOfName(Name);
            if (Index != -1) {
                const CFormDataItem& Item = Get(Index);
                return Item.Data;
            }
            return m_NullData;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CFormData::GetData(LPCTSTR Name) const {
            int Index = IndexOfName(Name);
            if (Index != -1) {
                const CFormDataItem& Item = Get(Index);
                return Item.Data;
            }
            return m_NullData;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CFormData::IndexOfName(const CString &Name) const {
            for (int I = 0; I < GetCount(); ++I) {
                const CFormDataItem& Item = Get(I);
                if (Item.Name.Lower() == Name)
                    return I;
            }
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CFormData::IndexOfName(LPCTSTR Name) const {
            for (int I = 0; I < GetCount(); ++I) {
                const CFormDataItem& Item = Get(I);
                if (Item.Name.Lower() == Name)
                    return I;
            }
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::Insert(int Index, const CFormDataItem &Item) {
            Put(Index, Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CFormData::Add(const CFormDataItem &Item) {
            int Result = GetCount();
            Insert(Result, Item);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::Delete(int Index) {
            m_pList.Delete(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::SetCount(int NewCount) {
            int LCount = GetCount();
            if (NewCount > LCount) {
                for (int I = LCount; I < NewCount; ++I)
                    Add(CFormDataItem());
            } else {
                for (int I = LCount - 1; I >= NewCount; --I)
                    Delete(I);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CFormDataItem &CFormData::First() {
            return m_pList.First();
        }
        //--------------------------------------------------------------------------------------------------------------

        CFormDataItem &CFormData::Last() {
            return m_pList.Last();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CFormData::Assign(const CFormData &Value) {
            Clear();
            for (int I = 0; I < Value.GetCount(); ++I) {
                Add(Value[I]);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace MiscStrings {
            const TCHAR http[] = _T("HTTP/1.1");
            const TCHAR question[] = _T("?");
            const TCHAR ampersand[] = _T("&");
            //const TCHAR dot[] = _T(".");
            const TCHAR space[] = _T(" ");
            const TCHAR separator[] = _T(": ");
            const TCHAR crlf[] = _T("\r\n");
        } // namespace MiscStrings

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPRequest ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::ToBuffers(CMemoryStream &Stream) {

            Method.SaveToStream(Stream);
            StringArrayToStream(Stream, MiscStrings::space);

            URI.SaveToStream(Stream);
            for (int i = 0; i < Params.Count(); ++i) {
                if (i == 0) {
                    StringArrayToStream(Stream, MiscStrings::question);
                } else {
                    StringArrayToStream(Stream, MiscStrings::ampersand);
                }
                Params[i].SaveToStream(Stream);
            }
            StringArrayToStream(Stream, MiscStrings::space);

            StringArrayToStream(Stream, MiscStrings::http);
            StringArrayToStream(Stream, MiscStrings::crlf);

            for (int i = 0; i < Headers.Count(); ++i) {
                const auto &H = Headers[i];
                H.Name().SaveToStream(Stream);
                StringArrayToStream(Stream, MiscStrings::separator);
                H.Value().SaveToStream(Stream);
                StringArrayToStream(Stream, MiscStrings::crlf);
            }

            StringArrayToStream(Stream, MiscStrings::crlf);
            Content.SaveToStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::Clear() {
            VMajor = 0;
            VMinor = 0;
            ContentLength = 0;
            Method.Clear();
            URI.Clear();
            Params.Clear();
            Headers.Clear();
            Content.Clear();
            Location.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue) {
            Headers.AddPair(lpszName, lpszValue);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::AddHeader(LPCTSTR lpszName, const CString &Value) {
            Headers.AddPair(lpszName, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::AddHeader(const CString &Name, const CString &Value) {
            Headers.AddPair(Name, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::ToText() {
            CString Temp;
            TCHAR ch;
            if (!Content.IsEmpty()) {
                Temp = Content;
                Content.Clear();
                for (size_t i = 0; i < Temp.Size(); ++i) {
                    ch = Temp[i];
                    if (!IsCtl(ch) || (ch == '\t') || (ch == '\r') || (ch == '\n'))
                        Content.Append(ch);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::ToJSON() {
            CString Temp;
            if (!Content.IsEmpty()) {
                Temp = Content;
                Content.Clear();
                for (size_t i = 0; i < Temp.Size(); ++i) {
                    switch (Temp[i]) {
                        case 8:
                            Content.Append(92);
                            Content.Append('b');
                            break;
                        case '\n':
                            Content.Append(92);
                            Content.Append('n');
                            break;
                        case 12:
                            Content.Append(92);
                            Content.Append('f');
                            break;
                        case '\r':
                            Content.Append(92);
                            Content.Append('r');
                            break;
                        case '\t':
                            Content.Append(92);
                            Content.Append('t');
                            break;
                        default:
                            Content.Append(Temp[i]);
                            break;
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPRequest::Prepare(CHTTPRequest *ARequest, LPCTSTR AMethod, LPCTSTR AURI, LPCTSTR AContentType,
                LPCTSTR AConnection) {

            TCHAR szSize[_INT_T_LEN + 1] = {0};

            ARequest->VMajor = 1;
            ARequest->VMinor = 1;

            ARequest->Method = AMethod;
            ARequest->URI = AURI;

            ARequest->AddHeader(_T("Host"), ARequest->Location.Host());
            ARequest->AddHeader(_T("User-Agent"), ARequest->UserAgent);
            ARequest->AddHeader(_T("Accept"), _T("*/*"));

            if (!ARequest->Content.IsEmpty()) {
                ARequest->AddHeader(_T("Accept-Ranges"), _T("bytes"));

                if (AContentType == nullptr) {
                    switch (ARequest->ContentType) {
                        case CContentType::html:
                            AContentType = _T("text/html");
                            break;
                        case CContentType::json:
                            AContentType = _T("application/json");
                            ARequest->ToJSON();
                            break;
                        case CContentType::xml:
                            AContentType = _T("application/xml");
                            ARequest->ToText();
                            break;
                        case CContentType::text:
                            AContentType = _T("text/plain");
                            ARequest->ToText();
                            break;
                        case CContentType::sbin:
                            AContentType = _T("application/octet-stream");
                            break;
                        default:
                            AContentType = _T("text/plain");
                            break;
                    }
                }

                ARequest->AddHeader(_T("Content-Type"), AContentType);
                ARequest->AddHeader(_T("Content-Length"), IntToStr((int) ARequest->Content.Size(), szSize, sizeof(szSize)));
            }

            if (AConnection == nullptr) {
                if (ARequest->CloseConnection)
                    ARequest->AddHeader(_T("Connection"), _T("close"));
                else
                    ARequest->AddHeader(_T("Connection"), _T("keep-alive"));
            } else {
                ARequest->AddHeader(_T("Connection"), AConnection);
            }

            return ARequest;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPRequest::Authorization(CHTTPRequest *ARequest, LPCTSTR AMethod, LPCTSTR ALogin,
                LPCTSTR APassword) {

            CString sPassphrase, sAuthorization;

            sPassphrase = ALogin;
            sPassphrase << ":";
            sPassphrase << APassword;

            sAuthorization = AMethod;
            sAuthorization << " ";
            sAuthorization << base64_encode(sPassphrase);

            ARequest->AddHeader("Authorization", sAuthorization);

            return ARequest;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPRequest::BuildLocation() {
            CString Protocol;
            const auto& Host = Headers[_T("Host")];
            if (Host.Find(':') == CString::npos) {
                Protocol = Headers[_T("X-Forwarded-Proto")];
                if (!Protocol.IsEmpty())
                    Protocol << "://";
            }
            CString decodeURI;
            if (!CHTTPServer::URLDecode(URI, decodeURI))
                return false;
            Location = Protocol + Host + decodeURI;
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPRequest::BuildCookies() {
            const auto& Cookie = Headers[_T("Cookie")];
            if (!Cookie.empty()) {
                SplitColumns(Cookie, Cookies, ';');
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPRequestParser ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::Consume(CHTTPRequest *ARequest, CHTTPContext& Context) {
            size_t ContentLength = 0;

            const auto BufferSize = Context.End - Context.Begin;
            const auto ch = (TCHAR) *Context.Begin++;

            switch (Context.State) {
                case Request::method_start:
                    if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::method;
                        ARequest->Method.Append(ch);
                        return -1;
                    }
                case Request::method:
                    if (ch == ' ') {
                        Context.State = Request::uri;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        ARequest->Method.Append(ch);
                        return -1;
                    }
                case Request::uri_start:
                    if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::uri;
                        ARequest->URI.Append(ch);
                        return -1;
                    }
                case Request::uri:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '?') {
                        ARequest->URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(ch);
                        return -1;
                    }
                case Request::uri_param_start:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '&') {
                        ARequest->URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(ch);
                        ARequest->Params.Add(ch);
                        Context.State = Request::uri_param;
                        return -1;
                    }
                case Request::uri_param:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '&') {
                        ARequest->URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (ch == '#') {
                        ARequest->URI.Append(ch);
                        Context.State = Request::uri;
                        return -1;
                    } else if (ch == '%') {
                        ARequest->URI.Append(ch);
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::uri_param_mime;
                        return -1;
                    } else if (ch == '+') {
                        ARequest->URI.Append(ch);
                        ARequest->Params.back().Append(' ');
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(ch);
                        ARequest->Params.back().Append(ch);
                        return -1;
                    }
                case Request::http_version_h:
                    if (ch == 'H') {
                        Context.State = Request::http_version_t_1;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_t_1:
                    if (ch == 'T') {
                        Context.State = Request::http_version_t_2;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_t_2:
                    if (ch == 'T') {
                        Context.State = Request::http_version_p;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_p:
                    if (ch == 'P') {
                        Context.State = Request::http_version_slash;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_slash:
                    if (ch == '/') {
                        ARequest->VMajor = 0;
                        ARequest->VMinor = 0;
                        Context.State = Request::http_version_major_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_major_start:
                    if (IsDigit(ch)) {
                        ARequest->VMajor = ARequest->VMajor * 10 + ch - '0';
                        Context.State = Request::http_version_major;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_major:
                    if (ch == '.') {
                        Context.State = Request::http_version_minor_start;
                        return -1;
                    } else if (IsDigit(ch)) {
                        ARequest->VMajor = ARequest->VMajor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_minor_start:
                    if (IsDigit(ch)) {
                        ARequest->VMinor = ARequest->VMinor * 10 + ch - '0';
                        Context.State = Request::http_version_minor;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_minor:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_1;
                        return -1;
                    } else if (IsDigit(ch)) {
                        ARequest->VMinor = ARequest->VMinor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::expecting_newline_1:
                    if (ch == '\n') {
                        Context.State = Request::header_line_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::header_line_start:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_3;
                        return -1;
                    } else if ((ARequest->Headers.Count() > 0) && (ch == ' ' || ch == '\t')) {
                        Context.State = Request::header_lws;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Add(CHeader());
                        ARequest->Headers.Last().Name().Append(ch);

                        Context.State = Request::header_name;
                        return -1;
                    }
                case Request::header_lws:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (ch == ' ' || ch == '\t') {
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value;
                        return -1;
                    }
                case Request::header_name:
                    if (ch == ':') {
                        Context.State = Request::space_before_header_value;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Name().Append(ch);
                        return -1;
                    }
                case Request::space_before_header_value:
                    if (ch == ' ') {
                        Context.State = Request::header_value;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::header_value:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        ARequest->Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Request::header_value_options_start:
                    if ((ch == ' ' || ch == '\t')) {
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(ch);
                        ARequest->Headers.Last().Data().Add(ch);

                        Context.State = Request::header_value_options;
                        return -1;
                    }
                case Request::header_value_options:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        ARequest->Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(ch);
                        ARequest->Headers.Last().Data().back().Append(ch);
                        return -1;
                    }
                case Request::expecting_newline_2:
                    if (ch == '\n') {
                        Context.State = Request::header_line_start;
                        return -1;
                    }

                    return 0;

                case Request::expecting_newline_3:
                    if (ch == '\n') {
                        ARequest->ContentLength = 0;
                        Context.ContentLength = BufferSize - 1;

                        if (ARequest->Headers.Count() > 0) {
                            if (!ARequest->BuildLocation())
                                return 0;

                            ARequest->BuildCookies();

                            const auto& contentLength = ARequest->Headers[_T("Content-Length")];
                            if (!contentLength.IsEmpty()) {
                                Context.ContentLength = strtoul(contentLength.c_str(), nullptr, 0);
                            }

                            const auto& contentType = ARequest->Headers[_T("Content-Type")];
                            if (contentType.Find("application/x-www-form-urlencoded") != CString::npos) {
                                ARequest->ContentLength = Context.ContentLength;
                                Context.State = Request::form_data_start;
                                return -1;
                            }
                        }

                        if (Context.ContentLength > 0) {
                            Context.State = Request::content;
                            return -1;
                        }

                        return 1;
                    }

                    return 0;

                case Request::content:
                    if (Context.ContentLength == 0)
                        return 1;

                    ContentLength = Context.ContentLength > BufferSize ? BufferSize : Context.ContentLength;

                    ARequest->Content.Append((LPCSTR) Context.Begin - 1, ContentLength);
                    ARequest->ContentLength += ContentLength;

                    Context.Begin += ContentLength - 1;
                    Context.ContentLength -= ContentLength;

                    return Context.ContentLength == 0 ? 1 : -1;

                case Request::form_data_start:
                    ARequest->Content.Append(ch);

                    if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::form_data;
                        ARequest->FormData.Add(ch);
                        return -1;
                    }
                case Request::form_data:
                    ARequest->Content.Append(ch);

                    if (ch == '\n') {
                        return 1;
                    } else if (ch == '\r') {
                        return -1;
                    } else if (ch == '&') {
                        Context.State = Request::form_data_start;
                        return -1;
                    } else if (ch == '+') {
                        ARequest->FormData.back().Append(' ');
                        return -1;
                    } else if (ch == '%') {
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::form_mime;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        ARequest->FormData.back().Append(ch);

                        if (ARequest->Content.Size() < ARequest->ContentLength) {
                            return -1;
                        }
                    }

                    return 1;
                case Request::uri_param_mime:
                    ARequest->URI.Append(ch);
                    Context.MIME[Context.MimeIndex++] = ch;
                    if (Context.MimeIndex == 2) {
                        ARequest->Params.back().Append((TCHAR) HexToDec(Context.MIME));
                        Context.State = Request::uri_param;
                    }
                    return -1;
                case Request::form_mime:
                    ARequest->Content.Append(ch);
                    Context.MIME[Context.MimeIndex++] = ch;
                    if (Context.MimeIndex == 2) {
                        ARequest->FormData.back().Append((TCHAR) HexToDec(Context.MIME));
                        Context.State = Request::form_data;
                    }
                    return -1;
                default:
                    return 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::Parse(CHTTPRequest *ARequest, CHTTPContext& Context) {
            Context.Result = -1;
            while ((Context.Result == -1) && (Context.Begin != Context.End)) {
                Context.Result = Consume(ARequest, Context);
            }
            return Context.Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::ParseFormData(CHTTPRequest *ARequest, CFormData& FormData) {
            const CString& Content = ARequest->Content;

            if (Content.IsEmpty())
                return 0;

            try {
                const auto& contentType = ARequest->Headers.Pairs("content-type");
                if (contentType.Value().Find("multipart/form-data") == CString::npos)
                    return 0;

                const CString CRLF(MiscStrings::crlf);
                const auto& boundary = contentType.Data()["boundary"];

                CString Boundary(CRLF);

                if (boundary.IsEmpty()) {
                    int i = 0;
                    char ch = Content.at(i++);
                    while (ch != 0 && ch != '\r') {
                        Boundary.Append(ch);
                        ch = Content.at(i++);
                    }
                } else {
                    Boundary << "--";
                    Boundary << boundary;
                }

                CStringList Data;

                size_t DataBegin = Boundary.Size();
                size_t DataEnd = Content.Find(Boundary, DataBegin);

                while (DataEnd != CString::npos) {
                    Data.Add(Content.SubString(DataBegin, DataEnd - DataBegin));
                    DataBegin = DataEnd + Boundary.Size() + CRLF.Size();
                    DataEnd = Content.Find(Boundary, DataBegin);
                }

                CStringList& formData = ARequest->FormData;
                CHTTPRequest Request;

                for (int I = 0; I < Data.Count(); I++) {
                    CHTTPContext Context = CHTTPContext((LPCBYTE) Data[I].Data(), Data[I].Size());
                    Context.State = Request::CParserState::header_line_start;
                    const int Result = Parse(&Request, Context);
                    if (Result == 1) {
                        FormData.Add(CFormDataItem());
                        CFormDataItem& DataItem = FormData.Last();

                        const auto& contentDisposition = Request.Headers.Pairs("content-disposition");

                        DataItem.Name = contentDisposition.Data()["name"];
                        DataItem.File = contentDisposition.Data()["filename"];
                        DataItem.Data = Request.Content;

                        if (DataItem.Data.Find('\n') == CString::npos) {
                            formData.AddPair(DataItem.Name, DataItem.Data);
                        }
                    }
                    Request.Clear();
                }

                return FormData.Count();
            } catch (...) {
                return -1;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPRequestParser::IsChar(int c) {
            return c >= 0 && c <= 127;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPRequestParser::IsCtl(int c) {
            return (c >= 0 && c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPRequestParser::IsTSpecial(int c) {
            switch (c) {
                case '(':
                case ')':
                case '<':
                case '>':
                case '@':
                case ',':
                case ';':
                case ':':
                case '\\':
                case '"':
                case '/':
                case '[':
                case ']':
                case '?':
                case '=':
                case '{':
                case '}':
                case ' ':
                case '\t':
                    return true;
                default:
                    return false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPRequestParser::IsDigit(int c) {
            return c >= '0' && c <= '9';
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocketParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CWebSocketParser::Parse(CWebSocket *ARequest, const CMemoryStream &Stream) {
            return ARequest->LoadFromStream(Stream);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPReply ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        static const CHTTPReply::CStatusType StatusArray[] = {
                CHTTPReply::switching_protocols,
                CHTTPReply::ok,
                CHTTPReply::created,
                CHTTPReply::accepted,
                CHTTPReply::non_authoritative,
                CHTTPReply::no_content,
                CHTTPReply::multiple_choices,
                CHTTPReply::moved_permanently,
                CHTTPReply::moved_temporarily,
                CHTTPReply::see_other,
                CHTTPReply::not_modified,
                CHTTPReply::bad_request,
                CHTTPReply::unauthorized,
                CHTTPReply::forbidden,
                CHTTPReply::not_found,
                CHTTPReply::not_allowed,
                CHTTPReply::internal_server_error,
                CHTTPReply::not_implemented,
                CHTTPReply::bad_gateway,
                CHTTPReply::service_unavailable,
                CHTTPReply::gateway_timeout
        };
        //--------------------------------------------------------------------------------------------------------------

        namespace StatusStrings {

            const TCHAR switching_protocols[] = _T("Switching Protocols");
            const TCHAR ok[] = _T("OK");
            const TCHAR created[] = _T("Created");
            const TCHAR accepted[] = _T("Accepted");
            const TCHAR non_authoritative[] = _T("Non-Authoritative Information");
            const TCHAR no_content[] = _T("No Content");
            const TCHAR multiple_choices[] = _T("Multiple Choices");
            const TCHAR moved_permanently[] = _T("Moved Permanently");
            const TCHAR moved_temporarily[] = _T("Moved Temporarily");
            const TCHAR see_other[] = _T("See Other");
            const TCHAR not_modified[] = _T("Not Modified");
            const TCHAR bad_request[] = _T("Bad Request");
            const TCHAR unauthorized[] = _T("Unauthorized");
            const TCHAR forbidden[] = _T("Forbidden");
            const TCHAR not_found[] = _T("Not Found");
            const TCHAR not_allowed[] = _T("Method Not Allowed");
            const TCHAR internal_server_error[] = _T("Internal Server Error");
            const TCHAR not_implemented[] = _T("Not Implemented");
            const TCHAR bad_gateway[] = _T("Bad Gateway");
            const TCHAR service_unavailable[] = _T("Service Unavailable");
            const TCHAR gateway_timeout[] = _T("Gateway Timeout");

            size_t ToBuffer(CHTTPReply::CStatusType AStatus, CStream &Stream) {
                switch (AStatus) {
                    case CHTTPReply::switching_protocols:
                        return StringArrayToStream(Stream, switching_protocols);
                    case CHTTPReply::ok:
                        return StringArrayToStream(Stream, ok);
                    case CHTTPReply::created:
                        return StringArrayToStream(Stream, created);
                    case CHTTPReply::accepted:
                        return StringArrayToStream(Stream, accepted);
                    case CHTTPReply::non_authoritative:
                        return StringArrayToStream(Stream, non_authoritative);
                    case CHTTPReply::no_content:
                        return StringArrayToStream(Stream, no_content);
                    case CHTTPReply::multiple_choices:
                        return StringArrayToStream(Stream, multiple_choices);
                    case CHTTPReply::moved_permanently:
                        return StringArrayToStream(Stream, moved_permanently);
                    case CHTTPReply::moved_temporarily:
                        return StringArrayToStream(Stream, moved_temporarily);
                    case CHTTPReply::see_other:
                        return StringArrayToStream(Stream, see_other);
                    case CHTTPReply::not_modified:
                        return StringArrayToStream(Stream, not_modified);
                    case CHTTPReply::bad_request:
                        return StringArrayToStream(Stream, bad_request);
                    case CHTTPReply::unauthorized:
                        return StringArrayToStream(Stream, unauthorized);
                    case CHTTPReply::forbidden:
                        return StringArrayToStream(Stream, forbidden);
                    case CHTTPReply::not_found:
                        return StringArrayToStream(Stream, not_found);
                    case CHTTPReply::not_allowed:
                        return StringArrayToStream(Stream, not_allowed);
                    case CHTTPReply::internal_server_error:
                        return StringArrayToStream(Stream, internal_server_error);
                    case CHTTPReply::not_implemented:
                        return StringArrayToStream(Stream, not_implemented);
                    case CHTTPReply::bad_gateway:
                        return StringArrayToStream(Stream, bad_gateway);
                    case CHTTPReply::service_unavailable:
                        return StringArrayToStream(Stream, service_unavailable);
                    case CHTTPReply::gateway_timeout:
                        return StringArrayToStream(Stream, gateway_timeout);
                    default:
                        return StringArrayToStream(Stream, internal_server_error);
                }
            }

            void ToString(CHTTPReply::CStatusType AStatus, CString &AString) {
                switch (AStatus) {
                    case CHTTPReply::switching_protocols:
                        AString = switching_protocols;
                        break;
                    case CHTTPReply::ok:
                        AString = ok;
                        break;
                    case CHTTPReply::created:
                        AString = created;
                        break;
                    case CHTTPReply::accepted:
                        AString = accepted;
                        break;
                    case CHTTPReply::non_authoritative:
                        AString = non_authoritative;
                        break;
                    case CHTTPReply::no_content:
                        AString = no_content;
                        break;
                    case CHTTPReply::multiple_choices:
                        AString = multiple_choices;
                        break;
                    case CHTTPReply::moved_permanently:
                        AString = moved_permanently;
                        break;
                    case CHTTPReply::moved_temporarily:
                        AString = moved_temporarily;
                        break;
                    case CHTTPReply::see_other:
                        AString = see_other;
                        break;
                    case CHTTPReply::not_modified:
                        AString = not_modified;
                        break;
                    case CHTTPReply::bad_request:
                        AString = bad_request;
                        break;
                    case CHTTPReply::unauthorized:
                        AString = unauthorized;
                        break;
                    case CHTTPReply::forbidden:
                        AString = forbidden;
                        break;
                    case CHTTPReply::not_found:
                        AString = not_found;
                        break;
                    case CHTTPReply::not_allowed:
                        AString = not_allowed;
                        break;
                    case CHTTPReply::internal_server_error:
                        AString = internal_server_error;
                        break;
                    case CHTTPReply::not_implemented:
                        AString = not_implemented;
                        break;
                    case CHTTPReply::bad_gateway:
                        AString = bad_gateway;
                        break;
                    case CHTTPReply::service_unavailable:
                        AString = service_unavailable;
                        break;
                    case CHTTPReply::gateway_timeout:
                        AString = gateway_timeout;
                        break;
                    default:
                        AString = internal_server_error;
                        break;
                }
            }
        } // namespace StatusStrings
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::ToBuffers(CMemoryStream &Stream) {

            StatusString = Status;
            StatusStrings::ToString(Status, StatusText);

            CString HTTP;
            HTTP.Format("HTTP/%d.%d %d %s", VMajor, VMinor, Status, StatusText.c_str());
            HTTP.SaveToStream(Stream);

            StringArrayToStream(Stream, MiscStrings::crlf);

            for (int i = 0; i < Headers.Count(); ++i) {
                const auto &H = Headers[i];
                H.Name().SaveToStream(Stream);
                StringArrayToStream(Stream, MiscStrings::separator);
                H.Value().SaveToStream(Stream);
                StringArrayToStream(Stream, MiscStrings::crlf);
            }

            StringArrayToStream(Stream, MiscStrings::crlf);
            Content.SaveToStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        #define CreateStockHtmlReplies(Code, Message) (                                 \
            "<html>\r\n"                                                                \
            "<head><title>" #Message "</title></head>\r\n"                              \
            "<body>\r\n"                                                                \
            "<center><h1>" #Code " " #Message "</h1></center>\r\n"                      \
            "<hr><center>" WWWServerName "</center>\r\n"                                \
            "</body>\r\n"                                                               \
            "</html>\r\n"                                                               \
        )                                                                               \
        //--------------------------------------------------------------------------------------------------------------

        #define CreateStockJsonReplies(Code, Message) (                                 \
            "{"                                                                         \
              "\"error\": {"                                                            \
                "\"code\":" #Code ","                                                   \
                "\"message\":\"" #Message "\""                                          \
              "}"                                                                       \
            "}"                                                                         \
        )                                                                               \
        //--------------------------------------------------------------------------------------------------------------

        #define CreateStockReplies(Code, Message) {                                     \
            _T(CreateStockHtmlReplies(Code, Message)),                                  \
            _T(CreateStockJsonReplies(Code, Message))                                   \
        }                                                                               \
        //--------------------------------------------------------------------------------------------------------------

        namespace StockReplies {

            LPCTSTR switching_protocols[]   = CreateStockReplies(101, Switching Protocols);
            LPCTSTR ok[]                    = CreateStockReplies(200, OK);
            LPCTSTR created[]               = CreateStockReplies(201, Created);
            LPCTSTR accepted[]              = CreateStockReplies(202, Accepted);
            LPCTSTR non_authoritative[]     = CreateStockReplies(202, Non - Authoritative Information);
            LPCTSTR no_content[]            = CreateStockReplies(204, No Content);
            LPCTSTR multiple_choices[]      = CreateStockReplies(300, Multiple Choices);
            LPCTSTR moved_permanently[]     = CreateStockReplies(301, Moved Permanently);
            LPCTSTR moved_temporarily[]     = CreateStockReplies(302, Moved Temporarily);
            LPCTSTR see_other[]             = CreateStockReplies(303, See Other);
            LPCTSTR not_modified[]          = CreateStockReplies(304, Not Modified);
            LPCTSTR bad_request[]           = CreateStockReplies(400, Bad Request);
            LPCTSTR unauthorized[]          = CreateStockReplies(401, Unauthorized);
            LPCTSTR forbidden[]             = CreateStockReplies(403, Forbidden);
            LPCTSTR not_found[]             = CreateStockReplies(404, Not Found);
            LPCTSTR not_allowed[]           = CreateStockReplies(405, Method Not Allowed);
            LPCTSTR internal_server_error[] = CreateStockReplies(500, Internal Server Error);
            LPCTSTR not_implemented[]       = CreateStockReplies(501, Not Implemented);
            LPCTSTR bad_gateway[]           = CreateStockReplies(502, Bad Gateway);
            LPCTSTR service_unavailable[]   = CreateStockReplies(503, Service Unavailable);
            LPCTSTR gateway_timeout[]       = CreateStockReplies(504, Gateway Timeout);

            LPCTSTR ToString(CHTTPReply::CStatusType AStatus, CHTTPReply::CContentType AMessage) {

                if (AMessage > CHTTPReply::CContentType::json)
                    AMessage = CHTTPReply::CContentType::html;

                switch (AStatus) {
                    case CHTTPReply::switching_protocols:
                        return switching_protocols[AMessage];
                    case CHTTPReply::ok:
                        return ok[AMessage];
                    case CHTTPReply::created:
                        return created[AMessage];
                    case CHTTPReply::accepted:
                        return accepted[AMessage];
                    case CHTTPReply::non_authoritative:
                        return non_authoritative[AMessage];
                    case CHTTPReply::no_content:
                        return no_content[AMessage];
                    case CHTTPReply::multiple_choices:
                        return multiple_choices[AMessage];
                    case CHTTPReply::moved_permanently:
                        return moved_permanently[AMessage];
                    case CHTTPReply::moved_temporarily:
                        return moved_temporarily[AMessage];
                    case CHTTPReply::see_other:
                        return see_other[AMessage];
                    case CHTTPReply::not_modified:
                        return not_modified[AMessage];
                    case CHTTPReply::bad_request:
                        return bad_request[AMessage];
                    case CHTTPReply::unauthorized:
                        return unauthorized[AMessage];
                    case CHTTPReply::forbidden:
                        return forbidden[AMessage];
                    case CHTTPReply::not_found:
                        return not_found[AMessage];
                    case CHTTPReply::not_allowed:
                        return not_allowed[AMessage];
                    case CHTTPReply::internal_server_error:
                        return internal_server_error[AMessage];
                    case CHTTPReply::not_implemented:
                        return not_implemented[AMessage];
                    case CHTTPReply::bad_gateway:
                        return bad_gateway[AMessage];
                    case CHTTPReply::service_unavailable:
                        return service_unavailable[AMessage];
                    case CHTTPReply::gateway_timeout:
                        return gateway_timeout[AMessage];
                    default:
                        return internal_server_error[AMessage];
                }
            }

        } // namespace stock_replies
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::Clear() {
            VMajor = 1;
            VMinor = 1;
            Status = CStatusType::internal_server_error;
            StatusString.Clear();
            StatusText.Clear();
            ContentType = CContentType::html;
            CloseConnection = false;
            Headers.Clear();
            Content.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue) {
            Headers.Add(CHeader(lpszName, lpszValue));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::AddHeader(LPCTSTR lpszName, const CString &Value) {
            Headers.Add(CHeader(lpszName, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::AddHeader(const CString &Name, const CString &Value) {
            Headers.Add(CHeader(Name, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::ToText() {
            CString Temp;
            TCHAR ch;
            if (!Content.IsEmpty()) {
                Temp = Content;
                Content.Clear();
                for (size_t i = 0; i < Temp.Size(); ++i) {
                    ch = Temp[i];
                    if (!IsCtl(ch) || (ch == '\t') || (ch == '\r') || (ch == '\n'))
                        Content.Append(ch);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::ToJSON() {
            CString Temp;
            if (!Content.IsEmpty()) {
                Temp = Content;
                Content.Clear();
                for (size_t i = 0; i < Temp.Size(); ++i) {
                    switch (Temp[i]) {
                        case 8:
                            Content.Append(92);
                            Content.Append('b');
                            break;
                        case '\n':
                            Content.Append(92);
                            Content.Append('n');
                            break;
                        case 12:
                            Content.Append(92);
                            Content.Append('f');
                            break;
                        case '\r':
                            Content.Append(92);
                            Content.Append('r');
                            break;
                        case '\t':
                            Content.Append(92);
                            Content.Append('t');
                            break;
                        default:
                            Content.Append(Temp[i]);
                            break;
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::StringToStatus() {
            int I = StrToInt(StatusString.c_str());
            for (const auto S : StatusArray) {
                if (I == S) {
                    Status = S;
                    break;
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCTSTR CHTTPReply::GetGMT(LPTSTR lpszBuffer, size_t Size, time_t Delta) {
            time_t timer = 0;
            struct tm *gmt;

            timer = time(&timer) + Delta;
            gmt = gmtime(&timer);

            if ((gmt != nullptr) && (strftime(lpszBuffer, Size, "%a, %d %b %Y %T %Z", gmt) != 0)) {
                return lpszBuffer;
            }

            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::SetCookie(LPCTSTR lpszName, LPCTSTR lpszValue, LPCTSTR lpszPath, time_t Expires,
                bool HttpOnly, LPCTSTR lpszSameSite) {

            TCHAR szDate[MAX_BUFFER_SIZE + 1] = {0};

            CString Cookie;

            Cookie = lpszName;
            Cookie << _T("=");
            Cookie << lpszValue;

            if (lpszPath != nullptr) {
                Cookie << "; Path=";
                Cookie << lpszPath;
            }

            if (Expires > 0) {
                Cookie << "; Expires=";
                Cookie << CHTTPReply::GetGMT(szDate, sizeof(szDate), Expires);
            } else if (Expires < 0) {
                Cookie << "; Max-Age=0";
            }

            if (HttpOnly)
                Cookie << "; HttpOnly";

            if (lpszSameSite != nullptr) {
                Cookie << "; SameSite=";
                Cookie << lpszSameSite;
            }

            AddHeader(_T("Set-Cookie"), Cookie);
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPReply *CHTTPReply::GetReply(CHTTPReply *AReply, CStatusType AStatus, LPCTSTR AContentType,
                LPCTSTR ATransferEncoding) {

            TCHAR szBuffer[MAX_STRING_LEN + 1] = {0};
            TCHAR szDate[MAX_BUFFER_SIZE + 1] = {0};
            TCHAR szSize[_INT_T_LEN + 1] = {0};

            AReply->VMajor = 1;
            AReply->VMinor = 1;

            AReply->Status = AStatus;

            AReply->AddHeader(_T("Server"), AReply->ServerName);

            if (GetGMT(szDate, sizeof(szDate)) != nullptr) {
                AReply->AddHeader(_T("Date"), szDate);
            }

            if (!AReply->Content.IsEmpty()) {
                AReply->AddHeader(_T("Accept-Ranges"), _T("bytes"));

                if (AContentType == nullptr) {
                    switch (AReply->ContentType) {
                        case CContentType::html:
                            AContentType = _T("text/html");
                            break;
                        case CContentType::json:
                            AContentType = _T("application/json");
                            AReply->ToJSON();
                            break;
                        case CContentType::xml:
                            AContentType = _T("application/xml");
                            AReply->ToText();
                            break;
                        case CContentType::text:
                            AContentType = _T("text/plain");
                            AReply->ToText();
                            break;
                        case CContentType::sbin:
                            AContentType = _T("application/octet-stream");
                            break;
                        default:
                            AContentType = _T("text/plain");
                            break;
                    }
                }

                AReply->AddHeader(_T("Content-Type"), AContentType);

                AReply->Content.Position(0);

                if (ATransferEncoding != nullptr && CompareString(ATransferEncoding, _T("chunked")) == 0) {
                    AReply->AddHeader(_T("Transfer-Encoding"), ATransferEncoding);

                    CString Buffer;
                    CString Length;

                    ssize_t count;
                    size_t pos = 0;

                    while (true) {
                        count = AReply->Content.Read(szBuffer, MAX_STRING_LEN);
                        Length = IntToStr((int) count, szSize, sizeof(szSize), 16);

                        Buffer.WriteBuffer(Length.Data(), Length.Size());
                        StringArrayToStream(Buffer, MiscStrings::crlf);

                        if (count == 0) {
                            StringArrayToStream(Buffer, MiscStrings::crlf);
                            break;
                        }

                        Buffer.WriteBuffer(szBuffer, count);
                        StringArrayToStream(Buffer, MiscStrings::crlf);
                    };

                    AReply->Content = Buffer;
                } else {
                    AReply->AddHeader(_T("Content-Length"), IntToStr((int) AReply->Content.Size(), szSize, sizeof(szSize)));
                }
            }

            switch (AStatus) {
                case not_allowed:
                case not_implemented:
                    AReply->AddHeader(_T("Allow"), AReply->AllowedMethods);
                    break;
                case service_unavailable:
                    AReply->AddHeader(_T("Retry-After"), _T("30"));
                    break;
                case no_content:
                    AReply->Content.Clear();
                    break;
                default:
                    break;
            }

            if (AReply->CloseConnection)
                AReply->AddHeader(_T("Connection"), _T("close"));
            else
                AReply->AddHeader(_T("Connection"), _T("keep-alive"));

            return AReply;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPReply *CHTTPReply::GetStockReply(CHTTPReply *AReply, CHTTPReply::CStatusType AStatus) {
            if (AStatus != CHTTPReply::no_content)
                AReply->Content = StockReplies::ToString(AStatus, AReply->ContentType);
            AReply = GetReply(AReply, AStatus);
            return AReply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::AddUnauthorized(CHTTPReply *AReply, bool ABearer, LPCTSTR AError, LPCTSTR AMessage) {
            const auto& LAuthenticate = AReply->Headers[_T("WWW-Authenticate")];
            if (LAuthenticate.IsEmpty()) {
                CString Basic(_T("Basic realm=\"Access denied\", charset=\"UTF-8\""));

                CString Bearer;
                Bearer.Format(_T("Bearer realm=\"Access denied\", error=\"%s\", error_description=\"%s\", charset=\"UTF-8\""), AError, AMessage);

                AReply->AddHeader(_T("WWW-Authenticate"), ABearer ? Bearer : Basic);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPReplyParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CHTTPReplyParser::Consume(CHTTPReply *AReply, CHTTPReplyContext& Context) {

            size_t ContentLength = 0;
            size_t ChunkedLength = 0;

            const auto bufferSize = Context.End - Context.Begin;
            const auto ch = (TCHAR) *Context.Begin++;

            switch (Context.State) {
                case Reply::http_version_h:
                    if (ch == 'H') {
                        Context.State = Reply::http_version_t_1;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_t_1:
                    if (ch == 'T') {
                        Context.State = Reply::http_version_t_2;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_t_2:
                    if (ch == 'T') {
                        Context.State = Reply::http_version_p;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_p:
                    if (ch == 'P') {
                        Context.State = Reply::http_version_slash;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_slash:
                    if (ch == '/') {
                        AReply->VMajor = 0;
                        AReply->VMinor = 0;
                        Context.State = Reply::http_version_major_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_major_start:
                    if (IsDigit(ch)) {
                        AReply->VMajor = AReply->VMajor * 10 + ch - '0';
                        Context.State = Reply::http_version_major;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_major:
                    if (ch == '.') {
                        Context.State = Reply::http_version_minor_start;
                        return -1;
                    } else if (IsDigit(ch)) {
                        AReply->VMajor = AReply->VMajor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_minor_start:
                    if (IsDigit(ch)) {
                        AReply->VMinor = AReply->VMinor * 10 + ch - '0';
                        Context.State = Reply::http_version_minor;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_minor:
                    if (ch == ' ') {
                        Context.State = Reply::http_status_start;
                        return -1;
                    } else if (IsDigit(ch)) {
                        AReply->VMinor = AReply->VMinor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status_start:
                    if (IsDigit(ch)) {
                        AReply->StatusString.Append(ch);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status:
                    if (ch == ' ') {
                        AReply->StringToStatus();
                        Context.State = Reply::http_status_text_start;
                        return -1;
                    } else if (IsDigit(ch)) {
                        AReply->StatusString.Append(ch);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status_text_start:
                    if (IsChar(ch)) {
                        AReply->StatusText.Append(ch);
                        Context.State = Reply::http_status_text;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status_text:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_1;
                        return -1;
                    } else if (IsChar(ch)) {
                        AReply->StatusText.Append(ch);
                        Context.State = Reply::http_status_text;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::expecting_newline_1:
                    if (ch == '\n') {
                        Context.State = Reply::header_line_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::header_line_start:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_3;
                        return -1;
                    } else if ((AReply->Headers.Count() > 0) && (ch == ' ' || ch == '\t')) {
                        Context.State = Reply::header_lws;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        AReply->Headers.Add(CHeader());
                        AReply->Headers.Last().Name().Append(ch);

                        Context.State = Reply::header_name;
                        return -1;
                    }
                case Reply::header_lws:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (ch == ' ' || ch == '\t') {
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Reply::header_value;
                        AReply->Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Reply::header_name:
                    if (ch == ':') {
                        Context.State = Reply::space_before_header_value;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        AReply->Headers.Last().Name().Append(ch);
                        return -1;
                    }
                case Reply::space_before_header_value:
                    if (ch == ' ') {
                        Context.State = Reply::header_value;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::header_value:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        AReply->Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Reply::header_value_options_start:
                    if ((ch == ' ' || ch == '\t')) {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Reply::header_value_options;
                        AReply->Headers.Last().Value().Append(ch);
                        AReply->Headers.Last().Data().Add(ch);
                        return -1;
                    }
                case Reply::header_value_options:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        AReply->Headers.Last().Value().Append(ch);
                        AReply->Headers.Last().Data().back().Append(ch);
                        return -1;
                    }
                case Reply::expecting_newline_2:
                    if (ch == '\n') {
                        Context.State = Reply::header_line_start;
                        return -1;
                    }

                    return 0;

                case Reply::expecting_newline_3:
                    if (ch == '\n') {
                        AReply->ContentLength = 0;
                        Context.ContentLength = bufferSize - 1;

                        if (AReply->Headers.Count() > 0) {
                            const auto& contentLength = AReply->Headers[_T("Content-Length")];
                            const auto& transferEncoding = AReply->Headers[_T("Transfer-Encoding")];

                            if (!contentLength.IsEmpty()) {
                                Context.ContentLength = strtoul(contentLength.c_str(), nullptr, 0);
                            } else if (transferEncoding == "chunked") {
                                Context.State = Reply::content_checking_length;
                                return -1;
                            }
                        }

                        if (Context.ContentLength > 0) {
                            Context.State = Reply::content;
                            return -1;
                        }

                        return 1;
                    }

                    return 0;

                case Reply::content:
                    if (Context.ContentLength == 0)
                        return 1;

                    ContentLength = Context.ContentLength > bufferSize ? bufferSize : Context.ContentLength;

                    AReply->Content.Append((LPCSTR) Context.Begin - 1, ContentLength);
                    AReply->ContentLength += ContentLength;

                    Context.Begin += ContentLength - 1;
                    Context.ContentLength -= ContentLength;

                    return Context.ContentLength == 0 ? 1 : -1;

                case Reply::content_checking_length:
                    if (IsHex(ch)) {
                        Context.Chunked.Append(ch);
                        return -1;
                    } else if (ch == '\r') {
                        if (Context.Chunked.IsEmpty())
                            return 0;
                        Context.ChunkedLength = StrToInt(Context.Chunked.c_str(), 16);
                        return -1;
                    } else if (ch == '\n') {
                        Context.State = Reply::content_checking_data;
                        return -1;
                    }

                    return 0;

                case Reply::content_checking_newline:
                    if (ch == '\r') {
                        Context.Chunked.Clear();
                        Context.ChunkedLength = 0;
                        return -1;
                    } else if (ch == '\n') {
                        Context.State = Reply::content_checking_length;
                        return -1;
                    }

                    return 0;

                case Reply::content_checking_data:
                    if (Context.ChunkedLength == 0)
                        return 1;

                    if (bufferSize > Context.ChunkedLength) {
                        ChunkedLength = Context.ChunkedLength;
                        Context.State = Reply::content_checking_newline;
                    } else {
                        ChunkedLength = bufferSize;
                        Context.State = Reply::content_checking_data;
                    }

                    AReply->Content.Append((LPCSTR) Context.Begin - 1, ChunkedLength);
                    AReply->ContentLength += ChunkedLength;

                    Context.Begin += ChunkedLength - 1;
                    Context.ChunkedLength -= ChunkedLength;

                    return -1;

                default:
                    return 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPReplyParser::Parse(CHTTPReply *AReply, CHTTPReplyContext& Context) {
            Context.Result = -1;
            while (Context.Result == -1 && Context.Begin != Context.End) {
                Context.Result = Consume(AReply, Context);
            }
            return Context.Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPReplyParser::IsChar(int c) {
            return c >= 0 && c <= 127;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPReplyParser::IsCtl(int c) {
            return (c >= 0 && c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPReplyParser::IsTSpecial(int c) {
            switch (c) {
                case '(':
                case ')':
                case '<':
                case '>':
                case '@':
                case ',':
                case ';':
                case ':':
                case '\\':
                case '"':
                case '/':
                case '[':
                case ']':
                case '?':
                case '=':
                case '{':
                case '}':
                case ' ':
                case '\t':
                    return true;
                default:
                    return false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPReplyParser::IsDigit(int c) {
            return c >= '0' && c <= '9';
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPReplyParser::IsHex(int c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServerConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPServerConnection::CHTTPServerConnection(CPollSocketServer *AServer):
                CTCPServerConnection(AServer) {

            m_State = Request::method_start;
            m_ContentLength = 0;

            m_Request = nullptr;
            m_Reply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPServerConnection::~CHTTPServerConnection() {
            CHTTPServerConnection::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPServerConnection::GetRequest() {
            if (m_Request == nullptr)
                m_Request = new CHTTPRequest();
            return m_Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPReply *CHTTPServerConnection::GetReply() {
            if (m_Reply == nullptr) {
                m_Reply = new CHTTPReply();
                m_Reply->ServerName = Server()->ServerName();
                m_Reply->AllowedMethods = Server()->AllowedMethods();
            }
            return m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::Clear() {
            CWebSocketConnection::Clear();

            m_State = Request::method_start;
            m_ContentLength = 0;

            FreeAndNil(m_Request);
            FreeAndNil(m_Reply);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::Parse(CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) {
            CHTTPContext Context = CHTTPContext((LPCBYTE) Stream.Memory(), Stream.Size(), m_State, m_ContentLength);
            const int ParseResult = CHTTPRequestParser::Parse(GetRequest(), Context);

            switch (ParseResult) {
                case 0:
                    m_ConnectionStatus = csRequestError;
                    break;

                case 1:
                    m_ConnectionStatus = csRequestOk;
                    DoRequest();
                    OnExecute(this);
                    break;

                default:
                    m_State = Context.State;

                    m_ContentLength = Context.ContentLength;

                    if (RecvBufferSize() < m_ContentLength)
                        RecvBufferSize(m_ContentLength);

                    m_ConnectionStatus = csWaitRequest;

                    break;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServerConnection::ParseInput(COnSocketExecuteEvent && OnExecute) {
            if (Connected()) {
                UpdateClock();
                CMemoryStream Stream(ReadAsync());
                if (Stream.Size() > 0) {
                    InputBuffer()->Extract(Stream.Memory(), Stream.Size());
                    switch (m_Protocol) {
                        case pHTTP:
                            CHTTPServerConnection::Parse(Stream, std::move(OnExecute));
                            break;
                        case pWebSocket:
                            CWebSocketConnection::Parse(Stream, std::move(OnExecute));
                            break;
                    }

                    return true;
                }
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendStockReply(CHTTPReply::CStatusType AStatus, bool ASendNow) {

            GetReply()->CloseConnection = CloseConnection();

            CHTTPReply::GetStockReply(m_Reply, AStatus);

            SendReply(ASendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(CHTTPReply::CStatusType AStatus, LPCTSTR AContentType, bool ASendNow) {

            if (AStatus == CHTTPReply::ok) {
                const CString &Value = GetRequest()->Headers[_T("Connection")].Lower();
                CloseConnection(!(Value == _T("keep-alive") || Value == _T("upgrade")));
            }

            GetReply()->CloseConnection = CloseConnection();

            CHTTPReply::GetReply(m_Reply, AStatus, AContentType);

            SendReply(ASendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(bool ASendNow) {

            GetReply()->ToBuffers(*OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            DoReply();

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SwitchingProtocols(const CString &Accept, const CString &Protocol) {

            RecvBufferSize(256 * 1024);

            CloseConnection(false);

            auto pReply = GetReply();

            pReply->Status = CHTTPReply::switching_protocols;

            pReply->AddHeader("Upgrade", "websocket");
            pReply->AddHeader("Connection", "Upgrade");

            if (!Accept.IsEmpty())
                pReply->AddHeader("Sec-WebSocket-Accept", base64_encode(Accept));

            if (!Protocol.IsEmpty())
                pReply->AddHeader("Sec-WebSocket-Protocol", Protocol);

            SendReply();

            m_Protocol = pWebSocket;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientConnection::CHTTPClientConnection(CPollSocketClient *AClient): CTCPClientConnection(AClient) {
            m_State = Reply::http_version_h;

            m_ContentLength = 0;
            m_ChunkedLength = 0;

            m_CloseConnection = true;

            m_Request = nullptr;
            m_Reply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientConnection::~CHTTPClientConnection() {
            CHTTPClientConnection::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::Clear() {
            CWebSocketConnection::Clear();

            m_State = Reply::http_version_h;
            m_ContentLength = 0;
            m_ChunkedLength = 0;

            FreeAndNil(m_Request);
            FreeAndNil(m_Reply);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::Parse(CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) {
                CHTTPReplyContext Context = CHTTPReplyContext((LPCBYTE) Stream.Memory(), Stream.Size(), m_State, m_ContentLength, m_ChunkedLength);

            const int ParseResult = CHTTPReplyParser::Parse(GetReply(), Context);

            switch (ParseResult) {
                case 0:
                    m_ConnectionStatus = csReplyError;
                    break;

                case 1:
                    m_ConnectionStatus = csReplyOk;
                    DoReply();
                    OnExecute(this);
                    break;

                default:
                    m_State = Context.State;

                    m_ContentLength = Context.ContentLength;
                    m_ChunkedLength = Context.ChunkedLength;

                    if (RecvBufferSize() < m_ContentLength)
                        RecvBufferSize(m_ContentLength);

                    m_ConnectionStatus = csWaitReply;

                    break;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPClientConnection::ParseInput(COnSocketExecuteEvent && OnExecute) {
            if (Connected()) {
                CMemoryStream Stream(ReadAsync());
                if (Stream.Size() > 0) {
                    InputBuffer()->Extract(Stream.Memory(), Stream.Size());
                    switch (m_Protocol) {
                        case pHTTP:
                            CHTTPClientConnection::Parse(Stream, std::move(OnExecute));
                            break;
                        case pWebSocket:
                            CWebSocketConnection::Parse(Stream, std::move(OnExecute));
                            break;
                    }

                    return true;
                }
            }

            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPClientConnection::GetRequest() {
            if (m_Request == nullptr) {
                m_Request = new CHTTPRequest();
                m_Request->Location.hostname = Client()->Host();
                m_Request->Location.port = Client()->Port();
                m_Request->UserAgent = Client()->ClientName();
            }
            return m_Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPReply *CHTTPClientConnection::GetReply() {
            if (m_Reply == nullptr) {
                m_Reply = new CHTTPReply;
            }
            return m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::SwitchingProtocols(CHTTPProtocol Protocol) {
            m_Protocol = Protocol;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::SendRequest(bool ASendNow) {

            GetRequest()->ToBuffers(*OutputBuffer());

            m_ConnectionStatus = csRequestReady;

            DoRequest();

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csRequestSent;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServer -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPServer::CHTTPServer(): CTCPAsyncServer() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPServer::CHTTPServer(const CString &IP, unsigned short Port): CHTTPServer() {
            DefaultIP() = IP;
            DefaultPort(Port);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::Assign(const CHTTPServer &Server) {
            if (&Server != this) {
                DefaultIP() = Server.DefaultIP();
                DefaultPort(Server.DefaultPort());

                ServerName() = Server.ServerName();
                AllowedMethods() = Server.AllowedMethods();

                IOHandler(Server.IOHandler());
                Bindings(Server.Bindings());

                m_Providers = Server.m_Providers;
                m_Sites = Server.m_Sites;

                m_ActiveLevel = Server.m_ActiveLevel;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::InitializeBindings() {
            CSocketHandle* pBinding = Bindings()->Add();
            for (int i = 0; i < m_Sites.Count(); ++i) {
                const auto& Site = m_Sites[i];
                const auto& Port = Site.Value()["listen"];
                if (Site.Name() == "*") {
                    if (!Port.IsEmpty())
                        pBinding->Port(Port.AsInteger());
                } else {
                    if (!Port.IsEmpty() && Port.AsInteger() != DefaultPort()) {
                        pBinding = Bindings()->Add();
                        pBinding->Port(Port.AsInteger());
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CHTTPServer::URLEncode(const CString &In) {
            const static TCHAR HexCodes[] = "0123456789ABCDEF";
            TCHAR ch;
            CString Out;
            for (int i = 0; i < In.Size(); i++) {
                ch = In.at(i);
                if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
                    Out += ch;
                } else {
                    Out += "%";
                    Out += HexCodes[(ch >> 4) & 0x0F];
                    Out += HexCodes[ch & 0x0F];
                }
            }
            return Out;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServer::URLDecode(const CString &In, CString &Out) {
            TCHAR ch;
            Out.Clear();
            for (size_t i = 0; i < In.size(); ++i) {
                ch = In.at(i);
                if (ch == '%') {
                    if (i + 3 <= In.size()) {
                        int value = (int) HexToDec(In.substr(i + 1, 2).c_str());
                        Out += static_cast<char>(value);
                        i += 2;
                    } else {
                        return false;
                    }
                } else {
                    Out += ch;
                }
            }
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CHTTPServer::URLDecode(const CString &In) {
            CString decodeURL;
            if (URLDecode(In, decodeURL))
                return decodeURL;
            return {};
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoTimeOut(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            chASSERT(pConnection);
            try {
                if (pConnection->Connected()) {
                    if (pConnection->Protocol() == pHTTP) {
                        if (pConnection->ConnectionStatus() == csRequestOk) {
                            pConnection->SendStockReply(CHTTPReply::gateway_timeout, true);
                            pConnection->CloseConnection(true);
                        }
                    }

                    if (pConnection->CloseConnection()) {
                        pConnection->Disconnect();
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoAccept(CPollEventHandler *AHandler) {
            CIOHandlerSocket *pIOHandler = nullptr;
            CPollEventHandler *pEventHandler = nullptr;
            CHTTPServerConnection *pConnection = nullptr;

            try {
                pIOHandler = (CIOHandlerSocket *) CServerIOHandler::Accept(AHandler->Socket(), SOCK_NONBLOCK);

                if (Assigned(pIOHandler)) {
                    pConnection = new CHTTPServerConnection(this);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    pConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
                    pConnection->OnReply([this](auto && Sender) { DoReply(Sender); });
#else
                    pConnection->OnDisconnected(std::bind(&CHTTPServer::DoDisconnected, this, _1));
                    pConnection->OnReply(std::bind(&CHTTPServer::DoReply, this, _1));
#endif
                    pConnection->IOHandler(pIOHandler);

                    pIOHandler->AfterAccept();

                    pEventHandler = m_pEventHandlers->Add(pIOHandler->Binding()->Handle());
                    pEventHandler->Binding(pConnection);
                    pEventHandler->Start(etServerIO);

                    DoConnected(pConnection);
                }
            } catch (Delphi::Exception::Exception &E) {
                delete pConnection;
                DoListenException(E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoRead(CPollEventHandler *AHandler) {

            auto OnExecuted = [this](CTCPConnection *AConnection) {
                return DoExecute(AConnection);
            };

            auto pConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            chASSERT(pConnection);
            try {
                pConnection->ParseInput(OnExecuted);
                if (pConnection->ConnectionStatus() == csRequestError) {
                    pConnection->CloseConnection(true);
                    if (pConnection->Protocol() == pHTTP)
                        pConnection->SendStockReply(CHTTPReply::bad_request);
                    pConnection->Clear();
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            chASSERT(pConnection);
            try {
                if (pConnection->WriteAsync()) {
                    if (pConnection->ConnectionStatus() == csReplyReady) {

                        pConnection->ConnectionStatus(csReplySent);
                        pConnection->Clear();

                        if (pConnection->CloseConnection()) {
                            pConnection->Disconnect();
                        }
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                pConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServer::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *pHandler;

            auto pConnection = dynamic_cast<CHTTPServerConnection *> (AConnection);
            chASSERT(pConnection);
            auto pRequest = pConnection->Request();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, pRequest->Method);
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers()->Count(); ++Index) {
                        pHandler = CommandHandlers()->Commands(Index);
                        if (pHandler->Enabled()) {
                            if (pHandler->Check(pRequest->Method, AConnection))
                                break;
                        }
                    }

                    if (Index == CommandHandlers()->Count())
                        DoNoCommandHandler(pRequest->Method, AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, E);
                }
                DoAfterCommandHandler(AConnection);
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServer::DoExecute(CTCPConnection *AConnection) {
            if (m_OnExecute != nullptr) {
                return m_OnExecute(AConnection);
            }
            return DoCommand(AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoReply(CObject *Sender) {
            DoAccessLog(dynamic_cast<CHTTPServerConnection *> (Sender));
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPClient::CHTTPClient(): CAsyncClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClient::CHTTPClient(const CString &Host, unsigned short Port): CAsyncClient(Host, Port) {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto pConnection = new CHTTPClientConnection(this);
            pConnection->IOHandler(AIOHandler);
            pConnection->AutoFree(true);
            AHandler->Binding(pConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoConnect(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

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
                    pConnection->OnDisconnected(std::bind(&CHTTPClient::DoDisconnected, this, _1));
#endif
                    DoConnected(pConnection);
                    DoRequest(pConnection);

                    AHandler->Start(etServerIO);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoRead(CPollEventHandler *AHandler) {

            auto OnExecuted = [this](CTCPConnection *AConnection) {
                return DoExecute(AConnection);
            };

            auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());
            try {
                if (pConnection->ParseInput(OnExecuted)) {
                    switch (pConnection->ConnectionStatus()) {
                        case csReplyError:
                            pConnection->Clear();
                            break;

                        case csReplyOk:
                            pConnection->Clear();

                            if (pConnection->CloseConnection()) {
                                pConnection->Disconnect();
                            }

                            break;

                        default:
                            break;
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoWrite(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());
            try {
                if (pConnection->WriteAsync()) {
                    if (pConnection->ConnectionStatus() == csRequestReady) {
                        pConnection->ConnectionStatus(csRequestSent);
                    }
                }
                if (pConnection->ClosedGracefully()) {
                    AHandler->Binding(nullptr);
                    if (pConnection->AutoFree())
                        delete pConnection;
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPClient::DoCommand(CTCPConnection *AConnection) {
            CCommandHandler *pHandler;

            auto pConnection = dynamic_cast<CHTTPServerConnection *> (AConnection);
            auto pRequest = pConnection->Request();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, pRequest->Method);
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers()->Count(); ++Index) {
                        pHandler = CommandHandlers()->Commands(Index);
                        if (pHandler->Enabled()) {
                            if (pHandler->Check(pRequest->Method, AConnection))
                                break;
                        }
                    }

                    if (Index == CommandHandlers()->Count())
                        DoNoCommandHandler(pRequest->Method, AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, E);
                }
                DoAfterCommandHandler(AConnection);
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPClient::DoExecute(CTCPConnection *AConnection) {
            if (m_OnExecute != nullptr) {
                return m_OnExecute(AConnection);
            }
            return DoCommand(AConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoRequest(CHTTPClientConnection *AConnection) {
            if (m_OnRequest != nullptr) {
                m_OnRequest(this, AConnection->Request());
                AConnection->SendRequest(true);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientItem -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientItem::CHTTPClientItem(CHTTPClientManager *AManager): CCollectionItem(AManager), CHTTPClient() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientItem::CHTTPClientItem(CHTTPClientManager *AManager, const CString &Host, unsigned short Port):
            CCollectionItem(AManager), CHTTPClient(Host, Port) {

        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientManager ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientItem *CHTTPClientManager::GetItem(int Index) const {
            return (CHTTPClientItem *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientItem *CHTTPClientManager::Add(const CString &Host, unsigned short Port) {
            return new CHTTPClientItem(this, Host, Port);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxy ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPProxy::CHTTPProxy(CHTTPProxyManager *AManager, CHTTPServerConnection *AConnection): CHTTPClientItem(AManager),
            CPollConnection(nullptr) {

            m_Request = nullptr;
            AConnection->Binding(this);

            m_pConnection = AConnection;
            m_ClientName = Server()->ServerName();

            AllocateEventHandlers(Server());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto pConnection = new CHTTPClientConnection(this);
            pConnection->IOHandler(AIOHandler);
            pConnection->AutoFree(true);
            AHandler->Binding(pConnection);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoConnect(CPollEventHandler *AHandler) {
            auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

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
                    pConnection->OnDisconnected(std::bind(&CHTTPProxy::DoDisconnected, this, _1));
#endif
                    DoConnected(pConnection);
                    DoRequest(pConnection);

                    AHandler->Start(etServerIO);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoRequest(CHTTPClientConnection *AConnection) {
            auto pRequest = AConnection->Request();
            *pRequest = *m_Request;
            AConnection->SendRequest(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPProxy::GetRequest() {
            if (m_Request == nullptr) {
                m_Request = new CHTTPRequest();
                m_Request->Location.hostname = Host();
                m_Request->Location.port = Port();
                m_Request->UserAgent = ClientName();
            }

            return m_Request;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxyManager -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPProxy *CHTTPProxyManager::GetItem(int Index) const {
            return (CHTTPProxy *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPProxy *CHTTPProxyManager::Add(CHTTPServerConnection *AConnection) {
            return new CHTTPProxy(this, AConnection);
        }
    }
}
}
