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

            void Init();
            void Cleanup();

        protected:

            CURL *m_curl;
            CURLcode m_Code;

            CString m_Result;

            static size_t CallBack(void *content, size_t size, size_t nmemb, CString *Buffer);

        public:

            CCurlApi();

            virtual ~CCurlApi();

            CURLcode Code() const { return m_Code; };
            CString Result() const { return m_Result; };

            CString GetErrorMessage() const;

            virtual void Reset();

            virtual CString Get(const CString &URL);
            virtual CString Post(const CString &URL, const CString &Content);
            virtual CString Send(const CString &URL, const CString &Action, const CString &Content, const CStringList &Headers);

        };
    }
}

using namespace Delphi::cURL;
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // DELPHI_CURL_HPP
