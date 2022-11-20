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
            m_Code = CURLE_OK;
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

        size_t CCurlApi::CallBack(void *content, size_t size, size_t nmemb, CString *Buffer) {
            size_t buffer_size = size * nmemb;
            Buffer->Append((LPCTSTR) content, buffer_size);
            return buffer_size;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CCurlApi::GetErrorMessage() const {
            return curl_easy_strerror(m_Code);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlApi::Reset() {
            curl_easy_reset(m_curl);
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CCurlApi::Get(const CString &URL) {
            return Send(URL, "GET", CString(), CStringList());
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CCurlApi::Post(const CString &URL, const CString &Content) {
            return Send(URL, "POST", Content, CStringList());
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CCurlApi::Send(const CString &URL, const CString &Action, const CString &Content, const CStringList &Headers) {
            CString Result;

            if ( m_curl ) {
                curl_easy_reset(m_curl);

                curl_easy_setopt(m_curl, CURLOPT_URL, URL.c_str());
                curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CCurlApi::CallBack);
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &Result);
                curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, FALSE);

                if ( Headers.Count() > 0 ) {

                    struct curl_slist *chunk = nullptr;
                    for ( int i = 0; i < Headers.Count(); i++ ) {
                        chunk = curl_slist_append(chunk, Headers[i].c_str());
                    }

                    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, chunk);
                }

                if ( Action == "GET" ) {

                    curl_easy_setopt(m_curl, CURLOPT_HTTPGET, TRUE);

                } else if ( Action == "POST" || Action == "PUT" || Action == "DELETE" ) {

                    if ( Action == "PUT" || Action == "DELETE" ) {
                        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, Action.c_str() );
                    } else {
                        curl_easy_setopt(m_curl, CURLOPT_POST, TRUE);

                        if (!Content.IsEmpty()) {
                            curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, Content.c_str());
                        }
                    }
                }

                m_Code = curl_easy_perform(m_curl);
            }

            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

    }
}
}
