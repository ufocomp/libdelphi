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

#define strcase(code) case code: s = __STRING(code)

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
            m_SList = nullptr;

            m_bTunnel = false;

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
            Reset();
            if (m_curl != nullptr) {
                curl_easy_cleanup(m_curl);
            }
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

            if (m_SList != nullptr) {
                curl_slist_free_all(m_SList);
                m_SList = nullptr;
            }

            if (m_curl != nullptr) {
                curl_easy_reset(m_curl);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlApi::Prepare(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const {
            Reset();

            if (m_curl != nullptr) {
                curl_easy_setopt(m_curl, CURLOPT_PRIVATE, this);
                curl_easy_setopt(m_curl, CURLOPT_URL, URL.href().c_str());
                curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, CCurlApi::HeaderCallBack);
                curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_Headers);
                curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CCurlApi::WriteCallBack);
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_Result);
                curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, m_TimeOut);
                curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(m_curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");
                curl_easy_setopt(m_curl, CURLOPT_HTTP_CONTENT_DECODING, 1L);
                curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_Error);
                curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, 3L);
                curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, 10L);

                if (m_bTunnel) {
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
                        m_SList = curl_slist_append(m_SList, List[i].c_str());
                    }

                    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_SList);
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
                        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, Method.c_str());
                    }
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Perform() const {
            CURLcode code = CURLE_INTERFACE_FAILED;

            if (m_curl != nullptr) {
                code = curl_easy_perform(m_curl);

                if (m_SList != nullptr) {
                    curl_slist_free_all(m_SList);
                    m_SList = nullptr;
                }

                if (code == CURLE_OK) {
                    CurlInfo();
                }
            }

            return code;
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Get(const CLocation &URL, const CHeaders &Headers) const {
            Prepare(URL, "GET", CString(), Headers);
            return Perform();
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Post(const CLocation &URL, const CString &Content, const CHeaders &Headers) const {
            Prepare(URL, "POST", Content, Headers);
            return Perform();
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLcode CCurlApi::Send(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const {
            Prepare(URL, Method, Content, Headers);
            return Perform();
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlFetch ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCurlFetch::CCurlFetch(): CCurlApi() {
            m_OnDone = nullptr;
            m_OnFail = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlFetch::CurlInfo() const {
            char *url = nullptr;

            TCHAR szString[_INT_T_LEN + 1] = {0};

            long response_code;

            m_Into.Clear();

            if (m_curl != nullptr) {
                curl_easy_getinfo(m_curl, CURLINFO_EFFECTIVE_URL, &url);
                curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);

                m_Into.AddPair("CURLINFO_EFFECTIVE_URL", url);
                m_Into.AddPair("CURLINFO_RESPONSE_CODE", IntToStr((int) response_code, szString, _INT_T_LEN));
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CCurlFetch::GetResponseCode() const {
            const auto &code = m_Into["CURLINFO_RESPONSE_CODE"];
            return code.empty() ? 0 : (int) StrToInt(code.c_str());
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlFetch::DoDone(CURLcode code) {
            if (m_OnDone != nullptr) {
                m_OnDone(this, code, Error());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlFetch::DoFail(CURLcode code) {
            if (m_OnFail != nullptr) {
                m_OnFail(this, code, Error());
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCurlAsyncFetch -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCurlAsyncFetch::CCurlAsyncFetch(): CCurlFetch() {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CCurlAsyncFetch::Prepare(const CLocation &URL, const CString &Method, const CString &Content, const CHeaders &Headers) const {
            m_Content = Content;
            CCurlFetch::Prepare(URL, Method, m_Content, Headers);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CCURLClient -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CCURLClient::CCURLClient(): CEPoll() {
            m_Handle = curl_multi_init();
            m_pTimer = nullptr;

            m_Action = 0;
            m_StillRunning = 0;
            m_TimeOut = 0;

            m_OnException = nullptr;

            curl_multi_setopt(m_Handle, CURLMOPT_SOCKETFUNCTION, CCURLClient::SocketCallBack);
            curl_multi_setopt(m_Handle, CURLMOPT_SOCKETDATA, this);
            curl_multi_setopt(m_Handle, CURLMOPT_TIMERFUNCTION, CCURLClient::MultiTimerCallBack);
            curl_multi_setopt(m_Handle, CURLMOPT_TIMERDATA, this);
        }
        //--------------------------------------------------------------------------------------------------------------

        CCURLClient::~CCURLClient() {
            curl_multi_cleanup(m_Handle);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::InitEventHandler(CPollEventHandler *AHandler) {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            AHandler->OnTimeOutEvent([this](auto && AHandler) { DoTimeOut(AHandler); });
            AHandler->OnEvent([this](auto && AHandler, auto && events) { DoEvent(AHandler, events); });
            AHandler->OnErrorEvent([this](auto && AHandler) { DoError(AHandler); });
#else
            AHandler->OnTimeOutEvent(std::bind(&CCURLClient::DoTimeOut, this, _1));
            AHandler->OnEvent(std::bind(&CCURLClient::DoEvent, this, _1, _2));
            AHandler->OnErrorEvent(std::bind(&CCURLClient::DoError, this, _1));
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        CPollEventHandler *CCURLClient::GetEventHandler(CSocket Socket) {
            auto pEventHandler = EventHandlers()->Add(Socket);
            InitEventHandler(pEventHandler);
            return pEventHandler;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::AddSocket(CCURLClient *AClient, curl_socket_t s, int action) {
            auto pEventHandler = AClient->GetEventHandler(s);
            SetSocket(pEventHandler, action);
            curl_multi_assign(AClient->Handle(), s, pEventHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::SetSocket(CPollEventHandler *AHandler, int action) {
            uint32_t events = ((action & CURL_POLL_IN) ? EPOLLIN : 0) | ((action & CURL_POLL_OUT) ? EPOLLOUT : 0);
            AHandler->Start(etEvent, events);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::RemoveSocket(CPollEventHandler *AHandler) {
            if (AHandler->Socket() != INVALID_SOCKET) {
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CCURLClient::SocketCallBack(CURL *curl, curl_socket_t s, int what, CCURLClient *AClient, CPollEventHandler *AHandler) {
            if (what == CURL_POLL_REMOVE) {
                CCURLClient::RemoveSocket(AHandler);
            } else {
                if (!AHandler) {
                    CCURLClient::AddSocket(AClient, s, what);
                } else {
                    CCURLClient::SetSocket(AHandler, what);
                }
            }

            return 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CCURLClient::MultiTimerCallBack(CURLM *multi, long timeout_ms, CCURLClient *AClient) {
            AClient->UpdateTimer(timeout_ms);
            return 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::ErrorCheck(const char *where, CURLMcode code) {
            if (CURLM_OK != code) {
                const char *s;
                switch (code) {
                    strcase(CURLM_BAD_HANDLE);
                        break;
                    strcase(CURLM_BAD_EASY_HANDLE);
                        break;
                    strcase(CURLM_OUT_OF_MEMORY);
                        break;
                    strcase(CURLM_INTERNAL_ERROR);
                        break;
                    strcase(CURLM_UNKNOWN_OPTION);
                        break;
                    strcase(CURLM_ADDED_ALREADY);
                        break;
                    strcase(CURLM_RECURSIVE_API_CALL);
                        break;
                    strcase(CURLM_WAKEUP_FAILURE);
                        break;
                    strcase(CURLM_BAD_FUNCTION_ARGUMENT);
                        break;
                    strcase(CURLM_ABORTED_BY_CALLBACK);
                        break;
                    strcase(CURLM_UNRECOVERABLE_POLL);
                        break;
                    strcase(CURLM_LAST);
                        break;
                    default:
                        s = "CURLM_unknown";
                        break;
                    strcase(CURLM_BAD_SOCKET);
                        /* ignore this error */
                        return;
                }

                throw Delphi::Exception::ExceptionFrm("[CURLClient]: %s returns %d: %s", where, code, s);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::UpdateTimer(long int Value, long int Interval) {
            if (m_pTimer == nullptr) {
                m_pTimer = CEPollTimer::CreateTimer(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
                m_pTimer->AllocateTimer(m_pEventHandlers, Value, Interval);
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
                m_pTimer->OnTimer([this](auto && AHandler) { DoTimer(AHandler); });
#else
                m_pTimer->OnTimer(std::bind(&CCURLClient::DoTimer, this, _1));
#endif
            } else {
                m_pTimer->SetTimer(Value, Interval);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::MultiInfo() {
            CCurlAsyncFetch *pFetch;
            CURLMsg *msg;
            int msgs_left;
            CURL *easy;
            CURLcode code;

            while ((msg = curl_multi_info_read(m_Handle, &msgs_left))) {
                if (msg->msg == CURLMSG_DONE) {
                    easy = msg->easy_handle;
                    code = msg->data.result;

                    curl_easy_getinfo(easy, CURLINFO_PRIVATE, &pFetch);

                    curl_multi_remove_handle(m_Handle, easy);

                    pFetch->CurlInfo();

                    if (code == CURLE_OK) {
                        pFetch->DoDone(code);
                    } else {
                        pFetch->DoFail(code);
                    }

                    delete pFetch;
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::DoTimer(CPollEventHandler *AHandler) {
            uint64_t exp;

            auto pTimer = dynamic_cast<CEPollTimer *> (AHandler->Binding());
            pTimer->Read(&exp, sizeof(uint64_t));

            try {
                const auto code = curl_multi_socket_action(m_Handle, CURL_SOCKET_TIMEOUT, 0, &m_StillRunning);
                ErrorCheck("Timer", code);
                MultiInfo();
            } catch (Delphi::Exception::Exception &E) {
                DoException(E);
                AHandler->Stop();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::DoTimeOut(CPollEventHandler *AHandler) {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::DoEvent(CPollEventHandler *AHandler, uint32_t events) {
            const int action = ((events & EPOLLIN) ? CURL_CSELECT_IN : 0) | ((events & EPOLLOUT) ? CURL_CSELECT_OUT : 0);

            try {
                const auto code = curl_multi_socket_action(m_Handle, AHandler->Socket(), action, &m_StillRunning);
                ErrorCheck("Event", code);
                MultiInfo();
            } catch (Delphi::Exception::Exception &E) {
                DoException(E);
            }

            if (m_StillRunning <= 0) {
                FreeAndNil(m_pTimer);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CURLMcode CCURLClient::Perform(const CLocation &URL, const CString &Method, const CString &Content,
                const CHeaders &Headers, COnCurlFetchEvent && OnDone, COnCurlFetchEvent && OnFail) {

            auto pFetch = new CCurlAsyncFetch();

            pFetch->TimeOut(m_TimeOut);
            pFetch->OnDone() = OnDone;
            pFetch->OnFail() = OnFail;

            pFetch->Prepare(URL, Method, Content, Headers);

            const auto code = curl_multi_add_handle(m_Handle, pFetch->Handle());
            try {
                ErrorCheck("Perform", code);
                UpdateTimer(0);
            } catch (Delphi::Exception::Exception &E) {
                DoException(E);
                delete pFetch;
            }

            return code;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CCURLClient::DoException(const Exception::Exception &E) {
            if (m_OnException != nullptr) {
                m_OnException(this, E);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

    }
}
}
