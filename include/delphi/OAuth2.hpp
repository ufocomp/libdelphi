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
                CString Format("OAuth2 error: ");
                Format << AFormat;
                va_list argList;
                va_start(argList, AFormat);
                FormatMessage(Format.c_str(), argList);
                va_end(argList);
            };

            ~COAuth2Error() override = default;
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CAuthParam ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        struct CAuthParam {
        private:

            mutable CStringList issuers;

            mutable CString algorithm;
            mutable CString client_id;
            mutable CString issuer;
            mutable CString secret;
            mutable CString auth_uri;
            mutable CString token_uri;
            mutable CStringList redirect_uri;
            mutable CString auth_provider_x509_cert_url;

        public:

            CString Provider;

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

            CAuthParam(): Status(ksUnknown) {
                StatusTime = Now();
            }

            CAuthParam(const CAuthParam &Other): CAuthParam() {
                if (this != &Other) {
                    this->Provider = Other.Provider;
                    this->Params = Other.Params;
                    this->Keys = Other.Keys;
                    this->StatusTime = Other.StatusTime;
                    this->Status = Other.Status;
                }
            }

            const CString& Algorithm() const {
                if (algorithm.IsEmpty())
                    algorithm = Params["algorithm"].AsString();
                return algorithm;
            }

            const CString& ClientId() const {
                if (client_id.IsEmpty())
                    client_id = Params["client_id"].AsString();
                return client_id;
            }

            const CString& Issuer() const {
                if (issuer.IsEmpty())
                    issuer = Params["issuers"][0].AsString();
                return issuer;
            }

            const CStringList& GetIssuers() const {
                if (issuers.Count() == 0) {
                    const auto& Issuers = Params["issuers"];
                    if (Issuers.IsArray()) {
                        for (int i = 0; i < Issuers.Count(); ++i) {
                            issuers.AddPair(Issuers[i].AsString(), Provider);
                        }
                    }
                }
                return issuers;
            }

            const CString& Secret() const {
                if (secret.IsEmpty())
                    secret = Params["secret"].AsString();
                return secret;
            }

            const CString& AuthURI() const {
                if (auth_uri.IsEmpty())
                    auth_uri = Params["auth_uri"].AsString();
                return auth_uri;
            }

            const CString& TokenURI() const {
                if (token_uri.IsEmpty())
                    token_uri = Params["token_uri"].AsString();
                return token_uri;
            }

            const CStringList& RedirectURI() const {
                if (redirect_uri.Count() == 0) {
                    const auto& RedirectURI = Params["redirect_uri"];
                    if (RedirectURI.IsArray()) {
                        for (int i = 0; i < RedirectURI.Count(); ++i) {
                            redirect_uri.Add(RedirectURI[i].AsString());
                        }
                    }
                }
                return redirect_uri;
            }

            const CString& CertURI() const {
                if (auth_provider_x509_cert_url.IsEmpty())
                    auth_provider_x509_cert_url = Params["auth_provider_x509_cert_url"].AsString();
                return auth_provider_x509_cert_url;
            }

            CString PublicKey(const CString &KeyId) const {
                if (Keys.IsObject())
                    return Keys[KeyId].AsString();
                return CString();
            }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CAuthParams -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPairs<CAuthParam> CAuthParams;

        //--------------------------------------------------------------------------------------------------------------

        //-- Helper ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        namespace Helper {

            inline void GetClients(const CAuthParams &AuthParams, CStringList &Clients) {
                CAuthParams::ConstEnumerator em(AuthParams);
                while (em.MoveNext()) {
                    Clients.Add(em.Current().Value().ClientId());
                }
            }

            inline void GetIssuers(const CAuthParams &AuthParams, CStringList &Issuers) {
                CAuthParams::ConstEnumerator em(AuthParams);
                while (em.MoveNext()) {
                    Issuers << em.Current().Value().GetIssuers();
                }
            }

            inline int IndexOfClientId(const CAuthParams &AuthParams, const CString &ClientId) {
                CAuthParams::ConstEnumerator em(AuthParams);
                while (em.MoveNext()) {
                    if (em.Current().Value().ClientId() == ClientId)
                        return em.Index();
                }
                return -1;
            }

            inline CString GetPublicKey(const CAuthParams &AuthParams, const CString &KeyId) {
                CString Result;

                auto Value = [&AuthParams, &KeyId, &Result](int Index) {
                    Result = AuthParams[Index].Value().PublicKey(KeyId);
                    return Result.IsEmpty();
                };

                int Index = 0;
                while (Index < AuthParams.Count() && Value(Index)) {
                    Index++;
                }

                if (Index == AuthParams.Count())
                    throw COAuth2Error(_T("Not found public key by id \"%s\" in listed."), KeyId.c_str());

                return Result;
            }
        }
    }
}

using namespace Delphi::OAuth2;
}

#endif //DELPHI_OAUTH2_HPP
