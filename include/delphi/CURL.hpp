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

            mutable CStringList m_Headers;
            mutable CString m_Result;

            virtual void CurlInfo() const abstract;

            static size_t CallBack(void *content, size_t size, size_t nmemb, CString *Buffer);
            static size_t HeaderCallBack(char *buffer, size_t size, size_t nitems, CStringList *Headers);

        public:

            CCurlApi();

            virtual ~CCurlApi();

            static CString GetErrorMessage(CURLcode code);

            virtual void Reset() const;

            const CStringList &Headers() const { return m_Headers; }
            const CString &Result() const { return m_Result; }

            virtual CURLcode Get(const CLocation &URL, const CStringList &Headers) const;
            virtual CURLcode Post(const CLocation &URL, const CString &Content, const CStringList &Headers) const;
            virtual CURLcode Send(const CLocation &URL, const CString &Method, const CString &Content, const CStringList &Headers, bool bTunnel) const;

        };
    }
}

using namespace Delphi::cURL;
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // DELPHI_CURL_HPP
