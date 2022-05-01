/*++

Library name:

  libdelphi

Module Name:

  OAuth2.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_OAUTH2_HPP
#define DELPHI_OAUTH2_HPP

extern "C++" {

namespace Delphi {

    namespace OAuth2 {

        class COAuth2Error : public ExceptionFrm {
            typedef ExceptionFrm inherited;

        public:

            COAuth2Error() : inherited() {};

            explicit COAuth2Error(LPCTSTR AFormat, ...) : inherited() {
                CString Format("OAuth 2.0: ");
                Format << AFormat;
                va_list argList;
                va_start(argList, AFormat);
                FormatMessage(Format.c_str(), argList);
                va_end(argList);
            };

            ~COAuth2Error() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CProvider -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CKeyStatus { ksUnknown = -1, ksFetching, ksSuccess, ksFailed };

        struct CProvider {
        private:

            CString m_Name;

            CJSON m_Params;
            CJSON m_Keys;

            CDateTime m_KeyStatusTime;
            CKeyStatus m_KeyStatus;

            void CheckApplication(const CString &Application) const {
                if (Application.IsEmpty())
                    throw COAuth2Error(_T("Application value cannot be empty."));

                if (!ApplicationExists(Application))
                    throw COAuth2Error(_T("Not found application \"%s\" in parameters value."), Application.c_str());
            }

        public:

            CProvider(): m_KeyStatus(ksUnknown) {
                m_KeyStatusTime = Now();
            }

            explicit CProvider(const CString &ProviderName): CProvider() {
              m_Name = ProviderName;
            }

            CProvider(const CProvider &Other): CProvider() {
                if (this != &Other) {
                    this->m_Name = Other.m_Name;
                    this->m_Params = Other.m_Params;
                    this->m_Keys = Other.m_Keys;
                    this->m_KeyStatusTime = Other.m_KeyStatusTime;
                    this->m_KeyStatus = Other.m_KeyStatus;
                }
            }

            CJSONObject &Applications() {
                return m_Params.Object();
            }

            const CJSONObject &Applications() const {
                return m_Params.Object();
            }

            CJSON &Params() {
                return m_Params;
            }

            const CJSON &Params() const {
                return m_Params;
            }

            CJSON &Keys() {
                return m_Keys;
            }

            const CJSON &Keys() const {
                return m_Keys;
            }

            CDateTime KeyStatusTime() const { return m_KeyStatusTime; }
            void KeyStatusTime(CDateTime Value) { m_KeyStatusTime = Value; }

            CKeyStatus KeyStatus() const { return m_KeyStatus; }
            void KeyStatus(CKeyStatus Value) { m_KeyStatus = Value; }

            CString &Name() { return m_Name; };
            const CString &Name() const { return m_Name; };

            bool ApplicationExists(const CString& Application) const {
                return m_Params.HasOwnProperty(Application);
            }

            void GetClients(CStringList &Clients) const {
                const auto &apps = Applications();
                for (int i = 0; i < apps.Count(); i++) {
                    const auto &String = apps.Members(i).String();;
                    Clients.AddPair(ClientId(String), String);
                }
            }

            void GetScopes(const CString &Application, CStringList &Scopes) const {
                const auto &scopes = Applications()[Application]["scopes"];
                if (scopes.IsArray()) {
                    for (int i = 0; i < scopes.Count(); ++i) {
                        Scopes.AddPair(scopes[i].AsString(), m_Name);
                    }
                }

                if (Scopes.Count() == 0 && m_Name == "google") {
                    Scopes.Add("api");
                    Scopes.Add("openid");
                }
            }

            void GetIssuers(const CString &Application, CStringList &Issuers) const {
                const auto &issuers = Applications()[Application]["issuers"];
                if (issuers.IsArray()) {
                    for (int i = 0; i < issuers.Count(); ++i) {
                        Issuers.AddPair(issuers[i].AsString(), m_Name);
                    }
                }

                if (Issuers.Count() == 0 && m_Name == "google") {
                    Issuers.AddPair("accounts.google.com", m_Name);
                    Issuers.AddPair("https://accounts.google.com", m_Name);
                }
            }

            CString Issuer(const CString &Application, int Index = 0) const {
                CheckApplication(Application);
                CStringList Issuers;
                GetIssuers(Application, Issuers);
                return Issuers.Names(Index);
            }

            CString Type(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["type"].AsString();
            }

            CString Algorithm(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["algorithm"].AsString();
            }

            CString ClientId(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["client_id"].AsString();
            }

            CString Secret(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["client_secret"].AsString();
            }

            CString AuthURI(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["auth_uri"].AsString();
            }

            CString TokenURI(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["token_uri"].AsString();
            }

            void RedirectURI(const CString &Application, CStringList &RedirectURIs) const {
                CheckApplication(Application);
                const auto &redirect_uris = Applications()[Application]["redirect_uris"];
                if (redirect_uris.IsArray()) {
                    for (int i = 0; i < redirect_uris.Count(); ++i) {
                        RedirectURIs.Add(redirect_uris[i].AsString());
                    }
                }
            }

            void JavaScriptOrigins(const CString &Application, CStringList &JavascriptOrigins) const {
                CheckApplication(Application);
                const auto &javascript_origins = Applications()[Application]["javascript_origins"];
                if (javascript_origins.IsArray()) {
                    for (int i = 0; i < javascript_origins.Count(); ++i) {
                        JavascriptOrigins.Add(javascript_origins[i].AsString());
                    }
                }
            }

            CString CertURI(const CString &Application) const {
                CheckApplication(Application);
                return Applications()[Application]["auth_provider_x509_cert_url"].AsString();
            }

            CString PublicKey(const CString &KeyId) const {
                if (m_KeyStatus == ksSuccess)
                    return m_Keys[KeyId].AsString();
                return {};
            }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CProviders ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPairs<CProvider> CProviders;

        //--------------------------------------------------------------------------------------------------------------

        //-- Helper ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace Helper {

            inline void GetIssuers(const CProviders &Providers, const CString &Application, CStringList &Issuers) {
                CProviders::ConstEnumerator em(Providers);
                while (em.MoveNext()) {
                    em.Current().Value().GetIssuers(Application, Issuers);
                }
            }

            inline void GetClients(const CProviders &Providers, CStringList &Clients) {
                CProviders::ConstEnumerator em(Providers);
                while (em.MoveNext()) {
                    em.Current().Value().GetClients(Clients);
                }
            }

            inline int ProviderByClientId(const CProviders &Providers, const CString &ClientId, CString &Application) {
                CStringList Clients;
                CProviders::ConstEnumerator em(Providers);
                Application.Clear();
                while (em.MoveNext()) {
                    em.Current().Value().GetClients(Clients);
                    Application = Clients[ClientId];
                    if (!Application.IsEmpty())
                        return em.Index();
                    Clients.Clear();
                }
                return -1;
            }

            inline CString GetPublicKey(const CProviders &Providers, const CString &KeyId) {
                CString Result;

                auto Value = [&Providers, &KeyId, &Result](int Index) {
                    Result = Providers[Index].Value().PublicKey(KeyId);
                    return Result.IsEmpty();
                };

                int Index = 0;
                while (Index < Providers.Count() && Value(Index)) {
                    Index++;
                }

                if (Index == Providers.Count())
                    throw COAuth2Error(_T("Not found public key by id \"%s\" in listed."), KeyId.c_str());

                return Result;
            }

            inline CString GetSecret(const CProvider &Provider, const CString &Application) {
                const auto &Secret = Provider.Secret(Application);
                if (Secret.IsEmpty())
                    throw COAuth2Error("Not found Secret for \"%s:%s\"", Provider.Name().c_str(), Application.c_str());
                return Secret;
            };
        }
    }
}

using namespace Delphi::OAuth2;
}

#endif //DELPHI_OAUTH2_HPP
