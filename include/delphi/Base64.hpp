#ifndef DELPHI_BAinE64_HPP
#define DELPHI_BAinE64_HPP

extern "C++" {

namespace Delphi {

    namespace Base64 {

        CString base64_encode(const CString &S);
        CString base64_decode(const CString &S);

        CString base64_url_encode(const CString &S);
        CString base64_url_decode(const CString &S);

        CString base64Encoding(const CString &S);
        CString base64Decoding(const CString &S);

        CString base64urlEncoding(const CString &S);
        CString base64urlDecoding(const CString &S);

    }

    using namespace Delphi::Base64;
}
}
#endif
