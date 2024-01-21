/*++

Library name:

  libdelphi

Module Name:

  CURL.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/CURL.hpp"

#define DELPHI_CURL_TIMEOUT 30

extern "C++" {

namespace Delphi {

    namespace cURL {

        static unsigned long GURLCount = 0;

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlComponent --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCurlComponent::CCurlComponent() {
            if (GURLCount == 0) {
                curl_global_init(CURL_GLOBAL_DEFAULT);
            }
            GURLCount++;
        }
        //--------------------------------------------------------------------------------------------------------------

        CCurlComponent::~CCurlComponent() {
            GURLCount--;
            if (GURLCount == 0) {
                curl_global_cleanup();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlApi --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCurlApi::CCurlApi() {
            m_curl = nullptr;
            m_TimeOut = DELPHI_CURL_TIMEOUT;

            Init();
        }
        //--------------------------------------------------------------------------------------------------------------

        CCurlApi::~CCurlApi() {
            Cleanup();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlApi::Init() {
            m_curl = curl_easy_init();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlApi::Cleanup() {
            if (m_curl != nullptr)
                curl_easy_cleanup(m_curl);
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CCurlApi::WriteCallBack(void *content, size_t size, size_t nmemb, CString *Buffer) {
            size_t buffer_size = nmemb * size;
            Buffer->Append((LPCTSTR) content, buffer_size);
            return buffer_size;
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CCurlApi::ReadCallBack(char *buffer, size_t size, size_t nmemb, CString *Content) {
            const auto content_size = Content->Size() - Content->Position();
            size_t buffer_size = Min(nmemb * size, content_size);
            Content->ReadBuffer(buffer, buffer_size);
            return buffer_size;
        }
        //--------------------------------------------------------------------------------------------------------------

        size_t CCurlApi::HeaderCallBack(char *buffer, size_t size, size_t nitems, CHeaders *Headers) {
            size_t buffer_size = nitems * size;
            if (buffer_size > 2) {
                const CString S((LPCTSTR) buffer, buffer_size - 2);
                const auto pos = S.Find(": ");
                if (pos != CString::npos) {
                    Headers->AddPair(S.SubString(0, pos), S.SubString(pos + 2));
                }
            }
            return buffer_size;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CCurlApi::GetErrorMessage(CURLcode code) {
            return curl_easy_strerror(code);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlApi::Reset() const {
            m_Headers.Clear();
            m_Result.Clear();
            curl_easy_reset(m_curl);
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Get(const CLocation &URL, const CHeaders &Headers) const {
            return Send(URL, "GET", CString(), Headers, false);
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Post(const CLocation &URL, const CString &Content, const CHeaders &Headers) const {
            return Send(URL, "POST", Content, Headers, false);
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Send(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers, bool bTunnel) const {
            CURLcode code = CURLE_SEND_ERROR;
            struct curl_slist *chunk = nullptr;

            if (m_curl) {
                Reset();

                curl_easy_setopt(m_curl, CURLOPT_URL, URL.href().c_str());
                curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, CCurlApi::HeaderCallBack);
                curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_Headers);
                curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CCurlApi::WriteCallBack);
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_Result);
                curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, TRUE);
                curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, m_TimeOut);
                curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(m_curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");
                curl_easy_setopt(m_curl, CURLOPT_HTTP_CONTENT_DECODING, 1L);

                if (bTunnel) {
                    curl_easy_setopt(m_curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
                }
#ifdef _DEBUG
                curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
#endif

                if (Headers.Count() > 0) {
                    CStringList List;

                    List.NameValueSeparator(": ");
                    List << Headers;

                    for (int i = 0; i < List.Count(); ++i) {
                        chunk = curl_slist_append(chunk, List[i].c_str());
                    }

                    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, chunk);
                }

                if (Method == "GET") {

                    curl_easy_setopt(m_curl, CURLOPT_HTTPGET, TRUE);

                } else if (Method == "POST" || Method == "PUT" || Method == "DELETE") {

                    if (Method == "POST") {
                        curl_easy_setopt(m_curl, CURLOPT_POST, TRUE);

                        if (!Content.IsEmpty()) {
                            curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, Content.c_str());
                        }
                    } else if (Method == "PUT") {
                        curl_easy_setopt(m_curl, CURLOPT_UPLOAD, TRUE);

                        if (!Content.IsEmpty()) {
                            Content.Position(0);

                            curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, CCurlApi::ReadCallBack);
                            curl_easy_setopt(m_curl, CURLOPT_READDATA, &Content);
                            curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) Content.Size());
                        }
                    } else {
                        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, Method.c_str() );
                    }
                }

                code = curl_easy_perform(m_curl);

                if (chunk != nullptr)
                    curl_slist_free_all(chunk);

                if (code == CURLE_OK) {
                    CurlInfo();
                }
            }

            return code;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlFetch ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCurlFetch::CCurlFetch(): CCurlApi() {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlFetch::CurlInfo() const {
            char *url = nullptr;

            TCHAR szString[_INT_T_LEN + 1] = {0};

            long response_code;

            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_getinfo(m_curl, CURLINFO_EFFECTIVE_URL, &url);

            m_Into.Clear();
            m_Into.AddPair("CURLINFO_RESPONSE_CODE", IntToStr((int) response_code, szString, _INT_T_LEN));
            m_Into.AddPair("CURLINFO_EFFECTIVE_URL", url);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CCurlFetch::GetResponseCode() const {
            const auto &code = m_Into["CURLINFO_RESPONSE_CODE"];
            return code.empty() ? 0 : (int) StrToInt(code.c_str());
        }

    }
}
}
