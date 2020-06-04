/*++

Library name:

  libdelphi

Module Name:

  CURL.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_CURL_HPP
#define DELPHI_CURL_HPP
//----------------------------------------------------------------------------------------------------------------------

#include <curl/curl.h>
//----------------------------------------------------------------------------------------------------------------------

#ifdef  __cplusplus
extern "C++" {
#endif // __cplusplus

namespace Delphi {

    namespace cURL {

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlComponent --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCurlComponent {
        public:
            CCurlComponent();
            ~CCurlComponent();
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlApi --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCurlApi: public CCurlComponent {
        private:

            CURL *m_curl;

            CURLcode m_Code;

            void Init();
            void Cleanup();

        protected:

            static size_t CallBack(void *content, size_t size, size_t nmemb, CString *Buffer);

        public:

            CCurlApi();

            ~CCurlApi();

            CURLcode Code() const { return m_Code; };

            CString GetErrorMessage() const;

            void Reset();

            void Send(const CString &url, const CString &Result);
            void Send(const CString &url, const CString &Result,
                      const CStringList &Headers, const CString &PostData, const CString &Action);

        };
    }
}

using namespace Delphi::cURL;
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // DELPHI_CURL_HPP
