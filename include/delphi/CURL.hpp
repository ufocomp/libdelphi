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

            bool m_bTunnel;

            char m_Error[CURL_ERROR_SIZE] = {0};

        protected:

            CURL *m_curl;

            size_t m_TimeOut;

            mutable CHeaders m_Headers;
            mutable CString m_Result;

            mutable struct curl_slist *m_SList;

            virtual void CurlInfo() const abstract;

            static size_t WriteCallBack(void *content, size_t size, size_t nmemb, CString *Buffer);
            static size_t ReadCallBack(char *buffer, size_t size, size_t nmemb, CString *Content);
            static size_t HeaderCallBack(char *buffer, size_t size, size_t nitems, CHeaders *Headers);

        public:

            CCurlApi();

            virtual ~CCurlApi();

            CURL *Handle() const { return m_curl; }

            virtual void Reset() const;

            size_t TimeOut() const { return m_TimeOut; }
            void TimeOut(size_t Value) { m_TimeOut = Value; }

            bool Tunnel() const { return m_bTunnel; }
            void Tunnel(bool Value) { m_bTunnel = Value; }

            const char *Error() const { return m_Error; }

            const CHeaders &Headers() const { return m_Headers; }
            const CString &Result() const { return m_Result; }

            virtual CURLcode Get(const CLocation &URL, const CHeaders &Headers) const;
            virtual CURLcode Post(const CLocation &URL, const CString &Content, const CHeaders &Headers) const;
            virtual CURLcode Send(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const;

            virtual void Prepare(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const;
            virtual CURLcode Perform() const;

            static CString GetErrorMessage(CURLcode code);

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlFetch ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCURLClient;
        class CCurlFetch;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CCurlFetch *Sender, CURLcode code, const CString &Error)> COnCurlFetchEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CCurlFetch: public CCurlApi {
            friend CCURLClient;

        private:

            mutable CStringList m_Into;

            COnCurlFetchEvent m_OnDone;
            COnCurlFetchEvent m_OnFail;

            void DoDone(CURLcode code);
            void DoFail(CURLcode code);

        protected:

            void CurlInfo() const override;

        public:

            CCurlFetch();
            ~CCurlFetch() override = default;

            const CStringList &Info() const { return m_Into; }

            int GetResponseCode() const;

            COnCurlFetchEvent &OnDone() { return m_OnDone; }
            const COnCurlFetchEvent &OnDone() const { return m_OnDone; }
            void OnDone(COnCurlFetchEvent && Value) { m_OnDone = Value; }

            COnCurlFetchEvent &OnFail() { return m_OnFail; }
            const COnCurlFetchEvent &OnFail() const { return m_OnFail; }
            void OnFail(COnCurlFetchEvent && Value) { m_OnFail = Value; }

        };
        //--------------------------------------------------------------------------------------------------------------

        class CCurlAsyncFetch: public CCurlFetch {
        private:

            mutable CString m_Content;

        public:

            CCurlAsyncFetch();
            ~CCurlAsyncFetch() override = default;

            void Prepare(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const override;

            const CString &Content() const { return m_Content; }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CCURLClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CCURLClient *Sender, const Delphi::Exception::Exception &E)> COnCurlClientExceptionEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CCURLClient: public CEPoll {
        private:

            CURLM *m_Handle;

            CEPollTimer *m_pTimer;

            int m_Action;
            int m_StillRunning;
            int m_TimeOut;

            COnCurlClientExceptionEvent m_OnException;

            void InitEventHandler(CPollEventHandler *AHandler);

            void UpdateTimer(long int Value, long int Interval = 0);

            void MultiInfo();

            static void ErrorCheck(const char *where, CURLMcode code);

        protected:

            static void AddSocket(CCURLClient *AClient, curl_socket_t s, int action);
            static void SetSocket(CPollEventHandler *AHandler, int action);
            static void RemoveSocket(CPollEventHandler *AHandler);

            static int SocketCallBack(CURL *curl, curl_socket_t s, int what, CCURLClient *AClient, CPollEventHandler *AHandler);
            static int MultiTimerCallBack(CURLM *multi, long timeout_ms, CCURLClient *AClient);

            void DoTimer(CPollEventHandler *AHandler) override;
            void DoTimeOut(CPollEventHandler *AHandler) override;
            void DoConnect(CPollEventHandler *AHandler) override {};
            void DoAccept(CPollEventHandler *AHandler) override {};
            void DoRead(CPollEventHandler *AHandler) override {};
            void DoWrite(CPollEventHandler *AHandler) override {};
            void DoError(CPollEventHandler *AHandler) override {};

            void DoEvent(CPollEventHandler *AHandler, uint32_t events) override;

            void DoException(const Delphi::Exception::Exception &E);
            
        public:

            CCURLClient();
            ~CCURLClient() override;

            CURLM *Handle() const { return m_Handle; }

            int Action() const { return m_Action; }

            int TimeOut() const { return m_TimeOut; }
            void TimeOut(int Value) { m_TimeOut = Value; }

            CPollEventHandler *GetEventHandler(CSocket Socket);

            CURLMcode Perform(const CLocation &URL, const CString &Method, const CString &Content,
                const CHeaders &Headers, COnCurlFetchEvent && OnDone, COnCurlFetchEvent && OnFail);

            COnCurlClientExceptionEvent &OnException() { return m_OnException; }
            const COnCurlClientExceptionEvent &OnException() const { return m_OnException; }
            void OnException(COnCurlClientExceptionEvent && Value) { m_OnException = Value; }

        };

    }
}

using namespace Delphi::cURL;
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // DELPHI_CURL_HPP
