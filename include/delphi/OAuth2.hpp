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
            mutable CStringList clients;

            mutable CString algorithm;
            mutable CString client_id;
            mutable CString issuer;
            mutable CString client_secret;
            mutable CString auth_uri;
            mutable CString token_uri;
            mutable CStringList redirect_uris;
            mutable CString auth_provider_x509_cert_url;

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
                if (issuer.IsEmpty()) {
                    issuer = GetIssuers(Application).First();
                }
                return issuer;
            }

            const CString& Algorithm(const CString &Application) const {
                if (algorithm.IsEmpty()) {
                    CheckApplication(Application);
                    algorithm = Params[Application]["algorithm"].AsString();
                }
                return algorithm;
            }

            const CString& ClientId(const CString &Application) const {
                if (client_id.IsEmpty()) {
                    CheckApplication(Application);
                    client_id = Params[Application]["client_id"].AsString();
                }
                return client_id;
            }

            const CString& Secret(const CString &Application) const {
                if (client_secret.IsEmpty()) {
                    CheckApplication(Application);
                    client_secret = Params[Application]["client_secret"].AsString();
                }
                return client_secret;
            }

            const CString& AuthURI(const CString &Application) const {
                if (auth_uri.IsEmpty()) {
                    CheckApplication(Application);
                    auth_uri = Params[Application]["auth_uri"].AsString();
                }
                return auth_uri;
            }

            const CString& TokenURI(const CString &Application) const {
                if (token_uri.IsEmpty()) {
                    CheckApplication(Application);
                    token_uri = Params[Application]["token_uri"].AsString();
                }
                return token_uri;
            }

            const CStringList& RedirectURI(const CString &Application) const {
                if (redirect_uris.Count() == 0) {
                    CheckApplication(Application);
                    const auto& RedirectURI = Params[Application]["redirect_uris"];
                    if (RedirectURI.IsArray()) {
                        for (int i = 0; i < RedirectURI.Count(); ++i) {
                            redirect_uris.Add(RedirectURI[i].AsString());
                        }
                    }
                }
                return redirect_uris;
            }

            const CString& CertURI(const CString &Application) const {
                if (auth_provider_x509_cert_url.IsEmpty()) {
                    CheckApplication(Application);
                    auth_provider_x509_cert_url = Params[Application]["auth_provider_x509_cert_url"].AsString();
                }
                return auth_provider_x509_cert_url;
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

            inline const CProvider &ProviderByClientId(const CProviders &Providers, const CString &ClientId, CString &Application) {
                CProviders::ConstEnumerator em(Providers);
                while (em.MoveNext()) {
                    Application = em.Current().Value().GetClients()[ClientId];
                    if (!Application.IsEmpty()) {
                        return em.Current().Value();
                    }
                }
                throw COAuth2Error(_T("Not found provider by Client ID."));
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
        }
    }
}

using namespace Delphi::OAuth2;
}

#endif //DELPHI_OAUTH2_HPP
