/*++

Library name:

  libdelphi

Module Name:

  Server.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/HTTP.hpp"

extern "C++" {

namespace Delphi {

    namespace HTTP {

        #define StringArrayToStream(Stream, Buf) (Stream)->Write((Buf), sizeof((Buf)) - sizeof(TCHAR))

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

        //-- CRequest --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        void CRequest::ToBuffers(CMemoryStream *AStream) {

            Method.SaveToStream(AStream);
            StringArrayToStream(AStream, MiscStrings::space);

            URI.SaveToStream(AStream);
            for (int i = 0; i < Params.Count(); ++i) {
                if (i == 0) {
                    StringArrayToStream(AStream, MiscStrings::question);
                } else {
                    StringArrayToStream(AStream, MiscStrings::ampersand);
                }
                Params[i].SaveToStream(AStream);
            }
            StringArrayToStream(AStream, MiscStrings::space);

            StringArrayToStream(AStream, MiscStrings::http);
            StringArrayToStream(AStream, MiscStrings::crlf);

            for (int i = 0; i < Headers.Count(); ++i) {
                CHeader &H = Headers[i];
                H.Name().SaveToStream(AStream);
                StringArrayToStream(AStream, MiscStrings::separator);
                H.Value().SaveToStream(AStream);
                StringArrayToStream(AStream, MiscStrings::crlf);
            }

            StringArrayToStream(AStream, MiscStrings::crlf);
            Content.SaveToStream(AStream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::Clear() {
            Method = "GET";
            URI = "/";
            VMajor = 1;
            VMinor = 1;
            Params.Clear();
            Headers.Clear();
            Content.Clear();
            ContentLength = 0;
            Location.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue) {
            Headers.AddPair(lpszName, lpszValue);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::AddHeader(LPCTSTR lpszName, const CString &Value) {
            Headers.AddPair(lpszName, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::AddHeader(const CString &Name, const CString &Value) {
            Headers.AddPair(Name, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::ToText() {
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

        void CRequest::ToJSON() {
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

        CRequest *CRequest::Prepare(CRequest *ARequest, LPCTSTR AMethod, LPCTSTR AURI, LPCTSTR AContentType) {

            TCHAR szSize[_INT_T_LEN + 1] = {0};

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

            if (ARequest->CloseConnection)
                ARequest->AddHeader(_T("Connection"), _T("close"));
            else
                ARequest->AddHeader(_T("Connection"), _T("keep-alive"));

            return ARequest;
        }
        //--------------------------------------------------------------------------------------------------------------

        CRequest *CRequest::Authorization(CRequest *ARequest, LPCTSTR AMethod, LPCTSTR ALogin,
                LPCTSTR APassword) {

            CString LPassphrase, LAuthorization;

            LPassphrase = ALogin;
            LPassphrase << ":";
            LPassphrase << APassword;

            LAuthorization = AMethod;
            LAuthorization << " ";
            LAuthorization << base64_encode(LPassphrase);

            ARequest->AddHeader("Authorization", LAuthorization);

            return ARequest;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CRequest::BuildLocation() {
            struct servent *sptr;
            const auto &Host = Headers.Values(_T("Host"));
            CString decodeURI;
            if (!CHTTPServer::URLDecode(URI, decodeURI))
                return false;
            Location = Host + decodeURI;
            if ((sptr = getservbyport(Location.port, "tcp")) != nullptr) {
                Location.protocol = sptr->s_name;
            } else {
                Location.protocol = HTTP_PREFIX;
            }
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CRequest::BuildCookies() {
            const auto& Cookie = Headers.Values(_T("Cookie"));
            if (!Cookie.empty()) {
                SplitColumns(Cookie, Cookies, ';');
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocket ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CWebSocket::CWebSocket() {
            m_Size = 0;
            m_Payload = new CMemoryStream();
        }
        //--------------------------------------------------------------------------------------------------------------

        CWebSocket::~CWebSocket() {
            FreeAndNil(m_Payload);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Clear() {
            m_Size = 0;
            m_Frame.Clear();
            m_Payload->Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Close(CMemoryStream *Stream) {
            const unsigned char Close[2] = { WS_FIN | WS_OPCODE_CLOSE, 0x00 };
            if (m_Frame.Length == 0) {
                Stream->Write(Close, sizeof(Close));
            } else {
                Stream->Write(Close, 1);
                PayloadToStream(Stream);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Ping(CMemoryStream *Stream) {
            const unsigned char Ping[2] = { WS_FIN | WS_OPCODE_PING, 0x00 };
            if (m_Frame.Length == 0) {
                Stream->Write(Ping, sizeof(Ping));
            } else {
                Stream->Write(Ping, 1);
                PayloadToStream(Stream);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Pong(CMemoryStream *Stream) {
            const unsigned char Pong[2] = { WS_FIN | WS_OPCODE_PONG, 0x00 };
            if (m_Frame.Length == 0) {
                Stream->Write(Pong, sizeof(Pong));
            } else {
                Stream->Write(Pong, 1);
                PayloadToStream(Stream);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Encode(CMemoryStream *Stream) {
            unsigned char Input;
            m_Payload->Position(0);
            for (size_t i = 0; i < m_Payload->Size(); i++) {
                m_Payload->Read(&Input, 1);
                Input = Input ^ m_Frame.MaskingKey[i % 4];
                Stream->Write(&Input, 1);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::Decode(CMemoryStream *Stream) {
            unsigned char Input;
            auto Position = Stream->Position();
            m_Payload->Position(m_Size);
            for (size_t i = Position; i < Stream->Size(); i++) {
                Stream->Read(&Input, 1);
                Input = Input ^ m_Frame.MaskingKey[(i - Position) % 4];
                m_Payload->Write(&Input, 1);
                m_Size++;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::PayloadFromStream(CMemoryStream *Stream) {
            if (m_Frame.Mask == WS_MASK) {
                Decode(Stream);
            } else {
                auto PayloadSize = Stream->Size() - Stream->Position();
                if (PayloadSize != 0) {
                    auto Position = m_Size;
                    m_Size += PayloadSize;
                    if (m_Size > m_Payload->Size())
                        m_Payload->Size(m_Size);
                    const auto Count = Stream->Read(Pointer((size_t) m_Payload->Memory() + Position), PayloadSize);
                    m_Payload->Position(Position + Count);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::PayloadToStream(CMemoryStream *Stream) {
            const unsigned char octet = m_Frame.Mask | m_Frame.Length;
            Stream->Write(&octet, sizeof(octet));

            if (m_Frame.Length == WS_PAYLOAD_LENGTH_16) {
                unsigned short Length16 = m_Payload->Size();
                Length16 = be16toh(Length16);
                Stream->Write(&Length16, sizeof(Length16));
            } else if (m_Frame.Length == WS_PAYLOAD_LENGTH_63) {
                unsigned long long Length63 = m_Payload->Size();
                Length63 = be64toh(Length63);
                Stream->Write(&Length63, sizeof(Length63));
            }

            if (m_Frame.Mask == WS_MASK) {
                Stream->Write(m_Frame.MaskingKey, sizeof(m_Frame.MaskingKey));
                Encode(Stream);
            } else {
                m_Payload->SaveToStream(Stream);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::SaveToStream(CMemoryStream *Stream) {
            const unsigned char octet = m_Frame.FIN | m_Frame.Opcode;
            Stream->Write(&octet, sizeof(octet));
            if (m_Frame.Length > 0) {
                PayloadToStream(Stream);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::LoadFromStream(CMemoryStream *Stream) {
            if (Stream->Size() < 2)
                return;

            size_t PayloadSize = 0;

            unsigned char octet[2] = {0, 0};
            Stream->Read(octet, sizeof(octet));

            m_Frame.FIN = octet[0] & WS_FIN;
            m_Frame.Opcode = octet[0] & 0x0Fu;

            m_Frame.Mask = octet[1] & WS_MASK;
            m_Frame.Length = octet[1] & 0x7Fu;

            if (m_Frame.Length == WS_PAYLOAD_LENGTH_16) {
                unsigned short Length16 = 0;
                Stream->Read(&Length16, sizeof(Length16));
                PayloadSize = htobe16(Length16);
            } else  if (m_Frame.Length == WS_PAYLOAD_LENGTH_63) {
                unsigned long long Length63 = 0;
                Stream->Read(&Length63, sizeof(Length63));
                PayloadSize = htobe64(Length63);
            } else {
                PayloadSize = m_Frame.Length;
            }

            if (m_Frame.Mask == WS_MASK) {
                Stream->Read(m_Frame.MaskingKey, sizeof(m_Frame.MaskingKey));
            }

            m_Payload->SetSize(PayloadSize);
            PayloadFromStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::SetPayload(CMemoryStream *Stream) {
            m_Frame.FIN = WS_FIN;
            m_Frame.Opcode = WS_OPCODE_BINARY;

            if (Stream->Size() < WS_PAYLOAD_LENGTH_16) {
                m_Frame.Length = Stream->Size();
            } else if (Stream->Size() <= 0xFFFF) {
                m_Frame.Length = WS_PAYLOAD_LENGTH_16;
            } else {
                m_Frame.Length = WS_PAYLOAD_LENGTH_63;
            }

            m_Payload->LoadFromStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CWebSocket::SetPayload(const CString &String) {
            m_Frame.FIN = WS_FIN;
            m_Frame.Opcode = WS_OPCODE_TEXT;

            if (String.Size() < WS_PAYLOAD_LENGTH_16) {
                m_Frame.Length = String.Size();
            } else if (String.Size() <= 0xFFFF) {
                m_Frame.Length = WS_PAYLOAD_LENGTH_16;
            } else {
                m_Frame.Length = WS_PAYLOAD_LENGTH_63;
            }

            m_Payload->Position(0);
            m_Payload->SetSize(String.Size());

            String.SaveToStream(m_Payload);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CRequestParser --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CRequestParser::Consume(CRequest *ARequest, CHTTPContext& Context) {
            char AInput = *Context.Begin++;
            switch (Context.State) {
                case Request::method_start:
                    if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return 0;
                    } else {
                        Context.State = Request::method;
                        ARequest->Method.Append(AInput);
                        return -1;
                    }
                case Request::method:
                    if (AInput == ' ') {
                        Context.State = Request::uri;
                        return -1;
                    } else if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return 0;
                    } else {
                        ARequest->Method.Append(AInput);
                        return -1;
                    }
                case Request::uri_start:
                    if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        Context.State = Request::uri;
                        ARequest->URI.Append(AInput);
                        return -1;
                    }
                case Request::uri:
                    if (AInput == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (AInput == '?') {
                        ARequest->URI.Append(AInput);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(AInput);
                        return -1;
                    }
                case Request::uri_param_start:
                    if (AInput == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(AInput);
                        ARequest->Params.Add(AInput);
                        Context.State = Request::uri_param;
                        return -1;
                    }
                case Request::uri_param:
                    if (AInput == ' ') {
                        Context.State = Request::http_version_h;
                        return -1;
                    } else if (AInput == '&') {
                        ARequest->URI.Append(AInput);
                        Context.State = Request::uri_param_start;
                        return -1;
                    } else if (AInput == '#') {
                        ARequest->URI.Append(AInput);
                        Context.State = Request::uri;
                        return -1;
                    } else if (AInput == '%') {
                        ARequest->URI.Append(AInput);
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::uri_param_mime;
                        return -1;
                    } else if (AInput == '+') {
                        ARequest->URI.Append(AInput);
                        ARequest->Params.back().Append(' ');
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->URI.Append(AInput);
                        ARequest->Params.back().Append(AInput);
                        return -1;
                    }
                case Request::http_version_h:
                    if (AInput == 'H') {
                        Context.State = Request::http_version_t_1;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_t_1:
                    if (AInput == 'T') {
                        Context.State = Request::http_version_t_2;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_t_2:
                    if (AInput == 'T') {
                        Context.State = Request::http_version_p;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_p:
                    if (AInput == 'P') {
                        Context.State = Request::http_version_slash;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_slash:
                    if (AInput == '/') {
                        ARequest->VMajor = 0;
                        ARequest->VMinor = 0;
                        Context.State = Request::http_version_major_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_major_start:
                    if (IsDigit(AInput)) {
                        ARequest->VMajor = ARequest->VMajor * 10 + AInput - '0';
                        Context.State = Request::http_version_major;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_major:
                    if (AInput == '.') {
                        Context.State = Request::http_version_minor_start;
                        return -1;
                    } else if (IsDigit(AInput)) {
                        ARequest->VMajor = ARequest->VMajor * 10 + AInput - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_minor_start:
                    if (IsDigit(AInput)) {
                        ARequest->VMinor = ARequest->VMinor * 10 + AInput - '0';
                        Context.State = Request::http_version_minor;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::http_version_minor:
                    if (AInput == '\r') {
                        Context.State = Request::expecting_newline_1;
                        return -1;
                    } else if (IsDigit(AInput)) {
                        ARequest->VMinor = ARequest->VMinor * 10 + AInput - '0';
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::expecting_newline_1:
                    if (AInput == '\n') {
                        Context.State = Request::header_line_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::header_line_start:
                    if (AInput == '\r') {
                        Context.State = Request::expecting_newline_3;
                        return -1;
                    } else if ((ARequest->Headers.Count() > 0) && (AInput == ' ' || AInput == '\t')) {
                        Context.State = Request::header_lws;
                        return -1;
                    } else if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Add(CHeader());
                        ARequest->Headers.Last().Name().Append(AInput);

                        Context.State = Request::header_name;
                        return -1;
                    }
                case Request::header_lws:
                    if (AInput == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (AInput == ' ' || AInput == '\t') {
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(AInput);
                        Context.State = Request::header_value;
                        return -1;
                    }
                case Request::header_name:
                    if (AInput == ':') {
                        Context.State = Request::space_before_header_value;
                        return -1;
                    } else if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Name().Append(AInput);
                        return -1;
                    }
                case Request::space_before_header_value:
                    if (AInput == ' ') {
                        Context.State = Request::header_value;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::header_value:
                    if (AInput == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (AInput == ';') {
                        ARequest->Headers.Last().Value().Append(AInput);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(AInput);
                        return -1;
                    }
                case Request::header_value_options_start:
                    if ((AInput == ' ' || AInput == '\t')) {
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(AInput);
                        ARequest->Headers.Last().Data().Add(AInput);

                        Context.State = Request::header_value_options;
                        return -1;
                    }
                case Request::header_value_options:
                    if (AInput == '\r') {
                        Context.State = Request::expecting_newline_2;
                        return -1;
                    } else if (AInput == ';') {
                        ARequest->Headers.Last().Value().Append(AInput);
                        Context.State = Request::header_value_options_start;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->Headers.Last().Value().Append(AInput);
                        ARequest->Headers.Last().Data().back().Append(AInput);
                        return -1;
                    }
                case Request::expecting_newline_2:
                    if (AInput == '\n') {
                        Context.State = Request::header_line_start;
                        return -1;
                    } else {
                        return 0;
                    }
                case Request::expecting_newline_3:
                    if (AInput == '\n') {
                        Context.ContentLength = Context.End - Context.Begin;

                        if (ARequest->Headers.Count() > 0) {
                            if (!ARequest->BuildLocation())
                                return 0;
                            ARequest->BuildCookies();

                            const auto& contentLength = ARequest->Headers.Values(_T("Content-Length"));
                            if (!contentLength.IsEmpty()) {
                                ARequest->ContentLength = strtoul(contentLength.c_str(), nullptr, 0);
                            } else {
                                ARequest->ContentLength = Context.ContentLength;
                            }

                            const auto& contentType = ARequest->Headers.Values(_T("Content-Type"));
                            if (contentType.Find("application/x-www-form-urlencoded") != CString::npos) {
                                Context.State = Request::form_data_start;
                                return -1;
                            }
                        } else {
                            ARequest->ContentLength = Context.ContentLength;
                        }

                        if (ARequest->ContentLength > 0) {
                            Context.State = Request::content;
                            return -1;
                        }

                        return 1;
                    } else {
                        return 0;
                    }
                case Request::content:
                    ARequest->Content.Append(AInput);

                    if (ARequest->Content.Size() < ARequest->ContentLength) {
                        return -1;
                    }

                    return 1;
                case Request::form_data_start:
                    ARequest->Content.Append(AInput);

                    if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        Context.State = Request::form_data;
                        ARequest->FormData.Add(AInput);
                        return -1;
                    }
                case Request::form_data:
                    ARequest->Content.Append(AInput);

                    if (AInput == '\n') {
                        return 1;
                    } else if (AInput == '\r') {
                        return -1;
                    } else if (AInput == '&') {
                        Context.State = Request::form_data_start;
                        return -1;
                    } else if (AInput == '+') {
                        ARequest->FormData.back().Append(' ');
                        return -1;
                    } else if (AInput == '%') {
                        Context.MimeIndex = 0;
                        ::SecureZeroMemory(Context.MIME, sizeof(Context.MIME));
                        Context.State = Request::form_mime;
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return 0;
                    } else {
                        ARequest->FormData.back().Append(AInput);

                        if (ARequest->Content.Size() < ARequest->ContentLength) {
                            return -1;
                        }
                    }

                    return 1;
                case Request::uri_param_mime:
                    ARequest->URI.Append(AInput);
                    Context.MIME[Context.MimeIndex++] = AInput;
                    if (Context.MimeIndex == 2) {
                        ARequest->Params.back().Append((TCHAR) HexToDec(Context.MIME));
                        Context.State = Request::uri_param;
                    }
                    return -1;
                case Request::form_mime:
                    ARequest->Content.Append(AInput);
                    Context.MIME[Context.MimeIndex++] = AInput;
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

        int CRequestParser::Parse(CRequest *ARequest, CHTTPContext& Context) {
            Context.Result = -1;
            while ((Context.Result == -1) && (Context.Begin != Context.End)) {
                Context.Result = Consume(ARequest, Context);
            }
            return Context.Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CRequestParser::ParseFormData(CRequest *ARequest, CFormData& FormData) {
            const CString& Content = ARequest->Content;

            if (Content.IsEmpty())
                return 0;

            try {
                const CHeader& contentType = ARequest->Headers["content-type"];
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
                CRequest Request;

                for (int I = 0; I < Data.Count(); I++) {
                    CHTTPContext Context = CHTTPContext(Data[I].Data(), Data[I].Size());
                    Context.State = Request::CParserState::header_line_start;
                    const int Result = Parse(&Request, Context);
                    if (Result == 1) {
                        FormData.Add(CFormDataItem());
                        CFormDataItem& DataItem = FormData.Last();

                        const CHeader& contentDisposition = Request.Headers["content-disposition"];

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

        bool CRequestParser::IsChar(int c) {
            return c >= 0 && c <= 127;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CRequestParser::IsCtl(int c) {
            return (c >= 0 && c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CRequestParser::IsTSpecial(int c) {
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

        bool CRequestParser::IsDigit(int c) {
            return c >= '0' && c <= '9';
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CWebSocketParser ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        void CWebSocketParser::Parse(CWebSocket *ARequest, CMemoryStream *AStream) {
            if (ARequest->Size() == 0) {
                ARequest->LoadFromStream(AStream);
            } else {
                ARequest->PayloadFromStream(AStream);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CReply ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        static const CReply::CStatusType StatusArray[] = {
                CReply::switching_protocols,
                CReply::ok,
                CReply::created,
                CReply::accepted,
                CReply::non_authoritative,
                CReply::no_content,
                CReply::multiple_choices,
                CReply::moved_permanently,
                CReply::moved_temporarily,
                CReply::see_other,
                CReply::not_modified,
                CReply::bad_request,
                CReply::unauthorized,
                CReply::forbidden,
                CReply::not_found,
                CReply::not_allowed,
                CReply::internal_server_error,
                CReply::not_implemented,
                CReply::bad_gateway,
                CReply::service_unavailable,
                CReply::gateway_timeout
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

            size_t ToBuffer(CReply::CStatusType AStatus, CStream *AStream) {
                switch (AStatus) {
                    case CReply::switching_protocols:
                        return StringArrayToStream(AStream, switching_protocols);
                    case CReply::ok:
                        return StringArrayToStream(AStream, ok);
                    case CReply::created:
                        return StringArrayToStream(AStream, created);
                    case CReply::accepted:
                        return StringArrayToStream(AStream, accepted);
                    case CReply::non_authoritative:
                        return StringArrayToStream(AStream, non_authoritative);
                    case CReply::no_content:
                        return StringArrayToStream(AStream, no_content);
                    case CReply::multiple_choices:
                        return StringArrayToStream(AStream, multiple_choices);
                    case CReply::moved_permanently:
                        return StringArrayToStream(AStream, moved_permanently);
                    case CReply::moved_temporarily:
                        return StringArrayToStream(AStream, moved_temporarily);
                    case CReply::see_other:
                        return StringArrayToStream(AStream, see_other);
                    case CReply::not_modified:
                        return StringArrayToStream(AStream, not_modified);
                    case CReply::bad_request:
                        return StringArrayToStream(AStream, bad_request);
                    case CReply::unauthorized:
                        return StringArrayToStream(AStream, unauthorized);
                    case CReply::forbidden:
                        return StringArrayToStream(AStream, forbidden);
                    case CReply::not_found:
                        return StringArrayToStream(AStream, not_found);
                    case CReply::not_allowed:
                        return StringArrayToStream(AStream, not_allowed);
                    case CReply::internal_server_error:
                        return StringArrayToStream(AStream, internal_server_error);
                    case CReply::not_implemented:
                        return StringArrayToStream(AStream, not_implemented);
                    case CReply::bad_gateway:
                        return StringArrayToStream(AStream, bad_gateway);
                    case CReply::service_unavailable:
                        return StringArrayToStream(AStream, service_unavailable);
                    case CReply::gateway_timeout:
                        return StringArrayToStream(AStream, gateway_timeout);
                    default:
                        return StringArrayToStream(AStream, internal_server_error);
                }
            }

            void ToString(CReply::CStatusType AStatus, CString &AString) {
                switch (AStatus) {
                    case CReply::switching_protocols:
                        AString = switching_protocols;
                        break;
                    case CReply::ok:
                        AString = ok;
                        break;
                    case CReply::created:
                        AString = created;
                        break;
                    case CReply::accepted:
                        AString = accepted;
                        break;
                    case CReply::non_authoritative:
                        AString = non_authoritative;
                        break;
                    case CReply::no_content:
                        AString = no_content;
                        break;
                    case CReply::multiple_choices:
                        AString = multiple_choices;
                        break;
                    case CReply::moved_permanently:
                        AString = moved_permanently;
                        break;
                    case CReply::moved_temporarily:
                        AString = moved_temporarily;
                        break;
                    case CReply::see_other:
                        AString = see_other;
                        break;
                    case CReply::not_modified:
                        AString = not_modified;
                        break;
                    case CReply::bad_request:
                        AString = bad_request;
                        break;
                    case CReply::unauthorized:
                        AString = unauthorized;
                        break;
                    case CReply::forbidden:
                        AString = forbidden;
                        break;
                    case CReply::not_found:
                        AString = not_found;
                        break;
                    case CReply::not_allowed:
                        AString = not_allowed;
                        break;
                    case CReply::internal_server_error:
                        AString = internal_server_error;
                        break;
                    case CReply::not_implemented:
                        AString = not_implemented;
                        break;
                    case CReply::bad_gateway:
                        AString = bad_gateway;
                        break;
                    case CReply::service_unavailable:
                        AString = service_unavailable;
                        break;
                    case CReply::gateway_timeout:
                        AString = gateway_timeout;
                        break;
                    default:
                        AString = internal_server_error;
                        break;
                }
            }
        } // namespace StatusStrings
        //--------------------------------------------------------------------------------------------------------------

        void CReply::ToBuffers(CMemoryStream *AStream) {

            StatusString = Status;
            StatusStrings::ToString(Status, StatusText);

            CString HTTP;
            HTTP.Format("HTTP/%d.%d %d %s", VMajor, VMinor, Status, StatusText.c_str());
            HTTP.SaveToStream(AStream);

            StringArrayToStream(AStream, MiscStrings::crlf);

            for (int i = 0; i < Headers.Count(); ++i) {
                CHeader &H = Headers[i];
                H.Name().SaveToStream(AStream);
                StringArrayToStream(AStream, MiscStrings::separator);
                H.Value().SaveToStream(AStream);
                StringArrayToStream(AStream, MiscStrings::crlf);
            }

            StringArrayToStream(AStream, MiscStrings::crlf);
            Content.SaveToStream(AStream);
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

            LPCTSTR ToString(CReply::CStatusType AStatus, CReply::CContentType AMessage) {

                if (AMessage > CReply::CContentType::json)
                    AMessage = CReply::CContentType::html;

                switch (AStatus) {
                    case CReply::switching_protocols:
                        return switching_protocols[AMessage];
                    case CReply::ok:
                        return ok[AMessage];
                    case CReply::created:
                        return created[AMessage];
                    case CReply::accepted:
                        return accepted[AMessage];
                    case CReply::non_authoritative:
                        return non_authoritative[AMessage];
                    case CReply::no_content:
                        return no_content[AMessage];
                    case CReply::multiple_choices:
                        return multiple_choices[AMessage];
                    case CReply::moved_permanently:
                        return moved_permanently[AMessage];
                    case CReply::moved_temporarily:
                        return moved_temporarily[AMessage];
                    case CReply::see_other:
                        return see_other[AMessage];
                    case CReply::not_modified:
                        return not_modified[AMessage];
                    case CReply::bad_request:
                        return bad_request[AMessage];
                    case CReply::unauthorized:
                        return unauthorized[AMessage];
                    case CReply::forbidden:
                        return forbidden[AMessage];
                    case CReply::not_found:
                        return not_found[AMessage];
                    case CReply::not_allowed:
                        return not_allowed[AMessage];
                    case CReply::internal_server_error:
                        return internal_server_error[AMessage];
                    case CReply::not_implemented:
                        return not_implemented[AMessage];
                    case CReply::bad_gateway:
                        return bad_gateway[AMessage];
                    case CReply::service_unavailable:
                        return service_unavailable[AMessage];
                    case CReply::gateway_timeout:
                        return gateway_timeout[AMessage];
                    default:
                        return internal_server_error[AMessage];
                }
            }

        } // namespace stock_replies
        //--------------------------------------------------------------------------------------------------------------

        void CReply::Clear() {
            VMajor = 1;
            VMinor = 1;
            Status = CStatusType::internal_server_error;
            StatusString.Clear();
            StatusText.Clear();
            ContentType = CContentType::html;
            CloseConnection = true;
            Headers.Clear();
            Content.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CReply::AddHeader(LPCTSTR lpszName, LPCTSTR lpszValue) {
            Headers.Add(CHeader());
            Headers.Last().Name() = lpszName;
            Headers.Last().Value() = lpszValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CReply::AddHeader(LPCTSTR lpszName, const CString &Value) {
            Headers.Add(CHeader());
            Headers.Last().Name() = lpszName;
            Headers.Last().Value() = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CReply::AddHeader(const CString &Name, const CString &Value) {
            Headers.Add(CHeader());
            Headers.Last().Name() = Name;
            Headers.Last().Value() = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CReply::ToText() {
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

        void CReply::ToJSON() {
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

        void CReply::StringToStatus() {
            int I = StrToInt(StatusString.c_str());
            for (const auto S : StatusArray) {
                if (I == S) {
                    Status = S;
                    break;
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        LPCTSTR CReply::GetGMT(LPTSTR lpszBuffer, size_t Size, time_t Delta) {
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

        void CReply::SetCookie(LPCTSTR lpszName, LPCTSTR lpszValue, LPCTSTR lpszPath, time_t Expires,
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
                Cookie << CReply::GetGMT(szDate, sizeof(szDate), Expires);
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

        CReply *CReply::GetReply(CReply *AReply, CStatusType AStatus, LPCTSTR AContentType) {

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
                AReply->AddHeader(_T("Content-Length"), IntToStr((int) AReply->Content.Size(), szSize, sizeof(szSize)));
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

        CReply *CReply::GetStockReply(CReply *AReply, CReply::CStatusType AStatus) {
            if (AStatus != CReply::no_content)
                AReply->Content = StockReplies::ToString(AStatus, AReply->ContentType);
            AReply = GetReply(AReply, AStatus);
            return AReply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CReply::AddUnauthorized(CReply *AReply, bool ABearer, LPCTSTR AError, LPCTSTR AMessage) {
            auto& LAuthenticate = AReply->Headers.Values(_T("WWW-Authenticate"));
            if (LAuthenticate.IsEmpty()) {
                CString Basic(_T("Basic realm=\"Access denied\", charset=\"UTF-8\""));

                CString Bearer;
                Bearer.Format(_T("Bearer realm=\"Access denied\", error=\"%s\", error_description=\"%s\", charset=\"UTF-8\""), AError, AMessage);

                AReply->AddHeader(_T("WWW-Authenticate"), ABearer ? Bearer : Basic);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CReplyParser ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        int CReplyParser::Consume(CReply *AReply, CReplyContext& Context) {
            char AInput = *Context.Begin++;
            switch (Context.State) {
                case Reply::http_version_h:
                    if (AInput == 'H') {
                        Context.State = Reply::http_version_t_1;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_t_1:
                    if (AInput == 'T') {
                        Context.State = Reply::http_version_t_2;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_t_2:
                    if (AInput == 'T') {
                        Context.State = Reply::http_version_p;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_p:
                    if (AInput == 'P') {
                        Context.State = Reply::http_version_slash;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_slash:
                    if (AInput == '/') {
                        AReply->VMajor = 0;
                        AReply->VMinor = 0;
                        Context.State = Reply::http_version_major_start;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_major_start:
                    if (IsDigit(AInput)) {
                        AReply->VMajor = AReply->VMajor * 10 + AInput - '0';
                        Context.State = Reply::http_version_major;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_major:
                    if (AInput == '.') {
                        Context.State = Reply::http_version_minor_start;
                        return -1;
                    } else if (IsDigit(AInput)) {
                        AReply->VMajor = AReply->VMajor * 10 + AInput - '0';
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_minor_start:
                    if (IsDigit(AInput)) {
                        AReply->VMinor = AReply->VMinor * 10 + AInput - '0';
                        Context.State = Reply::http_version_minor;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_version_minor:
                    if (AInput == ' ') {
                        Context.State = Reply::http_status_start;
                        return -1;
                    } else if (IsDigit(AInput)) {
                        AReply->VMinor = AReply->VMinor * 10 + AInput - '0';
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_status_start:
                    if (IsDigit(AInput)) {
                        AReply->StatusString.Append(AInput);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_status:
                    if (AInput == ' ') {
                        AReply->StringToStatus();
                        Context.State = Reply::http_status_text_start;
                        return -1;
                    } else if (IsDigit(AInput)) {
                        AReply->StatusString.Append(AInput);
                        Context.State = Reply::http_status;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_status_text_start:
                    if (IsChar(AInput)) {
                        AReply->StatusText.Append(AInput);
                        Context.State = Reply::http_status_text;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::http_status_text:
                    if (AInput == '\r') {
                        Context.State = Reply::expecting_newline_1;
                        return -1;
                    } else if (IsChar(AInput)) {
                        AReply->StatusText.Append(AInput);
                        Context.State = Reply::http_status_text;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::expecting_newline_1:
                    if (AInput == '\n') {
                        Context.State = Reply::header_line_start;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::header_line_start:
                    if (AInput == '\r') {
                        Context.State = Reply::expecting_newline_3;
                        return -1;
                    } else if ((AReply->Headers.Count() > 0) && (AInput == ' ' || AInput == '\t')) {
                        Context.State = Reply::header_lws;
                        return -1;
                    } else if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return false;
                    } else {
                        AReply->Headers.Add(CHeader());
                        AReply->Headers.Last().Name().Append(AInput);

                        Context.State = Reply::header_name;
                        return -1;
                    }
                case Reply::header_lws:
                    if (AInput == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (AInput == ' ' || AInput == '\t') {
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return false;
                    } else {
                        Context.State = Reply::header_value;
                        AReply->Headers.Last().Value().Append(AInput);
                        return -1;
                    }
                case Reply::header_name:
                    if (AInput == ':') {
                        Context.State = Reply::space_before_header_value;
                        return -1;
                    } else if (!IsChar(AInput) || IsCtl(AInput) || IsTSpecial(AInput)) {
                        return false;
                    } else {
                        AReply->Headers.Last().Name().Append(AInput);
                        return -1;
                    }
                case Reply::space_before_header_value:
                    if (AInput == ' ') {
                        Context.State = Reply::header_value;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::header_value:
                    if (AInput == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (AInput == ';') {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(AInput);
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return false;
                    } else {
                        AReply->Headers.Last().Value().Append(AInput);
                        return -1;
                    }
                case Reply::header_value_options_start:
                    if ((AInput == ' ' || AInput == '\t')) {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(AInput);
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return false;
                    } else {
                        Context.State = Reply::header_value_options;
                        AReply->Headers.Last().Value().Append(AInput);
                        AReply->Headers.Last().Data().Add(AInput);
                        return -1;
                    }
                case Reply::header_value_options:
                    if (AInput == '\r') {
                        Context.State = Reply::expecting_newline_2;
                        return -1;
                    } else if (AInput == ';') {
                        Context.State = Reply::header_value_options_start;
                        AReply->Headers.Last().Value().Append(AInput);
                        return -1;
                    } else if (IsCtl(AInput)) {
                        return false;
                    } else {
                        AReply->Headers.Last().Value().Append(AInput);
                        AReply->Headers.Last().Data().back().Append(AInput);
                        return -1;
                    }
                case Reply::expecting_newline_2:
                    if (AInput == '\n') {
                        Context.State = Reply::header_line_start;
                        return -1;
                    } else {
                        return false;
                    }
                case Reply::expecting_newline_3:
                    if (AInput == '\n') {
                        Context.ContentLength = Context.End - Context.Begin;

                        if (AReply->Headers.Count() > 0) {
                            const CString &contentLength = AReply->Headers.Values(_T("Content-Length"));
                            if (!contentLength.IsEmpty()) {
                                AReply->ContentLength = strtoul(contentLength.c_str(), nullptr, 0);
                            } else {
                                AReply->ContentLength = Context.ContentLength;
                            }
                        } else {
                            AReply->ContentLength = Context.ContentLength;
                        }

                        if (AReply->ContentLength > 0) {
                            Context.State = Reply::content;
                            return -1;
                        }

                        return true;
                    } else {
                        return false;
                    }
                case Reply::content:
                    AReply->Content.Append(AInput);
                    if (AReply->Content.Size() < AReply->ContentLength) {
                        return -1;
                    }
                    return true;

                default:
                    return false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CReplyParser::Parse(CReply *AReply, CReplyContext& Context) {
            Context.Result = -1;
            while (Context.Result == -1 && Context.Begin != Context.End) {
                Context.Result = Consume(AReply, Context);
            }
            return Context.Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CReplyParser::IsChar(int c) {
            return c >= 0 && c <= 127;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CReplyParser::IsCtl(int c) {
            return (c >= 0 && c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CReplyParser::IsTSpecial(int c) {
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

        bool CReplyParser::IsDigit(int c) {
            return c >= '0' && c <= '9';
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServerConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPServerConnection::CHTTPServerConnection(CPollSocketServer *AServer):
                CTCPServerConnection(AServer) {
            m_Protocol = pHTTP;
            m_State = Request::method_start;
            m_CloseConnection = true;
            m_ConnectionStatus = csConnected;
            m_Request = nullptr;
            m_Reply = nullptr;
            m_WSRequest = nullptr;
            m_WSReply = nullptr;
            m_OnReply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPServerConnection::~CHTTPServerConnection() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        CRequest *CHTTPServerConnection::GetRequest() {
            if (m_Request == nullptr)
                m_Request = new CRequest();
            return m_Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        CReply *CHTTPServerConnection::GetReply() {
            if (m_Reply == nullptr) {
                m_Reply = new CReply();
                m_Reply->ServerName = Server()->ServerName();
                m_Reply->AllowedMethods = Server()->AllowedMethods();
            }
            return m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        CWebSocket *CHTTPServerConnection::GetWSRequest() {
            if (m_WSRequest == nullptr)
                m_WSRequest = new CWebSocket();
            return m_WSRequest;
        }
        //--------------------------------------------------------------------------------------------------------------

        CWebSocket *CHTTPServerConnection::GetWSReply() {
            if (m_WSReply == nullptr)
                m_WSReply = new CWebSocket();
            return m_WSReply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::Clear() {
            m_State = Request::method_start;
            FreeAndNil(m_Request);
            FreeAndNil(m_Reply);
            FreeAndNil(m_WSRequest);
            FreeAndNil(m_WSReply);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::ParseHTTP(CMemoryStream *Stream) {
            CHTTPContext Context = CHTTPContext((LPCTSTR) Stream->Memory(), Stream->Size(), m_State);
            const int ParseResult = CRequestParser::Parse(GetRequest(), Context);

            switch (ParseResult) {
                case 0:
                    m_ConnectionStatus = csRequestError;
                    break;

                case 1:
                    m_ConnectionStatus = csRequestOk;
                    DoRequest();
                    break;

                default:
                    m_State = Context.State;
                    if (RecvBufferSize() < GetRequest()->ContentLength)
                        RecvBufferSize(GetRequest()->ContentLength);
                    m_ConnectionStatus = csWaitRequest;
                    break;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::ParseWebSocket(CMemoryStream *Stream) {

            auto LWSRequest = GetWSRequest();

            CWebSocketParser::Parse(LWSRequest, Stream);

            switch (LWSRequest->Frame().Opcode) {
                case WS_OPCODE_CONTINUATION:
                case WS_OPCODE_TEXT:
                case WS_OPCODE_BINARY:

                    if (LWSRequest->Frame().FIN == 0 || (LWSRequest->Size() < LWSRequest->Payload()->Size())) {
                        m_ConnectionStatus = csWaitRequest;
                    } else {
                        m_ConnectionStatus = csRequestOk;
                        DoRequest();
                    }

                    break;

                case WS_OPCODE_CLOSE:
                    Disconnect();
                    break;

                case WS_OPCODE_PING:
                    SendWebSocketPong();
                    break;

                case WS_OPCODE_PONG:
                    m_ConnectionStatus = csRequestOk;
                    break;

                default:
                    m_CloseConnection = true;
                    SendWebSocketClose();
                    break;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServerConnection::ParseInput() {
            bool Result = false;
            if (Connected()) {
                CMemoryStream Stream(ReadAsync());
                try {
                    Result = Stream.Size() > 0;
                    if (Result) {
                        InputBuffer()->Extract(Stream.Memory(), Stream.Size());
                        switch (m_Protocol) {
                            case pHTTP:
                                Tag(clock());
                                ParseHTTP(&Stream);
                                break;
                            case pWebSocket:
                                Tag(clock());
                                ParseWebSocket(&Stream);
                                break;
                        }
                    }
                } catch (...) {
                    throw;
                }
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendStockReply(CReply::CStatusType AStatus, bool ASendNow) {

            GetReply()->CloseConnection = CloseConnection();

            CReply::GetStockReply(m_Reply, AStatus);

            SendReply(ASendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(CReply::CStatusType AStatus, LPCTSTR AContentType, bool ASendNow) {

            if (AStatus == CReply::ok) {
                const CString &Value = GetRequest()->Headers.Values(_T("Connection")).Lower();
                if (!Value.IsEmpty()) {
                    if (Value == _T("keep-alive") || Value == _T("upgrade"))
                        CloseConnection(false);
                }
            }

            GetReply()->CloseConnection = CloseConnection();

            CReply::GetReply(m_Reply, AStatus, AContentType);

            SendReply(ASendNow);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendReply(bool ASendNow) {

            GetReply()->ToBuffers(OutputBuffer());

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

            m_Protocol = pWebSocket;

            RecvBufferSize(256 * 1024);

            CloseConnection(false);

            auto LReply = GetReply();

            LReply->Status = CReply::switching_protocols;

            LReply->AddHeader("Upgrade", "websocket");
            LReply->AddHeader("Connection", "Upgrade");

            if (!Accept.IsEmpty())
                LReply->AddHeader("Sec-WebSocket-Accept", base64_encode(Accept));

            if (!Protocol.IsEmpty())
                LReply->AddHeader("Sec-WebSocket-Protocol", Protocol);

            SendReply();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendWebSocket(bool ASendNow) {

            GetWSReply()->SaveToStream(OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendWebSocketPing(bool ASendNow) {

            GetWSReply()->Ping(OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendWebSocketPong(bool ASendNow) {

            GetWSReply()->Pong(OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::SendWebSocketClose(bool ASendNow) {

            GetWSReply()->Close(OutputBuffer());

            m_ConnectionStatus = csReplyReady;

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csReplySent;
                Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::DoRequest() {
            if (m_OnRequest != nullptr) {
                m_OnRequest(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServerConnection::DoReply() {
            if (m_OnReply != nullptr) {
                m_OnReply(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPClientConnection -------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientConnection::CHTTPClientConnection(CPollSocketClient *AClient): CTCPClientConnection(AClient) {
            m_State = Reply::http_version_h;
            m_CloseConnection = false;
            m_ConnectionStatus = csConnected;
            m_Request = nullptr;
            m_Reply = nullptr;
            m_OnReply = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPClientConnection::~CHTTPClientConnection() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::Clear() {
            m_State = Reply::http_version_h;
            FreeAndNil(m_Request);
            FreeAndNil(m_Reply);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPClientConnection::ParseInput() {
            bool Result = false;
            if (Connected()) {
                CMemoryStream LStream(ReadAsync());
                try {
                    Result = LStream.Size() > 0;
                    if (Result) {
                        InputBuffer()->Extract(LStream.Memory(), LStream.Size());

                        CReplyContext Context = CReplyContext((LPCTSTR) LStream.Memory(), LStream.Size(), m_State);
                        const int ParseResult = CReplyParser::Parse(GetReply(), Context);

                        switch (ParseResult) {
                            case 0:
                                Tag(clock());
                                m_ConnectionStatus = csReplyError;
                                break;

                            case 1:
                                Tag(clock());
                                m_ConnectionStatus = csReplyOk;
                                DoReply();
                                break;

                            default:
                                m_State = Context.State;
                                if (RecvBufferSize() < GetReply()->ContentLength)
                                    RecvBufferSize(GetReply()->ContentLength);
                                m_ConnectionStatus = csWaitReply;
                                break;
                        }
                    }
                } catch (...) {
                    throw;
                }
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        CRequest *CHTTPClientConnection::GetRequest() {
            if (m_Request == nullptr) {
                m_Request = new CRequest();
                m_Request->Location.hostname = Client()->Host();
                m_Request->Location.port = Client()->Port();
                m_Request->UserAgent = Client()->ClientName();
            }
            return m_Request;
        }
        //--------------------------------------------------------------------------------------------------------------

        CReply *CHTTPClientConnection::GetReply() {
            if (m_Reply == nullptr) {
                m_Reply = new CReply;
            }
            return m_Reply;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::SendRequest(bool ASendNow) {

            GetRequest()->ToBuffers(OutputBuffer());

            m_ConnectionStatus = csRequestReady;

            DoRequest();

            if (ASendNow) {
                WriteAsync();
                m_ConnectionStatus = csRequestSent;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::DoRequest() {
            if (m_OnRequest != nullptr) {
                m_OnRequest(this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClientConnection::DoReply() {
            if (m_OnReply != nullptr) {
                m_OnReply(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPServer -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPServer::CHTTPServer(): CAsyncServer() {

        }
        //--------------------------------------------------------------------------------------------------------------

        CHTTPServer::CHTTPServer(const CString &IP, unsigned short Port): CHTTPServer() {
            DefaultIP() = IP;
            DefaultPort(Port);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::InitializeBindings() {
            CSocketHandle* LBinding = Bindings()->Add();
            for (int i = 0; i < m_Sites.Count(); ++i) {
                const auto& Site = m_Sites[i];
                const auto& Port = Site.Value()["listen"];
                if (Site.Name() == "*") {
                    if (!Port.IsEmpty())
                        LBinding->Port(Port.AsInteger());
                } else {
                    if (!Port.IsEmpty() && Port.AsInteger() != DefaultPort()) {
                        LBinding = Bindings()->Add();
                        LBinding->Port(Port.AsInteger());
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
                if (ch == ' ') {
                    Out += '+';
                } else if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
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
                } else if (ch == '+') {
                    Out += ' ';
                } else {
                    Out += ch;
                }
            }
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoTimeOut(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            try {
                if (LConnection->ConnectionStatus() >= csRequestOk) {

                    if (LConnection->ConnectionStatus() == csRequestOk) {
                        if (LConnection->Protocol() == pHTTP)
                            LConnection->SendStockReply(CReply::gateway_timeout, true);
                        LConnection->CloseConnection(true);
                    }

                    if (LConnection->CloseConnection())
                        LConnection->Disconnect();
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoAccept(CPollEventHandler *AHandler) {
            CIOHandlerSocket *LIOHandler = nullptr;
            CPollEventHandler *LEventHandler = nullptr;
            CHTTPServerConnection *LConnection = nullptr;

            try {
                LIOHandler = (CIOHandlerSocket *) CServerIOHandler::Accept(AHandler->Socket(), SOCK_NONBLOCK);

                if (Assigned(LIOHandler)) {
                    LConnection = new CHTTPServerConnection(this);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    LConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
                    LConnection->OnReply([this](auto && Sender) { DoReply(Sender); });
#else
                    LConnection->OnDisconnected(std::bind(&CHTTPServer::DoDisconnected, this, _1));
                    LConnection->OnReply(std::bind(&CHTTPServer::DoReply, this, _1));
#endif
                    LConnection->IOHandler(LIOHandler);

                    LIOHandler->AfterAccept();

                    LEventHandler = m_EventHandlers->Add(LIOHandler->Binding()->Handle());
                    LEventHandler->Binding(LConnection);
                    LEventHandler->Start(etIO);

                    DoConnected(LConnection);
                } else {
                    throw ETCPServerError(_T("TCP Server Error..."));
                }
            } catch (Delphi::Exception::Exception &E) {
                delete LConnection;
                DoListenException(&E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoRead(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            try {
                if (LConnection->ParseInput()) {
                    switch (LConnection->ConnectionStatus()) {
                        case csRequestError:
                            LConnection->CloseConnection(true);
                            if (LConnection->Protocol() == pHTTP)
                                LConnection->SendStockReply(CReply::bad_request);
                            LConnection->Clear();
                            break;

                        case csRequestOk:
                            DoExecute(LConnection);
                            break;

                        default:
                            break;
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPServer::DoWrite(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPServerConnection *> (AHandler->Binding());
            try {
                if (LConnection->WriteAsync()) {
                    if (LConnection->ConnectionStatus() == csReplyReady) {

                        LConnection->ConnectionStatus(csReplySent);
                        LConnection->Clear();

                        if (LConnection->CloseConnection()) {
                            LConnection->Disconnect();
                        }
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPServer::DoCommand(CTCPConnection *AConnection) {
            auto LConnection = dynamic_cast<CHTTPServerConnection *> (AConnection);
            auto LRequest = LConnection->Request();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, LRequest->Method.c_str());
                try {
                    int i;
                    for (i = 0; i < CommandHandlers()->Count(); ++i) {
                        if (CommandHandlers()->Commands(i)->Enabled()) {
                            if (CommandHandlers()->Commands(i)->Check(LRequest->Method.c_str(), LRequest->Method.Size(), AConnection))
                                break;
                        }
                    }

                    if (i == CommandHandlers()->Count())
                        DoNoCommandHandler(LRequest->Method.c_str(), AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, &E);
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

        CHTTPClient::CHTTPClient(LPCTSTR AHost, unsigned short APort): CHTTPClient() {
            m_Host = AHost;
            m_Port = APort;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto LConnection = new CHTTPClientConnection(this);
            LConnection->IOHandler(AIOHandler);
            AHandler->Binding(LConnection, true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoConnect(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

            if (LConnection == nullptr) {
                AHandler->Stop();
                return;
            }

            try {
                auto LIOHandler = (CIOHandlerSocket *) LConnection->IOHandler();

                if (LIOHandler->Binding()->CheckConnection()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    LConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    LConnection->OnDisconnected(std::bind(&CHTTPClient::DoDisconnected, this, _1));
#endif
                    AHandler->Start(etIO);
                    DoConnected(LConnection);
                    DoRequest(LConnection);
                }
            } catch (Exception::Exception &E) {
                DoException(LConnection, &E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoRead(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());
            try {
                if (LConnection->ParseInput()) {
                    switch (LConnection->ConnectionStatus()) {
                        case csReplyError:
                            LConnection->Clear();
                            break;

                        case csReplyOk:
                            DoExecute(LConnection);
                            LConnection->Clear();

                            if (LConnection->CloseConnection()) {
                                LConnection->Disconnect();
                            }

                            break;

                        default:
                            break;
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPClient::DoWrite(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());
            try {
                if (LConnection->WriteAsync()) {
                    if (LConnection->ConnectionStatus() == csRequestReady) {
                        LConnection->ConnectionStatus(csRequestSent);
                    }
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                LConnection->Disconnect();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CHTTPClient::DoCommand(CTCPConnection *AConnection) {
            auto LConnection = dynamic_cast<CHTTPClientConnection *> (AConnection);
            auto LRequest = LConnection->Request();

            bool Result = CommandHandlers()->Count() > 0;

            if (Result) {
                DoBeforeCommandHandler(AConnection, LRequest->Method.c_str());
                try {
                    int i;
                    for (i = 0; i < CommandHandlers()->Count(); ++i) {
                        if (CommandHandlers()->Commands(i)->Enabled()) {
                            if (CommandHandlers()->Commands(i)->Check(LRequest->Method.c_str(),
                                                                      LRequest->Method.Size(), AConnection))
                                break;
                        }
                    }

                    if (i == CommandHandlers()->Count())
                        DoNoCommandHandler(LRequest->Method.c_str(), AConnection);
                } catch (Delphi::Exception::Exception &E) {
                    DoException(AConnection, &E);
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
                m_OnRequest(AConnection->Request());
                AConnection->SendRequest(true);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxy ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPProxy::CHTTPProxy(CHTTPServerConnection *AConnection, CHTTPProxyManager *AManager):
                CCollectionItem(AManager), CHTTPClient() {

            m_Request = nullptr;

            m_pConnection = AConnection;
            m_ClientName = Server()->ServerName();

            SetPollStack(Server()->PollStack());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoConnectStart(CIOHandlerSocket *AIOHandler, CPollEventHandler *AHandler) {
            auto LConnection = new CHTTPClientConnection(this);
            LConnection->IOHandler(AIOHandler);
            AHandler->Binding(LConnection, true);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoConnect(CPollEventHandler *AHandler) {
            auto LConnection = dynamic_cast<CHTTPClientConnection *> (AHandler->Binding());

            if (LConnection == nullptr) {
                AHandler->Stop();
                return;
            }

            try {
                auto LIOHandler = (CIOHandlerSocket *) LConnection->IOHandler();

                if (LIOHandler->Binding()->CheckConnection()) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                    LConnection->OnDisconnected([this](auto && Sender) { DoDisconnected(Sender); });
#else
                    LConnection->OnDisconnected(std::bind(&CHTTPProxy::DoDisconnected, this, _1));
#endif
                    AHandler->Start(etIO);
                    DoConnected(LConnection);
                    DoRequest(LConnection);
                }
            } catch (Delphi::Exception::Exception &E) {
                DoException(LConnection, &E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CHTTPProxy::DoRequest(CHTTPClientConnection *AConnection) {
            auto LRequest = AConnection->Request();
            *LRequest = *m_Request;
            AConnection->SendRequest(true);
        }
        //--------------------------------------------------------------------------------------------------------------

        CRequest *CHTTPProxy::GetRequest() {
            if (m_Request == nullptr) {
                m_Request = new CRequest();
                m_Request->Location.hostname = Host();
                m_Request->Location.port = Port();
                m_Request->UserAgent = ClientName();
            }

            return m_Request;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CHTTPProxyManager -----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CHTTPProxy *CHTTPProxyManager::Add(CHTTPServerConnection *AConnection) {
            return new CHTTPProxy(AConnection, this);
        }
        //--------------------------------------------------------------------------------------------------------------

    }
}
}
