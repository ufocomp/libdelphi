/*++

Library name:

  libdelphi

Module Name:

  libdelphi.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#pragma once
//----------------------------------------------------------------------------------------------------------------------

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <climits>
#include <ctime>
#include <csignal>
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <syscall.h>
#include <pwd.h>
#include <grp.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

#ifdef _GLIBCXX_FUNCTIONAL
using namespace std::placeholders;
#endif

#define MAX_BUFFER_SIZE 255
#define MAX_STRING_LEN  2048
//----------------------------------------------------------------------------------------------------------------------

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
#define chARRAY ARRAYSIZE
//----------------------------------------------------------------------------------------------------------------------

typedef int SOCKET;
//----------------------------------------------------------------------------------------------------------------------

#define INFINITE (-1)
//----------------------------------------------------------------------------------------------------------------------

#define _INT32_LEN   (sizeof("-2147483648") - 1)
#define _INT64_LEN   (sizeof("-9223372036854775808") - 1)

#if (__WORDSIZE == 64)
#define _INT_T_LEN        _INT64_LEN
#define _MAX_INT_T_VALUE  9223372036854775807
#else
#define _INT_T_LEN        _INT32_LEN
#define _MAX_INT_T_VALUE  2147483647
#endif
//----------------------------------------------------------------------------------------------------------------------

#define _MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define _MAX_INT32_VALUE   (uint32_t) 0x7fffffff

//----------------------------------------------------------------------------------------------------------------------

#ifndef UNICODE
#ifdef _UNICODE
#define UNICODE
#endif
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifdef _UNICODE
#define tistream std::wistream
#define tostream std::wostream
#else
#define tistream std::istream
#define tostream std::ostream
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifndef LF
#define LF '\n'
#endif
//----------------------------------------------------------------------------------------------------------------------

#define linefeed(p)          *(p)++ = LF;
#define LINEFEED_SIZE        1
#define LINEFEED             "\x0a"
//----------------------------------------------------------------------------------------------------------------------

#define path_separator(c)    ((c) == '/')
//----------------------------------------------------------------------------------------------------------------------

typedef intptr_t        int_t;
typedef uintptr_t       uint_t;
//----------------------------------------------------------------------------------------------------------------------

#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifdef _UNICODE
#define __TEXT(x) L##x
#else
#define __TEXT(x) x
#endif

#define TEXT(x) __TEXT(x)
//----------------------------------------------------------------------------------------------------------------------

#ifdef  _UNICODE
#define __T(x)      L ## x
#else   /* ndef _UNICODE */
#define __T(x)      x
#endif

#define _T(x) __T(x)
//----------------------------------------------------------------------------------------------------------------------

typedef char CHAR;
typedef wchar_t WCHAR;
//----------------------------------------------------------------------------------------------------------------------

typedef char* PSTR, *LPSTR;
typedef const char* LPCSTR;
typedef const WCHAR* LPCWSTR;
typedef unsigned char BYTE, *PBYTE, *LPBYTE;
typedef const unsigned char *LPCBYTE;
typedef char* PSTR, *LPSTR;
typedef wchar_t* LPWSTR, *PWSTR;
//----------------------------------------------------------------------------------------------------------------------

typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;
//----------------------------------------------------------------------------------------------------------------------

typedef void VOID, *PVOID, *LPVOID;
typedef const void* LPCVOID;
//----------------------------------------------------------------------------------------------------------------------

#define TRUE 1
#define FALSE 0
//----------------------------------------------------------------------------------------------------------------------

typedef int BOOL;
typedef unsigned short WORD, *PWORD, *LPWORD;
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
typedef double DOUBLE;
typedef int INT, *LPINT;
typedef unsigned int UINT;
typedef long LONG, *PLONG, *LPLONG;
//----------------------------------------------------------------------------------------------------------------------

#define abstract = 0
//----------------------------------------------------------------------------------------------------------------------

typedef void *Pointer;
typedef void *HANDLE;
typedef LONG HRESULT;
//----------------------------------------------------------------------------------------------------------------------

#define S_OK	0x0
#define S_FALSE	0x1

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
//----------------------------------------------------------------------------------------------------------------------

#ifndef Assigned
#  define Assigned(Obj) ((Obj) != nullptr)
#endif
//----------------------------------------------------------------------------------------------------------------------

#define FreeAndNil(Obj) {       \
    if ( (Obj) != nullptr ) {   \
        delete (Obj);           \
        (Obj) = nullptr;        \
   }                            \
}
//----------------------------------------------------------------------------------------------------------------------

#ifndef Min
#  define Min(A, B) (((A) < (B)) ? (A) : (B))
#endif
//----------------------------------------------------------------------------------------------------------------------

#ifndef Max
#  define Max(A, B) (((A) > (B)) ? (A) : (B))
#endif
//----------------------------------------------------------------------------------------------------------------------

