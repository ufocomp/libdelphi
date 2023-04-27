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
            for (int i = 0; i < GetCount(); ++i) {
                const CFormDataItem& Item = Get(i);
                if (Item.Name.Lower() == Name)
                    return i;
            }
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CFormData::IndexOfName(LPCTSTR Name) const {
            for (int i = 0; i < GetCount(); ++i) {
                const CFormDataItem& Item = Get(i);
                if (Item.Name.Lower() == Name)
                    return i;
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
                for (int i = LCount; i < NewCount; ++i)
                    Add(CFormDataItem());
            } else {
                for (int i = LCount - 1; i >= NewCount; --i)
                    Delete(i);
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
            for (int i = 0; i < Value.GetCount(); ++i) {
                Add(Value[i]);
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

        int CHTTPRequest::DelHeader(const CString &Name) {
            const auto index = Headers.IndexOfName(Name);
            if (index != -1 ) {
                Headers.Delete(index);
            }
            return index;
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

        CHTTPRequest *CHTTPRequest::Prepare(CHTTPRequest &Request, LPCTSTR AMethod, LPCTSTR AURI, LPCTSTR lpszContentType,
                LPCTSTR AConnection) {

            TCHAR szSize[_INT_T_LEN + 1] = {0};

            Request.VMajor = 1;
            Request.VMinor = 1;

            Request.Method = AMethod;
            Request.URI = AURI;

            Request.AddHeader(_T("Host"), Request.Location.Host());
            Request.AddHeader(_T("User-Agent"), Request.UserAgent);
            Request.AddHeader(_T("Accept"), _T("*/*"));

            if (!Request.Content.IsEmpty()) {
                Request.AddHeader(_T("Accept-Ranges"), _T("bytes"));

                if (lpszContentType == nullptr) {
                    switch (Request.ContentType) {
                        case CContentType::html:
                            lpszContentType = _T("text/html");
                            break;
                        case CContentType::json:
                            lpszContentType = _T("application/json");
                            Request.ToJSON();
                            break;
                        case CContentType::xml:
                            lpszContentType = _T("application/xml");
                            Request.ToText();
                            break;
                        case CContentType::text:
                            lpszContentType = _T("text/plain");
                            Request.ToText();
                            break;
                        case CContentType::sbin:
                            lpszContentType = _T("application/octet-stream");
                            break;
                        default:
                            lpszContentType = _T("text/plain");
                            break;
                    }
                }

                Request.AddHeader(_T("Content-Type"), lpszContentType);
                Request.AddHeader(_T("Content-Length"), IntToStr((int) Request.Content.Size(), szSize, sizeof(szSize)));
            }

            if (AConnection == nullptr) {
                if (Request.CloseConnection)
                    Request.AddHeader(_T("Connection"), _T("close"));
                else
                    Request.AddHeader(_T("Connection"), _T("keep-alive"));
            } else {
                Request.AddHeader(_T("Connection"), AConnection);
            }

            return &Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPRequest *CHTTPRequest::Authorization(CHTTPRequest &Request, LPCTSTR AMethod, LPCTSTR ALogin,
                LPCTSTR APassword) {

            CString sPassphrase, sAuthorization;

            sPassphrase = ALogin;
            sPassphrase << ":";
            sPassphrase << APassword;

            sAuthorization = AMethod;
            sAuthorization << " ";
            sAuthorization << base64_encode(sPassphrase);

            Request.AddHeader("Authorization", sAuthorization);

            return &Request;
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
            CStringList List;

            const auto& cookie = Headers[_T("Cookie")];
            if (!cookie.empty()) {
                SplitColumns(cookie, List, ';');
            }

            Cookies.Clear();
            for (int i = 0; i < List.Count(); i++) {
                Cookies.Add(List[i].Trim());
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPRequestParser ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::Consume(CHTTPRequest &Request, CHTTPContext& Context) {
            size_t ContentLength = 0;

            const auto BufferSize = Context.End - Context.Begin;
            const auto ch = (TCHAR) *Context.Begin++;

            switch (Context.State) {
                case Request::method_start:
                    Request.Clear();
                    if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::method;
                        Request.Method.Append(ch);
                        return -1;
                    }
                case Request::method:
                    if (ch == ' ') {
                        Context.State = Request::uri;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Request.Method.Append(ch);
                        return -1;
                    }
                case Request::uri_start:
                    if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::uri;
                        Request.URI.Append(ch);
                        return -1;
                    }
                case Request::uri:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '?') {
                        Request.URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.URI.Append(ch);
                        return -1;
                    }
                case Request::uri_param_start:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '&') {
                        Request.URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.URI.Append(ch);
                        Request.Params.Add(ch);
                        Context.State = Request::uri_param;
                        return -1;
                    }
                case Request::uri_param:
                    if (ch == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (ch == '&') {
                        Request.URI.Append(ch);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (ch == '#') {
                        Request.URI.Append(ch);
                        Context.State = Request::uri;
                        return -1;
                    } else if (ch == '%') {
                        Request.URI.Append(ch);
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::uri_param_mime;
                        return -1;
                    } else if (ch == '+') {
                        Request.URI.Append(ch);
                        Request.Params.back().Append(' ');
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.URI.Append(ch);
                        Request.Params.back().Append(ch);
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
                        Request.VMajor = 0;
                        Request.VMinor = 0;
                        Context.State = Request::http_version_major_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_major_start:
                    if (IsDigit(ch)) {
                        Request.VMajor = Request.VMajor * 10 + ch - '0';
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
                        Request.VMajor = Request.VMajor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_minor_start:
                    if (IsDigit(ch)) {
                        Request.VMinor = Request.VMinor * 10 + ch - '0';
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
                        Request.VMinor = Request.VMinor * 10 + ch - '0';
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
                    } else if ((Request.Headers.Count() > 0) && (ch == ' ' || ch == '\t')) {
                        Context.State = Request::header_lws;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Request.Headers.Add(CHeader());
                        Request.Headers.Last().Name().Append(ch);

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
                        Request.Headers.Last().Value().Append(ch);
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
                        Request.Headers.Last().Name().Append(ch);
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
                        Request.Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Request::header_value_options_start:
                    if ((ch == ' ' || ch == '\t')) {
                        Request.Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.Headers.Last().Value().Append(ch);
                        Request.Headers.Last().Data().Add(ch);

                        Context.State = Request::header_value_options;
                        return -1;
                    }
                case Request::header_value_options:
                    if (ch == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        Request.Headers.Last().Value().Append(ch);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.Headers.Last().Value().Append(ch);
                        Request.Headers.Last().Data().back().Append(ch);
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
                        Request.ContentLength = 0;
                        Context.ContentLength = BufferSize - 1;

                        if (Request.Headers.Count() > 0) {
                            if (!Request.BuildLocation())
                                return 0;

                            Request.BuildCookies();

                            const auto& contentLength = Request.Headers[_T("Content-Length")];
                            if (!contentLength.IsEmpty()) {
                                Context.ContentLength = strtoul(contentLength.c_str(), nullptr, 0);
                            }

                            const auto& contentType = Request.Headers[_T("Content-Type")];
                            if (contentType.Find("application/x-www-form-urlencoded") != CString::npos) {
                                Request.ContentLength = Context.ContentLength;
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

                    Request.Content.Append((LPCSTR) Context.Begin - 1, ContentLength);
                    Request.ContentLength += ContentLength;

                    Context.Begin += ContentLength - 1;
                    Context.ContentLength -= ContentLength;

                    return Context.ContentLength == 0 ? 1 : -1;

                case Request::form_data_start:
                    Request.Content.Append(ch);

                    if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Request::form_data;
                        Request.FormData.Add(ch);
                        return -1;
                    }
                case Request::form_data:
                    Request.Content.Append(ch);

                    if (ch == '\n') {
                        return 1;
                    } else if (ch == '\r') {
                        return -1;
                    } else if (ch == '&') {
                        Context.State = Request::form_data_start;
                        return -1;
                    } else if (ch == '+') {
                        Request.FormData.back().Append(' ');
                        return -1;
                    } else if (ch == '%') {
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::form_mime;
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Request.FormData.back().Append(ch);

                        if (Request.Content.Size() < Request.ContentLength) {
                            return -1;
                        }
                    }

                    return 1;
                case Request::uri_param_mime:
                    Request.URI.Append(ch);
                    Context.MIME[Context.MimeIndex++] = ch;
                    if (Context.MimeIndex == 2) {
                        Request.Params.back().Append((TCHAR) HexToDec(Context.MIME));
                        Context.State = Request::uri_param;
                    }
                    return -1;
                case Request::form_mime:
                    Request.Content.Append(ch);
                    Context.MIME[Context.MimeIndex++] = ch;
                    if (Context.MimeIndex == 2) {
                        Request.FormData.back().Append((TCHAR) HexToDec(Context.MIME));
                        Context.State = Request::form_data;
                    }
                    if (Context.Begin == Context.End)
                        return 1;
                    return -1;
                default:
                    return 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::Parse(CHTTPRequest &Request, CHTTPContext& Context) {
            Context.Result = -1;
            while ((Context.Result == -1) && (Context.Begin != Context.End)) {
                Context.Result = Consume(Request, Context);
            }
            return Context.Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPRequestParser::ParseFormData(const CHTTPRequest &Request, CFormData& FormData) {
            const auto &caContent = Request.Content;

            if (caContent.IsEmpty())
                return 0;

            try {
                const auto &contentType = Request.Headers.Pairs("content-type");
                if (contentType.Value().Find("multipart/form-data") == CString::npos)
                    return 0;

                const CString CRLF(MiscStrings::crlf);
                const auto& boundary = contentType.Data()["boundary"];

                CString Boundary(CRLF);

                if (boundary.IsEmpty()) {
                    int i = 0;
                    char ch = caContent.at(i++);
                    while (ch != 0 && ch != '\r') {
                        Boundary.Append(ch);
                        ch = caContent.at(i++);
                    }
                } else {
                    Boundary << "--";
                    Boundary << boundary;
                }

                CStringList Data;

                size_t DataBegin = Boundary.Size();
                size_t DataEnd = caContent.Find(Boundary, DataBegin);

                while (DataEnd != CString::npos) {
                    Data.Add(caContent.SubString(DataBegin, DataEnd - DataBegin));
                    DataBegin = DataEnd + Boundary.Size() + CRLF.Size();
                    DataEnd = caContent.Find(Boundary, DataBegin);
                }

                CHTTPRequest httpRequest;

                for (int i = 0; i < Data.Count(); i++) {
                    CHTTPContext Context = CHTTPContext((LPCBYTE) Data[i].Data(), Data[i].Size());
                    Context.State = Request::CParserState::header_line_start;
                    const int Result = Parse(httpRequest, Context);
                    if (Result == 1) {
                        FormData.Add(CFormDataItem());
                        auto &DataItem = FormData.Last();

                        const auto &contentDisposition = httpRequest.Headers.Pairs("content-disposition");

                        DataItem.Name = contentDisposition.Data()["name"];
                        DataItem.File = contentDisposition.Data()["filename"];
                        DataItem.Data = httpRequest.Content;

//                        if (DataItem.Data.Find('\n') == CString::npos) {
//                            Request.FormData.AddPair(DataItem.Name, DataItem.Data);
//                        }
                    }
                    httpRequest.Clear();
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

        int CWebSocketParser::Parse(CWebSocket &Request, const CMemoryStream &Stream) {
            return Request.LoadFromStream(Stream);
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
                CHTTPReply::status_443,
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
            const TCHAR status_443[] = _T("443");
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
                    case CHTTPReply::status_443:
                        return StringArrayToStream(Stream, status_443);
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
                    case CHTTPReply::status_443:
                        AString = status_443;
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
            LPCTSTR status_443[]            = CreateStockReplies(443, 443);
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
                    case CHTTPReply::status_443:
                        return status_443[AMessage];
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

        int CHTTPReply::DelHeader(const CString &Name) {
            const auto index = Headers.IndexOfName(Name);
            if (index != -1 ) {
                Headers.Delete(index);
            }
            return index;
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
            int i = StrToInt(StatusString.c_str());
            for (const auto S : StatusArray) {
                if (i == S) {
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
            Cookie << CHTTPServer::URLEncode(lpszValue);

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

        void CHTTPReply::InitReply(CHTTPReply &Reply, CStatusType Status, LPCTSTR lpszContentType,
                LPCTSTR lpszTransferEncoding) {

            TCHAR szBuffer[MAX_STRING_LEN + 1] = {0};
            TCHAR szDate[MAX_BUFFER_SIZE + 1] = {0};
            TCHAR szSize[_INT_T_LEN + 1] = {0};

            Reply.VMajor = 1;
            Reply.VMinor = 1;

            Reply.Status = Status;

            Reply.AddHeader(_T("Server"), Reply.ServerName);

            if (GetGMT(szDate, sizeof(szDate)) != nullptr) {
                Reply.AddHeader(_T("Date"), szDate);
            }

            if (!Reply.Content.IsEmpty()) {
                Reply.AddHeader(_T("Accept-Ranges"), _T("bytes"));

                if (lpszContentType == nullptr) {
                    switch (Reply.ContentType) {
                        case CContentType::html:
                            lpszContentType = _T("text/html");
                            break;
                        case CContentType::json:
                            lpszContentType = _T("application/json");
                            Reply.ToJSON();
                            break;
                        case CContentType::xml:
                            lpszContentType = _T("application/xml");
                            Reply.ToText();
                            break;
                        case CContentType::text:
                            lpszContentType = _T("text/plain");
                            Reply.ToText();
                            break;
                        case CContentType::sbin:
                            lpszContentType = _T("application/octet-stream");
                            break;
                        default:
                            lpszContentType = _T("text/plain");
                            break;
                    }
                }

                Reply.AddHeader(_T("Content-Type"), lpszContentType);

                Reply.Content.Position(0);

                if (lpszTransferEncoding != nullptr && CompareString(lpszTransferEncoding, _T("chunked")) == 0) {
                    Reply.AddHeader(_T("Transfer-Encoding"), lpszTransferEncoding);

                    CString Buffer;
                    CString Length;

                    ssize_t count;
                    size_t pos = 0;

                    while (true) {
                        count = (ssize_t) Reply.Content.Read(szBuffer, MAX_STRING_LEN);
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

                    Reply.Content = Buffer;
                } else {
                    Reply.AddHeader(_T("Content-Length"), IntToStr((int) Reply.Content.Size(), szSize, sizeof(szSize)));
                }
            }

            switch (Status) {
                case not_allowed:
                case not_implemented:
                    Reply.AddHeader(_T("Allow"), Reply.AllowedMethods);
                    break;
                case service_unavailable:
                    Reply.AddHeader(_T("Retry-After"), _T("30"));
                    break;
                case no_content:
                    Reply.Content.Clear();
                    break;
                default:
                    break;
            }

            if (Reply.CloseConnection)
                Reply.AddHeader(_T("Connection"), _T("close"));
            else
                Reply.AddHeader(_T("Connection"), _T("keep-alive"));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::InitStockReply(CHTTPReply &Reply, CHTTPReply::CStatusType Status) {
            if (Status != CHTTPReply::no_content)
                Reply.Content = StockReplies::ToString(Status, Reply.ContentType);
            InitReply(Reply, Status);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPReply::AddUnauthorized(CHTTPReply &Reply, bool bBearer, LPCTSTR lpszError, LPCTSTR lpszMessage) {
            const auto& caAuthenticate = Reply.Headers[_T("WWW-Authenticate")];
            if (caAuthenticate.IsEmpty()) {
                CString Basic(_T("Basic realm=\"Access denied\", charset=\"UTF-8\""));

                CString Bearer;
                Bearer.Format(_T("Bearer realm=\"Access denied\", error=\"%s\", error_description=\"%s\", charset=\"UTF-8\""), lpszError, lpszMessage);

                Reply.AddHeader(_T("WWW-Authenticate"), bBearer ? Bearer : Basic);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPReplyParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CHTTPReplyParser::Consume(CHTTPReply &Reply, CHTTPReplyContext &Context) {

            size_t ContentLength = 0;
            size_t ChunkedLength = 0;

            const auto bufferSize = Context.End - Context.Begin;
            const auto ch = (TCHAR) *Context.Begin++;

            switch (Context.State) {
                case Reply::http_version_h:
                    Reply.Clear();
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
                        Reply.VMajor = 0;
                        Reply.VMinor = 0;
                        Context.State = Reply::http_version_major_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_major_start:
                    if (IsDigit(ch)) {
                        Reply.VMajor = Reply.VMajor * 10 + ch - '0';
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
                        Reply.VMajor = Reply.VMajor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_version_minor_start:
                    if (IsDigit(ch)) {
                        Reply.VMinor = Reply.VMinor * 10 + ch - '0';
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
                        Reply.VMinor = Reply.VMinor * 10 + ch - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status_start:
                    if (IsDigit(ch)) {
                        Reply.StatusString.Append(ch);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status:
                    if (ch == ' ') {
                        Reply.StringToStatus();
                        Context.State = Reply::http_status_text_start;
                        return -1;
                    } else if (IsDigit(ch)) {
                        Reply.StatusString.Append(ch);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return 0;
                    }
                case Reply::http_status_text_start:
                    if (IsChar(ch)) {
                        Reply.StatusText.Append(ch);
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
                        Reply.StatusText.Append(ch);
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
                    } else if ((Reply.Headers.Count() > 0) && (ch == ' ' || ch == '\t')) {
                        Context.State = Reply::header_lws;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Reply.Headers.Add(CHeader());
                        Reply.Headers.Last().Name().Append(ch);

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
                        Reply.Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Reply::header_name:
                    if (ch == ':') {
                        Context.State = Reply::space_before_header_value;
                        return -1;
                    } else if (!IsChar(ch) || IsCtl(ch) || IsTSpecial(ch)) {
                        return 0;
                    } else {
                        Reply.Headers.Last().Name().Append(ch);
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
                        Reply.Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Reply.Headers.Last().Value().Append(ch);
                        return -1;
                    }
                case Reply::header_value_options_start:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if ((ch == ' ' || ch == '\t')) {
                        Context.State = Reply::header_value_options_start;
                        Reply.Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Context.State = Reply::header_value_options;
                        Reply.Headers.Last().Value().Append(ch);
                        Reply.Headers.Last().Data().Add(ch);
                        return -1;
                    }
                case Reply::header_value_options:
                    if (ch == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (ch == ';') {
                        Context.State = Reply::header_value_options_start;
                        Reply.Headers.Last().Value().Append(ch);
                        return -1;
                    } else if (IsCtl(ch)) {
                        return 0;
                    } else {
                        Reply.Headers.Last().Value().Append(ch);
                        Reply.Headers.Last().Data().back().Append(ch);
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
                        Reply.ContentLength = 0;
                        Context.ContentLength = bufferSize - 1;

                        if (Reply.Headers.Count() > 0) {
                            const auto& contentLength = Reply.Headers[_T("Content-Length")];
                            const auto& transferEncoding = Reply.Headers[_T("Transfer-Encoding")];

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

                    Reply.Content.Append((LPCSTR) Context.Begin - 1, ContentLength);
                    Reply.ContentLength += ContentLength;

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

                    Reply.Content.Append((LPCSTR) Context.Begin - 1, ChunkedLength);
                    Reply.ContentLength += ChunkedLength;

                    Context.Begin += ChunkedLength - 1;
                    Context.ChunkedLength -= ChunkedLength;

                    return -1;

                default:
                    return 0;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CHTTPReplyParser::Parse(CHTTPReply &Reply, CHTTPReplyContext &Context) {
            Context.Result = -1;
            while (Context.Result == -1 && Context.Begin != Context.End) {
                Context.Result = Consume(Reply, Context);
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

            m_TimeOut = 0;
            m_State = Request::method_start;
            m_ContentLength = 0;

            m_Reply.ServerName = AServer->ServerName();
            m_Reply.AllowedMethods = AServer->AllowedMethods();
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPServerConnection::~CHTTPServerConnection() {
            CHTTPServerConnection::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::Clear() {
            CWebSocketConnection::Clear();

            m_Request.Clear();
            m_Reply.Clear();

            m_State = Request::method_start;
            m_ContentLength = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::Parse(const CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) {
            CHTTPContext Context((LPCBYTE) Stream.Memory(), Stream.Size(), m_State, m_ContentLength);
            const int result = CHTTPRequestParser::Parse(m_Request, Context);

            switch (result) {
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
                    InputBuffer().Extract(Stream.Memory(), Stream.Size());
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

        void CHTTPServerConnection::SendStockReply(CHTTPReply::CStatusType Status, bool bSendNow) {
            m_Reply.CloseConnection = CloseConnection();
            CHTTPReply::InitStockReply(m_Reply, Status);
            SendReply(bSendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(CHTTPReply::CStatusType Status, LPCTSTR lpszContentType, bool bSendNow) {
            CHTTPReply::InitReply(m_Reply, Status, lpszContentType);
            SendReply(bSendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(bool bSendNow) {
            m_Reply.ToBuffers(OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            DoReply();

            if (bSendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SwitchingProtocols(const CString &Accept, const CString &Protocol) {
            RecvBufferSize(256 * 1024);

            CloseConnection(false);

            m_Reply.Status = CHTTPReply::switching_protocols;

            m_Reply.AddHeader("Upgrade", "websocket");
            m_Reply.AddHeader("Connection", "Upgrade");

            if (!Accept.IsEmpty())
                m_Reply.AddHeader("Sec-WebSocket-Accept", base64_encode(Accept));

            if (!Protocol.IsEmpty())
                m_Reply.AddHeader("Sec-WebSocket-Protocol", Protocol);

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

            m_Request.Location.hostname = AClient->Host();
            m_Request.Location.port = AClient->Port();
            m_Request.UserAgent = AClient->ClientName();
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientConnection::~CHTTPClientConnection() {
            CHTTPClientConnection::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::Clear() {
            CWebSocketConnection::Clear();

            m_Request.Clear();
            m_Reply.Clear();

            m_State = Reply::http_version_h;
            m_ContentLength = 0;
            m_ChunkedLength = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::Parse(const CMemoryStream &Stream, COnSocketExecuteEvent && OnExecute) {
            CHTTPReplyContext Context((LPCBYTE) Stream.Memory(), Stream.Size(), m_State, m_ContentLength, m_ChunkedLength);

            const int ParseResult = CHTTPReplyParser::Parse(m_Reply, Context);

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
                    InputBuffer().Extract(Stream.Memory(), Stream.Size());
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

        const CHTTPRequest *CHTTPClientConnection::ptrRequest() const {
            return &m_Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CHTTPReply *CHTTPClientConnection::ptrReply() const {
            return &m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::SwitchingProtocols(CHTTPProtocol Protocol) {
            m_Protocol = Protocol;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::SendRequest(bool bSendNow) {
            m_Request.ToBuffers(OutputBuffer());

            m_ConnectionStatus = csRequestReady;

            DoRequest();

            if (bSendNow) {
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
                } else {
                    pConnection->Disconnect();
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
                    pEventHandler->Start(etIO);

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
            const auto &caRequest = pConnection->Request();

            bool Result = CommandHandlers().Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, caRequest.Method);
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers().Count(); ++Index) {
                        pHandler = CommandHandlers().Commands(Index);
                        if (pHandler->Enabled()) {
                            if (pHandler->Check(caRequest.Method, AConnection))
                                break;
                        }
                    }

                    if (Index == CommandHandlers().Count())
                        DoNoCommandHandler(caRequest.Method, AConnection);
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

        CHTTPClient::CHTTPClient(): CHTTPClient("localhost", 80) {

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

                    AHandler->Start(etIO);
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

            if (pConnection == nullptr) {
                AHandler->Stop();
                return;
            }

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

            if (pConnection == nullptr) {
                AHandler->Stop();
                return;
            }

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
            const auto &caRequest = pConnection->Request();

            const bool bResult = CommandHandlers().Count() > 0;

            if (bResult) {
                DoBeforeCommandHandler(AConnection, caRequest.Method);
                try {
                    int Index;
                    for (Index = 0; Index < CommandHandlers().Count(); ++Index) {
                        pHandler = CommandHandlers().Commands(Index);
                        if (pHandler->Enabled()) {
                            if (pHandler->Check(caRequest.Method, AConnection))
                                break;
                        }
                    }

                    if (Index == CommandHandlers().Count())
                        DoNoCommandHandler(caRequest.Method, AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, E);
                }
                DoAfterCommandHandler(AConnection);
            }

            return bResult;
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

        CHTTPProxy::CHTTPProxy(CHTTPProxyManager *AManager, CHTTPServerConnection *AConnection): CHTTPClientItem(AManager) {
            m_ProxyType = ptHTTP;
            m_pProxyConnection = nullptr;
            m_pConnection = AConnection;
            m_ClientName = Server()->ServerName();

            m_Request.Location.hostname = Host();
            m_Request.Location.port = Port();
            m_Request.UserAgent = ClientName();

            AllocateEventHandlers(Server());
        }
        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_SSL
        void CHTTPProxy::SetUsedSSL(bool Value) {
            CAsyncClient::SetUsedSSL(Value);
            if (m_pProxyConnection != nullptr) {
                auto pIOHandler = dynamic_cast<CIOHandlerSocket *> (m_pProxyConnection->IOHandler());
                auto pBinding = pIOHandler->Binding();
                pBinding->SSLMethod(Value ? sslClient : sslNotUsed);
                if (Value) {
                    pBinding->AllocateSSL();
                    pBinding->ConnectSSL();
#if (OPENSSL_VERSION_NUMBER >= 0x30000000L)
                    pBinding->SetOptionsSSL(SSL_OP_IGNORE_UNEXPECTED_EOF);
#endif
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------
#endif
        void CHTTPProxy::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            m_pProxyConnection = new CHTTPClientConnection(this);
            m_pProxyConnection->IOHandler(AIOHandler);
            m_pProxyConnection->AutoFree(true);
            AHandler->Binding(m_pProxyConnection);
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
                    DoHandshake(pConnection);

                    AHandler->Start(etIO);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(pConnection, E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoRead(CPollEventHandler *AHandler) {
            if (m_ProxyType == ptHTTP) {
                CHTTPClient::DoRead(AHandler);
            } else {
                auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

                if (pConnection == nullptr) {
                    AHandler->Stop();
                    return;
                }

                CMemoryStream Stream(pConnection->ReadAsync());
                if (Stream.Size() > 0) {
                    pConnection->InputBuffer().Extract(Stream.Memory(), Stream.Size());
                    unsigned char frame[2];
                    Stream.ReadBuffer(&frame, sizeof(frame));
                    if (frame[0] == 0x05 && (frame[1] == 0x00 || frame[1] == 0x06)) {
                        if (Stream.Size() == 2) {
                            SOCKS5(pConnection);
                        } else {
                            m_ProxyType = ptHTTP;
#ifdef WITH_SSL
                            UsedSSL(m_Request.Location.protocol == HTTPS_PREFIX);
#endif
                            DoRequest(pConnection);
                        }
                    } else {
                        throw ExceptionFrm(_T("SOCKS5 error: %x"), frame[1]);
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoWrite(CPollEventHandler *AHandler) {
            if (m_ProxyType == ptHTTP) {
                CHTTPClient::DoWrite(AHandler);
            } else {
                auto pConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

                if (pConnection == nullptr) {
                    AHandler->Stop();
                    return;
                }

                try {
                    if (pConnection->WriteAsync()) {
                        DoRead(AHandler);
                    }
                } catch (Delphi::Exception::Exception &E) {
                    DoException(pConnection, E);
                    AHandler->Stop();
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::Auth(CHTTPClientConnection *AConnection) {
            unsigned char frame[3] = { 0x05, 0x01, 0x00 };
            AConnection->WriteBuffer(&frame, sizeof(frame), true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::SOCKS5(CHTTPClientConnection *AConnection) {
            auto &Buffer = AConnection->OutputBuffer();

            union {
                uint16_t val;
                uint8_t  arr[2];
            } len16 = {0};

            unsigned char frame[4] = { 0x05, 0x01, 0x00, 0x03 };

            Buffer.WriteBuffer(&frame, sizeof(frame));

            frame[0] = m_Request.Location.hostname.Size();
            Buffer.WriteBuffer(&frame, 1);
            Buffer.WriteBuffer(m_Request.Location.hostname.Data(), m_Request.Location.hostname.Size());

            len16.val = be16toh(m_Request.Location.port);
            Buffer.WriteBuffer(len16.arr, sizeof(len16));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoHandshake(CHTTPClientConnection *AConnection) {
            if (m_ProxyType == ptSOCKS5) {
                Auth(AConnection);
            } else {
                DoRequest(AConnection);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoRequest(CHTTPClientConnection *AConnection) {
            auto &Request = AConnection->Request();
            Request = m_Request;
            AConnection->SendRequest(true);
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
