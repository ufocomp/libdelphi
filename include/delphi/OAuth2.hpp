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

        struct CProvider {
        private:

            void CheckApplication(const CString &Application) const {
                if (Application.IsEmpty())
                    throw COAuth2Error(_T("Application value cannot be empty."));

                if (!ApplicationExists(Application))
                    throw COAuth2Error(_T("Not found application \"%s\" in parameters value."), Application.c_str());
            }

        public:

            CString Name;

            CJSON Params;
            CJSON Keys;

            CDateTime KeyStatusTime;

            enum CKeyStatus {
                ksUnknown = -1,
                ksFetching,
                ksSuccess,
                ksFailed
            } KeyStatus;

            CProvider(): KeyStatus(ksUnknown) {
                KeyStatusTime = Now();
            }

            CProvider(const CProvider &Other): CProvider() {
                if (this != &Other) {
                    this->Name = Other.Name;
                    this->Params = Other.Params;
                    this->Keys = Other.Keys;
                    this->KeyStatusTime = Other.KeyStatusTime;
                    this->KeyStatus = Other.KeyStatus;
                }
            }

            const CJSONObject &Applications() const {
                return Params.Object();
            }

            bool ApplicationExists(const CString& Application) const {
                return Params.HasOwnProperty(Application);
            }

            void GetClients(CStringList &Clients) const {
                const auto &apps = Applications();
                for (int i = 0; i < apps.Count(); i++) {
                    const auto& String = apps.Members(i).String();;
                    Clients.AddPair(ClientId(String), String);
                }
            }

            void GetScopes(const CString &Application, CStringList &Scopes) const {
                const auto& Issuers = Applications()[Application]["scopes"];
                if (Issuers.IsArray()) {
                    for (int i = 0; i < Issuers.Count(); ++i) {
                        Scopes.AddPair(Issuers[i].AsString(), Name);
                    }
                }

                if (Scopes.Count() == 0 && Name == "google") {
                    Scopes.Add("api");
                    Scopes.Add("openid");
                }
            }

            void GetIssuers(const CString &Application, CStringList &Issuers) const {
                const auto& issuers = Applications()[Application]["issuers"];
                if (issuers.IsArray()) {
                    for (int i = 0; i < issuers.Count(); ++i) {
                        Issuers.AddPair(issuers[i].AsString(), Name);
                    }
                }

                if (Issuers.Count() == 0 && Name == "google") {
                    Issuers.AddPair("accounts.google.com", Name);
                    Issuers.AddPair("https://accounts.google.com", Name);
                }
            }

            CString Issuer(const CString &Application, int Index = 0) const {
                CheckApplication(Application);
                CStringList Issuers;
                GetIssuers(Application, Issuers);
                return Issuers[Index];
            }

            CString Algorithm(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["algorithm"].AsString();
            }

            CString ClientId(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["client_id"].AsString();
            }

            CString Secret(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["client_secret"].AsString();
            }

            CString AuthURI(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["auth_uri"].AsString();
            }

            CString TokenURI(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["token_uri"].AsString();
            }

            void RedirectURI(const CString &Application, CStringList &RedirectURIs) const {
                CheckApplication(Application);
                const auto& redirect_uris = Params[Application]["redirect_uris"];
                if (redirect_uris.IsArray()) {
                    for (int i = 0; i < redirect_uris.Count(); ++i) {
                        RedirectURIs.Add(redirect_uris[i].AsString());
                    }
                }
            }

            void JavaScriptOrigins(const CString &Application, CStringList &JavascriptOrigins) const {
                CheckApplication(Application);
                const auto& javascript_origins = Params[Application]["javascript_origins"];
                if (javascript_origins.IsArray()) {
                    for (int i = 0; i < javascript_origins.Count(); ++i) {
                        JavascriptOrigins.Add(javascript_origins[i].AsString());
                    }
                }
            }

            CString CertURI(const CString &Application) const {
                CheckApplication(Application);
                return Params[Application]["auth_provider_x509_cert_url"].AsString();
            }

            CString PublicKey(const CString &KeyId) const {
                if (KeyStatus == ksSuccess)
                    return Keys[KeyId].AsString();
                return CString();
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
                    throw COAuth2Error("Not found Secret for \"%s:%s\"", Provider.Name.c_str(), Application.c_str());
                return Secret;
            };
        }
    }
}

using namespace Delphi::OAuth2;
}

#endif //DELPHI_OAUTH2_HPP
