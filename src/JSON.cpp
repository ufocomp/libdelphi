/*++

Library name:

  libdelphi

Module Name:

  JSON.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/JSON.hpp"

#define JSON_INVALID_VALUE_TYPE "Invalid JSON value type."

extern "C++" {

namespace Delphi {

    namespace Json {

        CString EncodeJsonString(const CString &String) {
            CString Result;
            TCHAR ch;
            for (size_t Index = 0; Index < String.Size(); Index++) {
                ch = String.at(Index);
                switch (ch) {
                    case '\r':
                        Result.Append("\\r");
                        break;
                    case '\n':
                        Result.Append("\\n");
                        break;
                    case '\t':
                        Result.Append("\\t");
                        break;
                    case '"':
                    case '\\':
                        Result.Append("\\");
                        Result.Append(ch);
                        break;
                    default:
                        Result.Append(ch);
                        break;
                }
            }
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString DecodeJsonString(const CString &String) {
            CString Result;
            size_t Index = 0;

            if (!String.IsEmpty()) {
                TCHAR ch = String.at(Index);
                while (ch != 0) {
                    if (ch == '\\') {
                        ch = String.at(++Index);
                        switch (ch) {
                            case 'r':
                                Result.Append('\r');
                                break;
                            case 'n':
                                Result.Append('\n');
                                break;
                            case 't':
                                Result.Append('\t');
                                break;
                            case '"':
                                Result.Append('"');
                                break;
                            case '\\':
                                Result.Append('\\');
                                break;
                            default:
                                Result.Append(ch);
                                break;
                        }
                    } else {
                        Result.Append(ch);
                    }

                    ch = String.at(++Index);
                }
            }

            return Result;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSON -----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CJSON::CJSON(): CJSON(this, jvtNull) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON::CJSON(CJSONValueType ValueType): CJSON(this, ValueType) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON::CJSON(CPersistent *AOwner): CJSON(AOwner, jvtNull) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON::CJSON(const CString &AString): CJSON() {
            StringToJson(AString);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON::CJSON(CPersistent *AOwner, CJSONValueType ValueType): CPersistent(AOwner), m_ValueType(ValueType) {
            m_Value = nullptr;
            m_UpdateCount = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON::~CJSON() {
            if (this != m_Value)
                delete m_Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::SetUpdateState(bool Updating) {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::BeginUpdate() {
            if (m_UpdateCount == 0)
                SetUpdateState(true);
            m_UpdateCount++;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::EndUpdate() {
            m_UpdateCount--;
            if (m_UpdateCount == 0)
                SetUpdateState(false);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject *CJSON::GetObject() {
            m_ValueType = jvtObject;
            if (m_Value == nullptr)
                m_Value = new CJSONObject(this);
            return dynamic_cast<CJSONObject *> (m_Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray *CJSON::GetArray() {
            m_ValueType = jvtArray;
            if (m_Value == nullptr)
                m_Value = new CJSONArray(this);
            return dynamic_cast<CJSONArray *> (m_Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Assign(const CJSON &Source) {
            Clear();
            m_ValueType = Source.ValueType();

            if (Assigned(Source.Value())) {
                if (Source.Value()->IsObject())
                    GetObject()->Assign(Source.Object());

                if (Source.Value()->IsArray())
                    GetArray()->Assign(Source.Array());
            } else {
                if (m_ValueType == jvtObject) {
                    GetObject()->Assign((const CJSONObject &) Source);
                } else if (m_ValueType == jvtArray) {
                    GetArray()->Assign((const CJSONArray &) Source);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Concat(const CJSON& Source) {
            if (Assigned(Source.Value()) && (ValueType() == Source.Value()->ValueType())) {
                if (Source.Value()->IsObject())
                    GetObject()->Concat(Source.Object());

                if (Source.Value()->IsArray())
                    GetArray()->Concat(Source.Array());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSON::HasOwnProperty(const CString &String) const {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject())
                    return Object().HasOwnProperty(String);
                if (m_Value->IsArray())
                    return Array().HasOwnProperty(String);
            }
            return false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Delete(const CString &Value) {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject()) {
                    int Index = Object().IndexOfString(Value);
                    if (Index != -1)
                        Object().Delete(Index);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Delete(reference Value) {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject()) {
                    int Index = Object().IndexOfString(Value);
                    if (Index != -1)
                        Object().Delete(Index);
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Delete(int Index) {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject())
                    Object().Delete(Index);

                if (m_Value->IsArray())
                    Array().Delete(Index);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSON::GetCount() const noexcept {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject())
                    return Object().Count();

                if (m_Value->IsArray())
                    return Array().Count();
            }

            return 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Clear() {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject())
                    return Object().Clear();

                if (m_Value->IsArray())
                    return Array().Clear();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CJSON::JsonToString() const {
            CString S;

            if (ValueType() == jvtObject) {
                S = "{";

                for (int i = 0; i < Count(); i++) {

                    if (i > 0) {
                        S += ", ";
                    }

                    S += "\"";
                    S += Members(i).String();
                    S += "\": ";

                    S << Object()[i].ToString();
                }

                S += "}";
            }

            if (IsArray()) {

                S = "[";

                for (int i = 0; i < Count(); i++) {

                    if (i > 0) {
                        S += ", ";
                    }

                    S << Array()[i].ToString();
                }

                S += "]";
            }

            return S;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::StringToJson(const CString &Value) {
            if (!Value.IsEmpty())
              StrToJson(Value.c_str(), Value.Size());
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSON::JsonToStr(LPTSTR ABuffer, size_t &ASize) {
            const CString& S = JsonToString();
            size_t Size = ASize;
            ASize = S.Size();
            if (Size >= ASize) {
                ::CopyMemory(ABuffer, (LPTSTR) S.Data(), ASize);
                Size = ASize;
            }
            return (ASize == Size);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::StrToJson(LPCTSTR ABuffer, size_t ASize) {

            CJSONParser pParser(this);
            CJSONParserResult R;

            BeginUpdate();
            try {
                if (Assigned(ABuffer)) {
                    Clear();
                    R = pParser.Parse((LPTSTR) ABuffer, ABuffer + ASize);
                    if (!R.result) {
                        throw Exception::EJSONParseSyntaxError(_T("JSON Parser syntax error in position %d, char: %#x"), R.pos, ABuffer[R.pos]);
                    }
                }
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }

        //--------------------------------------------------------------------------------------------------------------

        void CJSON::LoadFromFile(LPCTSTR lpszFileName) {
            CFileStream Stream(lpszFileName, O_RDONLY);
            LoadFromStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::LoadFromStream(const CStream &Stream) {

            size_t BufSize, Count;
            LPTSTR Buffer;

            Count = Stream.Size() - Stream.Position();

            if (Count > MaxBufSize)
                BufSize = MaxBufSize;
            else
                BufSize = Count;

            const auto char_size = sizeof(TCHAR);

            Buffer = (LPTSTR) GHeap->Alloc(HEAP_ZERO_MEMORY, BufSize + char_size);
            BeginUpdate();
            try {
                Stream.Read(Buffer, BufSize);
                StrToJson(Buffer, BufSize);
            } catch (...) {
                GHeap->Free(0, Buffer, BufSize + char_size);
                EndUpdate();
                throw;
            }
            GHeap->Free(0, Buffer, BufSize + char_size);
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::SaveToFile(LPCTSTR lpszFileName) const {
            CFileStream Stream(lpszFileName, OF_CREATE);
            SaveToStream(Stream);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::SaveToStream(CStream &Stream) const {
            const CString& S = JsonToString();
            Stream.WriteBuffer(S.Data(), S.Size());
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONMember &CJSON::Members(int Index) {
            return Object().Members(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONMember &CJSON::Members(int Index) const {
            return Object().Members(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSON::Members(int Index, const CJSONMember &Value) {
            return Object().Members(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSON::operator[](int Index) {
            if (m_ValueType != jvtArray)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Array()[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSON::operator[](int Index) const {
            if (m_ValueType != jvtArray)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Array()[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSON::operator[](CJSON::reference String) {
            if (m_ValueType != jvtObject)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Object().Values(String);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSON::operator[](CJSON::reference String) const {
            if (m_ValueType != jvtObject)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Object().Values(String);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSON::operator[](const CString &String) const {
            if (m_ValueType != jvtObject)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Object().Values(String);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSON::operator[](const CString &String) {
            if (m_ValueType != jvtObject)
                throw ExceptionFrm(JSON_INVALID_VALUE_TYPE);
            return Object().Values(String);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONElements ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CJSONElements::CJSONElements(CPersistent *AOwner, CJSONValueType ValueType): CJSON(AOwner, ValueType) {
            m_LineBreak = sLineBreak;
            m_Delimiter = ",";
            m_QuoteChar = '"';
            m_StrictDelimiter = false;
            m_CurrentIndex = -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONElements::GetValueFromIndex(int Index) {
            return Get(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONElements::GetValueFromIndex(int Index) const {
            return Get(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::SetValueFromIndex(int Index, const CJSONValue& Value) {
            if (!Value.IsEmpty()) {
                if (Index < 0) Index = Add(Value);
                Put(Index, Value);
            } else {
                if (Index >= 0)
                    Delete(Index);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::SetCurrentIndex(int Index) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            m_CurrentIndex = Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Error(const CString &Msg, int Data) {
            throw ExceptionFrm(Msg.c_str(), Data);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONElements::GetCapacity() const noexcept {
            return Count();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Put(int Index, const CJSONValue &Value) {
            Delete(Index);
            Insert(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::SetCapacity(int NewCapacity) {

        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONElements::Add(const CJSONValue &Value) {
            int Result = GetCount();
            Insert(Result, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONElements::Add(const CString &Value) {
            int Result = GetCount();
            Insert(Result, CJSONValue(Value));
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONElements::Add(reference Value) {
            int Result = GetCount();
            Insert(Result, CJSONValue(Value));
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::AddElements(const CJSONElements &Value) {
            BeginUpdate();
            try {
                for (int I = 0; I < Value.Count(); ++I)
                    Add(Value[I]);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Assign(const CJSONElements &Source) {
            BeginUpdate();
            try {
                Clear();

                m_QuoteChar = Source.m_QuoteChar;
                m_Delimiter = Source.m_Delimiter;
                m_LineBreak = Source.m_LineBreak;
                m_StrictDelimiter = Source.m_StrictDelimiter;

                AddElements(Source);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Concat(const CJSONElements &Source) {
            BeginUpdate();
            try {
                AddElements(Source);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::SetElements(const CJSONElements &Source) {
            BeginUpdate();
            try {
                Clear();
                AddElements(Source);
            } catch (...) {
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONElements::Equals(const CJSONElements &Value) {
            int I, Count;
            Count = GetCount();
            if (Count != Value.GetCount())
                return false;
            for (I = 0; I < Count; ++I)
                if (Get(I) != Value.Get(I))
                    return false;
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Exchange(int Index1, int Index2) {

            BeginUpdate();
            try {
                CJSONValue &Temp = Values(Index1);
                Values(Index1, Values(Index2));
                Values(Index2, Temp);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONElements::IndexOf(const CJSONValue &Value) const {
            for (int I = 0; I < GetCount(); ++I) {
                if (Get(I) == Value)
                    return I;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONElements::Move(int CurIndex, int NewIndex) {
            if (CurIndex != NewIndex) {
                BeginUpdate();
                try {
                    auto Temp = Get(CurIndex);
                    Delete(CurIndex);
                    Insert(NewIndex, Temp);
                } catch (...) {
                    EndUpdate();
                    throw;
                }
                EndUpdate();
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CJSONElements::JsonToString() const {
            CString S;

            S += "[";

            for (int i = 0; i < Count(); i++) {
                if (i > 0) {
                    S += ", ";
                }

                S << Values(i).ToString();
            }

            S += "]";

            return S;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONMembers ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CJSONMembers::CJSONMembers(CPersistent *AOwner, CJSONValueType ValueType): CJSON(AOwner, ValueType) {
            m_LineBreak = sLineBreak;
            m_Delimiter = sDelimiter;
            m_QuoteChar = sQuoteChar;
            m_NameValueSeparator = sNameValueSeparator;
            m_StrictDelimiter = false;
            m_CurrentIndex = -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetValue(const CString &String, const CJSONValue &Value) {
            int I = IndexOfString(String);

            if (!Value.IsEmpty()) {
                if (I < 0) I = Add(CJSONMember(Value));
                PutPair(I, String, Value);
            } else {
                if (I >= 0)
                    Delete(I);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetValue(reference String, const CJSONValue &Value) {
            int I = IndexOfString(String);

            if (!Value.IsEmpty()) {
                if (I < 0) I = Add(CJSONMember(Value));
                PutPair(I, String, Value);
            } else {
                if (I >= 0)
                    Delete(I);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONMembers::GetValueFromIndex(int Index) {
            return Get(Index).Value();
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONMembers::GetValueFromIndex(int Index) const {
            return Get(Index).Value();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetValueFromIndex(int Index, const CJSONValue &Value) {
            if (!Value.IsEmpty()) {
                if (Index < 0)
                    Index = Add(CJSONMember(Value));
                const CString &String = Strings(Index);
                PutPair(Index, String, Value);
            } else {
                if (Index >= 0)
                    Delete(Index);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetCurrentIndex(int Index) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            m_CurrentIndex = Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Error(const CString &Msg, int Data) {
            throw ExceptionFrm(Msg.c_str(), Data);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::GetCapacity() const noexcept {
            return Count();
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CJSONMembers::JsonToString() const {
            CString S;

            S = "{";

            for (int i = 0; i < Count(); i++) {

                if (i > 0) {
                    S += ", ";
                }

                S += "\"";
                S += Members(i).String();
                S += "\": ";

                S << Members(i).Value().ToString();
            }

            S += "}";

            return S;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Put(int Index, const CJSONMember &Value) {
            Delete(Index);
            Insert(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::PutPair(int Index, const CString &String, const CJSONValue& Value) {
            Delete(Index);
            InsertPair(Index, String, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::PutPair(int Index, reference String, const CJSONValue& Value) {
            Delete(Index);
            InsertPair(Index, String, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetCapacity(int NewCapacity) {

        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::Add(const CJSONMember &Value) {
            int Result = GetCount();
            Insert(Result, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, const CJSONMembers &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(CJSONMembers::reference String, const CJSONMembers &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, const CJSONElements &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(CJSONMembers::reference String, const CJSONElements &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, const CJSONValue &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, const CJSONValue &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, const CString &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, const CString &Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, CJSONMembers::reference Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, reference Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, bool Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, bool Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, int Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, int Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, float Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, float Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(const CString &String, double Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::AddPair(reference String, double Value) {
            int Result = GetCount();
            InsertPair(Result, String, Value);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::AddMembers(const CJSONMembers& AMembers) {
            BeginUpdate();
            try {
                for (int I = 0; I < AMembers.Count(); ++I)
                    Add(AMembers.Members(I));
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Assign(const CJSONMembers& Source) {

            BeginUpdate();
            try {
                Clear();

                m_NameValueSeparator = Source.m_NameValueSeparator;
                m_QuoteChar = Source.m_QuoteChar;
                m_Delimiter = Source.m_Delimiter;
                m_LineBreak = Source.m_LineBreak;
                m_StrictDelimiter = Source.m_StrictDelimiter;

                AddMembers(Source);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Concat(const CJSONMembers& Source) {

            BeginUpdate();
            try {
                AddMembers(Source);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::SetMembers(const CJSONMembers& Source) {
            BeginUpdate();
            try {
                Clear();
                AddMembers(Source);
            } catch (...) {
            }
            EndUpdate();
        }

        //--------------------------------------------------------------------------------------------------------------

        bool CJSONMembers::Equals(const CJSONMembers &Members) {
            int I, Count;
            Count = GetCount();
            if (Count != Members.GetCount())
                return false;
            for (I = 0; I < Count; ++I)
                if (Get(I) != Members.Get(I))
                    return false;
            return true;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Exchange(int Index1, int Index2) {
            BeginUpdate();
            try {
                const CJSONMember &Temp = Members(Index1);
                Members(Index1, Members(Index2));
                Members(Index2, Temp);
            } catch (...) {
                EndUpdate();
                throw;
            }
            EndUpdate();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::IndexOf(const CJSONMember &Value) const {
            for (int I = 0; I < GetCount(); ++I) {
                if (Get(I) == Value)
                    return I;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::IndexOfString(const CString &String) const {
            for (int I = 0; I < GetCount(); ++I) {
                if (Get(I).String() == String)
                    return I;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONMembers::IndexOfString(reference String) const {
            for (int I = 0; I < GetCount(); ++I) {
                if (Get(I).String() == String)
                    return I;
            }

            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONMembers::Move(int CurIndex, int NewIndex) {
            if (CurIndex != NewIndex) {
                BeginUpdate();
                try {
                    const CJSONMember &Temp = Get(CurIndex);
                    Delete(CurIndex);
                    Insert(NewIndex, Temp);
                } catch (...) {
                    EndUpdate();
                    throw;
                }
                EndUpdate();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONValue ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONValue::Get(int Index) {
            if (IsObject())
                return AsObject()[Index];

            if (IsArray())
                return AsArray()[Index];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONValue::Get(int Index) const {
            if (IsObject())
                return AsObject()[Index];

            if (IsArray())
                return AsArray()[Index];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::Put(int Index, const CJSONValue &Value) {
            if (IsObject())
                AsObject()[Index] = Value;

            if (IsArray())
                AsArray()[Index] = Value;

            *this = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONValue::GetValue(const CString &String) {
            if (IsObject())
                return AsObject()[String];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONValue::GetValue(const CString &String) const {
            if (IsObject())
                return AsObject()[String];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONValue::GetValue(reference String) {
            if (IsObject())
                return AsObject()[String];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONValue::GetValue(reference String) const {
            if (IsObject())
                return AsObject()[String];

            return *this;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::SetValue(const CString &String, const CJSONValue &Value) {
            if (IsObject())
                AsObject()[String] = Value;

            *this = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::SetValue(CJSONValue::reference String, const CJSONValue &Value) {
            if (IsObject())
                AsObject()[String] = Value;

            *this = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::Assign(const CJSONValue &Value) {
            inherited::Assign(Value);
            m_Data = Value.m_Data;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::Assign(const CJSONMembers &Value) {
            inherited::Assign(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::Assign(const CJSONElements &Value) {
            inherited::Assign(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::StringData(const CString &Value) {
            m_Data = EncodeJsonString(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONValue::StringData(CJSONValue::reference AValue) {
            CString Value(AValue);
            m_Data = EncodeJsonString(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CJSONValue::AsString() const {
            return DecodeJsonString(m_Data);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONValue::AsBoolean() const {
            LPCTSTR LBoolStr[] = ARRAY_BOOLEAN_STRINGS;

            for (size_t i = 0; i < chARRAY(LBoolStr); ++i) {
                if (SameText(LBoolStr[i], m_Data.c_str()))
                    return Odd(i);
            }

            throw EConvertError(_T("Invalid conversion string \"%s\" to boolean value."), m_Data.c_str());
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONValue::Compare(const CJSONValue &Value) const {

            if (IsEmpty())
                return -1;

            if (Value.IsEmpty())
                return 1;

            if (ValueType() == Value.ValueType()) {

                switch (ValueType()) {
                    case jvtObject:
                    case jvtArray:
                        if (Count() == Value.Count())
                            return 0;
                        if (Count() > Value.Count())
                            return 1;
                        return -1;

                    case jvtString:
                        return AsString().Compare(Value.AsString());

                    case jvtNumber:
                        if (AsDouble() == Value.AsDouble())
                            return 0;
                        if (AsDouble() > Value.AsDouble())
                            return 1;
                        return -1;

                    case jvtBoolean:
                        if (AsBoolean() == Value.AsBoolean())
                            return 0;
                        if (AsBoolean())
                            return 1;
                        return -1;

                    case jvtNull:
                        return 0;
                }
            }

            return incorrect_type;
        }
        //--------------------------------------------------------------------------------------------------------------

        CString CJSONValue::JsonToString() const {
            CString S;

            switch (ValueType()) {
                case jvtObject:
                    S = AsObject().ToString();
                    break;

                case jvtArray:
                    S = AsArray().ToString();
                    break;

                case jvtString:
                    S = "\"";
                    S += Data();
                    S += "\"";
                    break;

                case jvtNumber:
                    S = Data();
                    break;

                case jvtBoolean:
                    if (AsBoolean()) {
                        S = "true";
                    } else {
                        S = "false";
                    }
                    break;

                case jvtNull:
                    S = "null";
                    break;
            }

            return S;
        }

        bool CJSONValue::HasOwnProperty(const CString &String) const {
            if (Assigned(m_Value)) {
                if (m_Value->IsObject())
                    return Object().HasOwnProperty(String);
                if (m_Value->IsArray())
                    return Array().HasOwnProperty(String);
                return dynamic_cast<CJSONValue *> (m_Value)->Data() == String;
            }
            return m_Data == String;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONArray ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        void QuickSort(CJSONValueList& SortList, int L, int R, CJSONListSortCompare SCompare) {
            int I, J;
            CJSONValue T;

            do {
                I = L;
                J = R;
                const CJSONValue& P = SortList[(L + R) >> 1];

                do {
                    while (SCompare(SortList[I], P) < 0)
                        I++;
                    while (SCompare(SortList[J], P) > 0)
                        J--;
                    if (I <= J) {
                        T = SortList[I];
                        SortList[I] = SortList[J];
                        SortList[J] = T;
                        I++;
                        J--;
                    }
                } while (I <= J);

                if (L < J)
                    QuickSort(SortList, L, J, SCompare);

                L = I;
            } while (I < R);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray::CJSONArray(): CJSONArray(this) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray::CJSONArray(const CJSONArray &Array): CJSONArray(this) {
            CJSONArray::Assign(Array);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray::CJSONArray(const CString &String): CJSONArray(this) {
            StringToJson(String);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray::CJSONArray(CPersistent *AOwner): CJSONElements(AOwner, jvtArray) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray::~CJSONArray() {
            CJSONArray::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONArray::Get(int Index) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            return m_pList[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONArray::Get(int Index) const {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            return m_pList[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Put(int Index, const CJSONValue &Value) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            m_pList[Index] = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONArray::Add(const CJSONValue &Value) {
            return m_pList.Add(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONArray::Add(const CString &Value) {
            return m_pList.Add(CJSONValue(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONArray::Add(reference Value) {
            return m_pList.Add(CJSONValue(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Insert(int Index, const CJSONValue &Value) {
            m_pList.Insert(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::SetCapacity(int NewCapacity) {
            m_pList.SetCapacity(NewCapacity);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONArray::GetCapacity() const noexcept {
            return m_pList.Capacity();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONArray::GetCount() const noexcept {
            return m_pList.Count();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Assign(const CJSONElements &Source) {
            Clear();
            for (int I = 0; I < Source.Count(); ++I) {
                Add(Source[I]);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Concat(const CJSONElements &Source) {
            for (int I = 0; I < Source.Count(); ++I) {
                Add(Source[I]);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Clear() {
            m_pList.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Delete(int Index) {
            m_pList.Delete(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONArray::HasOwnProperty(const CString &String) const {
            int Index = 0;
            while (Index < GetCount() && Get(Index).Data() != String)
                Index++;
            return Index != GetCount();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONArray::Sort(CJSONListSortCompare Compare) {
            if (m_pList.Count() > 0)
                QuickSort(m_pList, 0, GetCount() - 1, Compare);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONObject -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CJSONObject::CJSONObject(): CJSONObject(this) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject::CJSONObject(const CJSONObject &Object): CJSONObject(this) {
            CJSONObject::Assign(Object);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject::CJSONObject(const CString &String): CJSONObject(this) {
            StringToJson(String);
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject::CJSONObject(CPersistent *AOwner): CJSONMembers(AOwner, jvtObject) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject::~CJSONObject() {
            CJSONObject::Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CJSONObject::GetString(int Index) const {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            return m_pList[Index].String();
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONObject::GetValue(const CString &String) {
            int Index = IndexOfString(String);
            if (Index != -1) {
                return m_pList[Index].Value();
            }

            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONObject::GetValue(const CString &String) const {
            int Index = IndexOfString(String);
            if (Index != -1) {
                return m_pList[Index].Value();
            }

            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONObject::GetValue(reference String) {
            int Index = IndexOfString(String);
            if (Index != -1) {
                return m_pList[Index].Value();
            }

            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONValue &CJSONObject::GetValue(reference String) const {
            int Index = IndexOfString(String);
            if (Index != -1) {
                return m_pList[Index].Value();
            }

            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONMember &CJSONObject::Get(int Index) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            return m_pList[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        const CJSONMember &CJSONObject::Get(int Index) const {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            return m_pList[Index];
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::Put(int Index, const CJSONMember &Value) {
            if ((Index < 0) || (Index >= GetCount()))
                throw ExceptionFrm(SListIndexError, Index);

            m_pList[Index] = Value;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::Add(const CJSONMember &Value) {
            return m_pList.Add(Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString& String, const CJSONMembers &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, const CJSONMembers &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString& String, const CJSONElements &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, const CJSONElements &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString& String, const CJSONValue &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, const CJSONValue &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString& String, const CString &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, reference Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, const CString &Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString &String, bool Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(CJSONObject::reference String, bool Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString &String, int Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, int Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString &String, float Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, float Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(const CString &String, double Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::AddPair(reference String, double Value) {
            return m_pList.Add(CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::Insert(int Index, const CJSONMember &Value) {
            return m_pList.Insert(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, const CJSONMembers &Value) {
            m_pList.Insert(Index, CJSONMember(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, const CJSONMembers &Value) {
            m_pList.Insert(Index, CJSONMember(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, const CJSONElements &Value) {
            m_pList.Insert(Index, CJSONMember(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, const CJSONElements &Value) {
            m_pList.Insert(Index, CJSONMember(Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, const CJSONValue &Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, const CJSONValue &Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, const CString &Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, const CString &Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, reference Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, reference Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, bool Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, bool Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, int Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, int Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, float Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, float Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, const CString &String, double Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::InsertPair(int Index, reference String, double Value) {
            m_pList.Insert(Index, CJSONMember(String, Value));
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONObject::HasOwnProperty(const CString &String) const {
            int Index = 0;
            while (Index < GetCount() && Get(Index).String() != String)
                Index++;
            return Index != GetCount();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::SetCapacity(int NewCapacity) {
            m_pList.SetCapacity(NewCapacity);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::GetCapacity() const noexcept {
            return m_pList.Capacity();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONObject::GetCount() const noexcept {
            return m_pList.Count();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::Clear() {
            m_pList.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::Update(int Index) {

        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONObject::Delete(int Index) {
            m_pList.Delete(Index);
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CJSONParser -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        const TCHAR _value_true[]  = _T("true");
        const TCHAR _value_false[] = _T("false");
        const TCHAR _value_null[]  = _T("null");
        //--------------------------------------------------------------------------------------------------------------

        CJSONParser::CJSONParser(CJSON *Json) : CObject() {
            m_Json = Json;

            m_State = json_start;
            m_Result = -1;
            m_CharIndex = 0;
            m_Escape = false;

            m_pJsonList = new CList();
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONParser::~CJSONParser() {
            delete m_pJsonList;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONParser::Reset() {
            m_State = json_start;
            m_Result = -1;
            m_CharIndex = 0;

            m_pJsonList->Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONParser::DeleteLastJson() {
            m_pJsonList->Extract(m_pJsonList->Last());
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSON &CJSONParser::CurrentJson() {
            return *(CJSON *) m_pJsonList->Last();
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONMember &CJSONParser::CurrentMember() {
            return CurrentObject().Last();
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONObject &CJSONParser::CurrentObject() {
            if (CurrentJson().ValueType() != jvtObject)
                throw Exception::Exception(_T("Invalid JSON type!"));
            return dynamic_cast<CJSONObject &> (CurrentJson());
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONArray &CJSONParser::CurrentArray() {
            if (CurrentJson().ValueType() != jvtArray)
                throw Exception::Exception(_T("Invalid JSON type!"));
            return dynamic_cast<CJSONArray &> (CurrentJson());
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONValue &CJSONParser::CurrentValue() {
            if (CurrentJson().IsObject())
                return CurrentObject().Last().Value();
            return CurrentArray().Last();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CJSONParser::CreateValue(CJSONValueType ValueType) {
            if (CurrentJson().IsObject()) {
                switch (ValueType) {
                    case jvtObject:
                        m_pJsonList->Add(CurrentValue().GetObject());
                        break;
                    case jvtArray:
                        m_pJsonList->Add(CurrentValue().GetArray());
                        break;
                    default:
                        CurrentValue().ValueType(ValueType);
                        break;
                }
            } else if (CurrentJson().IsArray()) {
                switch (ValueType) {
                    case jvtObject:
                    case jvtArray:
                        CurrentArray().Add(CJSONValue(ValueType));
                        m_pJsonList->Add(CurrentArray().Last().Value());
                        break;
                    default:
                        CurrentArray().Add(CJSONValue(ValueType));
                        break;
                }
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CJSONParserResult CJSONParser::Parse(LPTSTR ABegin, LPCTSTR AEnd) {
            LPCTSTR Start = ABegin;

            if (m_Json->Count() > 0 ) {
                m_State = value_start;
            }

            while ((m_Result == -1) && (ABegin != AEnd)) {
                m_Result = Consume(*ABegin++);
            }
            return {m_Result, size_t(ABegin - Start - 1)};
        }
        //--------------------------------------------------------------------------------------------------------------

        int CJSONParser::Consume(u_char AInput) {
            switch (m_State) {
                case json_start:
                    if (AInput == '{') {
                        m_pJsonList->Add(m_Json->GetObject());
                        m_State = string_start;
                        return -1;
                    } else if (AInput == '[') {
                        m_pJsonList->Add(m_Json->GetArray());
                        m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case string_start:
                    if (AInput == '"') {
                        CurrentObject().Add(CJSONMember());
                        m_State = string;
                        return -1;
                    } else if (AInput == '}') {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case string:
                    if (IsLetter(AInput) || IsDigit(AInput)) {
                        CurrentMember().String().Append((TCHAR) AInput);
                        m_State = string;
                        return -1;
                    } else if (AInput == '"') {
                        m_State = string_end;
                        return -1;
                    }
                    return false;
                case string_end:
                    if (AInput == ':') {
                        m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_start:
                    if (AInput == '{') {
                        CreateValue(jvtObject);
                        m_State = string_start;
                        return -1;
                    } else if (AInput == '[') {
                        CreateValue(jvtArray);
                        m_State = value_start;
                        return -1;
                    } else if (AInput == '"') {
                        CreateValue(jvtString);
                        m_State = value_string_start;
                        return -1;
                    } else if ((AInput == '-') || IsDigit(AInput)) {
                        CreateValue(jvtNumber);
                        CurrentValue().Data().Append(AInput);
                        m_State = value_digit;
                        return -1;
                    } else if (AInput == 't') {
                        CreateValue(jvtBoolean);
                        CurrentValue().Data().Append(AInput);
                        m_CharIndex++;
                        m_State = value_true;
                        return -1;
                    } else if (AInput == 'f') {
                        CreateValue(jvtBoolean);
                        CurrentValue().Data().Append(AInput);
                        m_CharIndex++;
                        m_State = value_false;
                        return -1;
                    } else if (AInput == 'n') {
                        CreateValue(jvtNull);
                        CurrentValue().Data().Append(AInput);
                        m_CharIndex++;
                        m_State = value_null;
                        return -1;
                    } else if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_end:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        if (m_pJsonList->Count() == 0)
                            return true;
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_string_start:
                    if (AInput == '"') {
                        m_State = value_string_end;
                        return -1;
                    } else if (IsCharacter(AInput)) {
                        if (AInput == '\\') {
                            m_Escape = true;
                        }
                        CurrentValue().Data().Append(AInput);
                        m_State = value_string;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_string:
                    if (!m_Escape && AInput == '"') {
                        m_State = value_string_end;
                        return -1;
                    } else if (IsCharacter(AInput)) {
                        if (m_Escape) {
                            m_Escape = false;
                        } else {
                            if (AInput == '\\') {
                                m_Escape = true;
                            }
                        }
                        CurrentValue().Data().Append(AInput);
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_string_end:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if (AInput == '"') {
                        m_State = value_string;
                        return -1;
                    } else if (IsCharacter(AInput)) {
                        m_State = value_string_end;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_digit:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if ((AInput == '.') || IsDigit(AInput) || (AInput == 'e') || (AInput == 'E') ||
                               (AInput == '+') || (AInput == '-')) {
                        CurrentValue().Data().Append(AInput);
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_true:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (AInput == _value_true[m_CharIndex]) {
                        m_CharIndex++;
                        if (m_CharIndex == 4) {
                            m_CharIndex = 0;
                        }
                        CurrentValue().Data().Append(AInput);
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_false:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (AInput == _value_false[m_CharIndex]) {
                        m_CharIndex++;
                        if (m_CharIndex == 5) {
                            m_CharIndex = 0;
                        }
                        CurrentValue().Data().Append(AInput);
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                case value_null:
                    if ((AInput == '}') || (AInput == ']')) {
                        DeleteLastJson();
                        m_State = value_end;
                        return -1;
                    } else if (AInput == _value_null[m_CharIndex]) {
                        m_CharIndex++;
                        if (m_CharIndex == 4) {
                            m_CharIndex = 0;
                        }
                        CurrentValue().Data().Append(AInput);
                        return -1;
                    } else if (AInput == ',') {
                        if (CurrentJson().IsObject())
                            m_State = string_start;
                        else
                            m_State = value_start;
                        return -1;
                    } else if (IsWS(AInput)) {
                        return -1;
                    }
                    return false;
                default:
                    return false;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsLetter(u_char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '-') || (c == '@') || (c == '#') || (c == '.');
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsCharacter(unsigned c) {
            return (c >= 0x20 && c <= 0x10FFFF) || (c == '"') || (c == '\\');
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsCtl(u_char c) {
            return (c <= 31) || (c == 127);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsDigit(u_char c) {
            return c >= '0' && c <= '9';
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsWS(u_char c) {
            return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CJSONParser::IsEscape(u_char c) {
            switch (c) {
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'n':
                case 'r':
                case 't':
                case 'u':
                    return true;
                default:
                    return false;
            }
        }
    }
}
}