inline void chASSERTFAIL(LPCSTR file, int line, LPCSTR expr);
inline void chMBANDDEBUG(LPCSTR szMsg);
inline void chMB(LPCSTR szMsg);
//----------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#  define chASSERT(x) if (!(x)) chASSERTFAIL(__FILE__, __LINE__, #x)
#else
#  define chASSERT(x)
#endif

// Assert in debug builds, but don't remove the code in retail builds.
#ifdef _DEBUG
#  define chVERIFY(x) chASSERT(x)
#else
#  define chVERIFY(x) (x)
#endif
//----------------------------------------------------------------------------------------------------------------------

#define DELPHI_LIB_NAME         _T("libdelphi")
#define DELPHI_LIB_DESCRIPTION  _T("Delphi classes for C++")
#define DELPHI_LIB_VERSION      _T("2.9.2")
#define DELPHI_LIB_VER          DELPHI_LIB_NAME _T("/") DELPHI_LIB_VERSION
//----------------------------------------------------------------------------------------------------------------------

#define SYS_ERRNO_COUNT         135

#ifdef DELPHI_LIB_EXPORTS
#  define LIB_DELPHI

#  ifndef DELPHI_SYSUTILS_HPP
#    include "delphi/SysUtils.hpp"
#  endif

#  ifndef DELPHI_STRINGS_HPP
#    include "delphi/Strings.hpp"
#  endif

#  ifndef DELPHI_CLASSES_HPP
#    include "delphi/Classes.hpp"
#  endif

#  ifndef DELPHI_BASE64_HPP
#    include "delphi/Base64.hpp"
#  endif

#  ifndef DELPHI_VARIANT_HPP
#    include "delphi/Variant.hpp"
#  endif

#  ifndef DELPHI_EXCEPTION_HPP
#    include "delphi/Exceptions.hpp"
#  endif

#  ifndef DELPHI_TEMPLATES_HPP
#    include "delphi/Templates.hpp"
#  endif

#  ifndef DELPHI_INIFILES_HPP
#    include "delphi/IniFiles.hpp"
#  endif

#  ifndef DELPHI_PAIR_HPP
#    include "delphi/Pair.hpp"
#  endif

#  ifndef DELPHI_JSON_HPP
#    include "delphi/JSON.hpp"
#  endif

#  ifndef DELPHI_SOCKETS_HPP
#    include "delphi/Sockets.hpp"
#  endif

#  ifndef DELPHI_QUEUE_HPP
#    include "delphi/Queue.hpp"
#  endif

#  ifndef DELPHI_OAUTH2_HPP
#    include "delphi/OAuth2.hpp"
#  endif

#  ifndef DELPHI_HTTP_HPP
#    include "delphi/HTTP.hpp"
#  endif

#  ifndef DELPHI_MESSAGE_HPP
#    include "delphi/Message.hpp"
#  endif

#  ifndef DELPHI_SMTP_HPP
#    include "delphi/SMTP.hpp"
#  endif

#  ifndef DELPHI_WSPROTOCOL_HPP
#    include "delphi/WSProtocol.hpp"
#  endif

#ifdef WITH_CURL
#  ifndef DELPHI_CURL_HPP
#    include "delphi/CURL.hpp"
#  endif
#endif

#ifdef WITH_POSTGRESQL
#  ifndef DELPHI_POSTGRES_HPP
#    include "delphi/Postgres.hpp"
#  endif
#endif

#ifdef WITH_SQLITE
#  ifndef DELPHI_SQLLITE_HPP
#    include "delphi/SQLite.hpp"
#  endif
#endif

#  ifndef DELPHI_SIGANL_HPP
#    include "delphi/Signal.hpp"
#  endif

#  ifndef DELPHI_PROCESS_HPP
#    include "delphi/Process.hpp"
#  endif

#  ifndef DELPHI_APPLICATION_HPP
#    include "delphi/Application.hpp"
#  endif

using namespace Delphi;
//----------------------------------------------------------------------------------------------------------------------

#else
#  define LIB_DELPHI
#endif
//----------------------------------------------------------------------------------------------------------------------

inline void chMB(LPCSTR szMsg)
{
    DebugMessage(szMsg);
}
//----------------------------------------------------------------------------------------------------------------------

inline void chMBANDDEBUG(LPCSTR szMsg)
{
    chMB(szMsg);
    abort();
}
//----------------------------------------------------------------------------------------------------------------------

// Put up an assertion failure message box.
inline void chASSERTFAIL(LPCSTR file, int line, LPCSTR expr)
{
    TCHAR sz[MAX_BUFFER_SIZE] = {0};
    StringCchPrintf(sz, chARRAY(sz), _T("File %s, line %d : %s\n"), file, line, expr);
    chMBANDDEBUG(sz);
}
//----------------------------------------------------------------------------------------------------------------------
