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

            mutable CStringList issuers;
            mutable CStringList scopes;
            mutable CStringList clients;

            mutable CStringPairs algorithm;
            mutable CStringPairs client_id;
            mutable CStringPairs issuer;
            mutable CStringPairs client_secret;
            mutable CStringPairs auth_uri;
            mutable CStringPairs token_uri;
            mutable TPairs<CStringList> redirect_uris;
            mutable CStringPairs auth_provider_x509_cert_url;

            void CheckApplication(const CString &Application) const {
                if (Application.IsEmpty())
                    throw COAuth2Error(_T("Application value cannot be empty."));

                if (!Params.HasOwnProperty(Application))
                    throw COAuth2Error(_T("Not found application \"%s\" in parameters value."), Application.c_str());
            }

        public:

            CString Name;

            CJSON Params;
            CJSON Keys;

            CDateTime StatusTime;

            enum CKeyStatus {
                ksUnknown = -1,
                ksFetching,
                ksSuccess,
                ksError,
                ksSaved
            } Status;

            CProvider(): Status(ksUnknown) {
                StatusTime = Now();
            }

            CProvider(const CProvider &Other): CProvider() {
                if (this != &Other) {
                    this->Name = Other.Name;
                    this->Params = Other.Params;
                    this->Keys = Other.Keys;
                    this->StatusTime = Other.StatusTime;
                    this->Status = Other.Status;
                }
            }

            const CJSONObject &Applications() const {
                return Params.Object();
            }

            const CStringList &GetClients() const {
                if (clients.Count() == 0) {
                    const auto &apps = Applications();
                    for (int i = 0; i < apps.Count(); i++) {
                        const auto& String = apps.Members(i).String();;
                        clients.AddPair(ClientId(String), String);
                    }
                }
                return clients;
            }

            const CStringList& GetScopes(const CString &Application) const {
                if (scopes.Count() == 0) {
                    const auto& Issuers = Applications()[Application]["scopes"];
                    if (Issuers.IsArray()) {
                        for (int i = 0; i < Issuers.Count(); ++i) {
                            scopes.AddPair(Issuers[i].AsString(), Name);
                        }
                    }
                    if (scopes.Count() == 0 && Name == "google") {
                        scopes.Add("api");
                        scopes.Add("openid");
                    }
                }
                return scopes;
            }

            const CStringList& GetIssuers(const CString &Application) const {
                if (issuers.Count() == 0) {
                    const auto& Issuers = Applications()[Application]["issuers"];
                    if (Issuers.IsArray()) {
                        for (int i = 0; i < Issuers.Count(); ++i) {
                            issuers.AddPair(Issuers[i].AsString(), Name);
                        }
                    }
                    if (issuers.Count() == 0 && Name == "google") {
                        issuers.AddPair("accounts.google.com", Name);
                        issuers.AddPair("https://accounts.google.com", Name);
                    }
                }
                return issuers;
            }

            const CString& Issuer(const CString &Application) const {
                CheckApplication(Application);
                if (issuer[Application].IsEmpty()) {
                    issuer.AddPair(Application, GetIssuers(Application).First());
                }
                return issuer[Application].Value();
            }

            const CString& Algorithm(const CString &Application) const {
                CheckApplication(Application);
                if (algorithm[Application].IsEmpty()) {
                    algorithm.AddPair(Application, Params[Application]["algorithm"].AsString());
                }
                return algorithm[Application].Value();
            }

            const CString& ClientId(const CString &Application) const {
                CheckApplication(Application);
                if (client_id[Application].IsEmpty()) {
                    client_id.AddPair(Application, Params[Application]["client_id"].AsString());
                }
                return client_id[Application].Value();
            }

            const CString& Secret(const CString &Application) const {
                CheckApplication(Application);
                if (client_secret[Application].IsEmpty()) {
                    client_secret.AddPair(Application, Params[Application]["client_secret"].AsString());
                }
                return client_secret[Application].Value();
            }

            const CString& AuthURI(const CString &Application) const {
                CheckApplication(Application);
                if (auth_uri[Application].IsEmpty()) {
                    auth_uri.AddPair(Application, Params[Application]["auth_uri"].AsString());
                }
                return auth_uri[Application].Value();
            }

            const CString& TokenURI(const CString &Application) const {
                CheckApplication(Application);
                if (token_uri[Application].IsEmpty()) {
                    token_uri.AddPair(Application, Params[Application]["token_uri"].AsString());
                }
                return token_uri[Application].Value();
            }

            const CStringList& RedirectURI(const CString &Application) const {
                CheckApplication(Application);
                if (redirect_uris[Application].Value().Count() == 0) {
                    const auto& RedirectURI = Params[Application]["redirect_uris"];
                    if (RedirectURI.IsArray()) {
                        for (int i = 0; i < RedirectURI.Count(); ++i) {
                            redirect_uris[Application].Value().Add(RedirectURI[i].AsString());
                        }
                    }
                }
                return redirect_uris[Application].Value();
            }

            const CString& CertURI(const CString &Application) const {
                CheckApplication(Application);
                if (auth_provider_x509_cert_url[Application].IsEmpty()) {
                    auth_provider_x509_cert_url.AddPair(Application, Params[Application]["auth_provider_x509_cert_url"].AsString());
                }
                return auth_provider_x509_cert_url[Application].Value();
            }

            CString PublicKey(const CString &KeyId) const {
                if (Keys.IsObject())
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
                    Issuers << em.Current().Value().GetIssuers(Application);
                }
            }

            inline void GetClients(const CProviders &Providers, CStringList &Clients) {
                CProviders::ConstEnumerator em(Providers);
                while (em.MoveNext()) {
                    Clients << em.Current().Value().GetClients();
                }
            }

            inline int ProviderByClientId(const CProviders &Providers, const CString &ClientId, CString &Application) {
                CProviders::ConstEnumerator em(Providers);
                while (em.MoveNext()) {
                    Application = em.Current().Value().GetClients()[ClientId];
                    if (!Application.IsEmpty()) {
                        return em.Index();
                    }
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
