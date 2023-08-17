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

            size_t m_TimeOut;

            mutable CHeaders m_Headers;
            mutable CString m_Result;

            virtual void CurlInfo() const abstract;

            static size_t CallBack(void *content, size_t size, size_t nmemb, CString *Buffer);
            static size_t HeaderCallBack(char *buffer, size_t size, size_t nitems, CHeaders *Headers);

        public:

            CCurlApi();

            virtual ~CCurlApi();

            CURL *Handle() const { return m_curl; }

            virtual void Reset() const;

            size_t TimeOut() const { return m_TimeOut; }
            void TimeOut(size_t Value) { m_TimeOut = Value; }

            const CHeaders &Headers() const { return m_Headers; }
            const CString &Result() const { return m_Result; }

            virtual CURLcode Get(const CLocation &URL, const CHeaders &Headers) const;
            virtual CURLcode Post(const CLocation &URL, const CString &Content, const CHeaders &Headers) const;
            virtual CURLcode Send(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers, bool bTunnel) const;

            static CString GetErrorMessage(CURLcode code);

        };
    }
}

using namespace Delphi::cURL;
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // DELPHI_CURL_HPP
