/*++

Library name:

  libdelphi

Module Name:

  Pair.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Pair.hpp"

extern "C++" {

namespace Delphi {

    namespace Classes {

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringPairs ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CStringPairs::~CStringPairs() {
            Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::Put(int Index, const CStringPair &Item) {
            m_pList.Insert(Index, Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        CStringPair &CStringPairs::Get(int Index) {
            return m_pList.Items(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CStringPair &CStringPairs::Get(int Index) const {
            return m_pList.Items(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CStringPairs::GetValue(const CString &Name) const {
            int Index = IndexOfName(Name);
            if (Index != -1) {
                const CStringPair &Pair = Get(Index);
                return Pair.Value;
            }
            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        const CString &CStringPairs::GetValue(LPCTSTR Name) const {
            int Index = IndexOfName(Name);
            if (Index != -1) {
                const CStringPair &Pair = Get(Index);
                return Pair.Value;
            }
            return m_NullValue;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::Clear() {
            m_pList.Clear();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::IndexOfName(const CString &Name) const {
            for (int I = 0; I < GetCount(); ++I) {
                const CStringPair &Pair = Get(I);
                if (Pair.Name == Name)
                    return I;
            }
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::IndexOfName(LPCTSTR Name) const {
            for (int I = 0; I < GetCount(); ++I) {
                const CStringPair &Pair = Get(I);
                if (Pair.Name == Name)
                    return I;
            }
            return -1;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::Insert(int Index, const CStringPair &Item) {
            Put(Index, Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::Add(const CStringPair &Item) {
            int Result = GetCount();
            Insert(Result, Item);
            return Result;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::AddPair(LPCTSTR lpszName, LPCTSTR lpszValue) {
            int Index = Add(CStringPair());
            Last().Name = lpszName;
            Last().Value = lpszValue;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::AddPair(LPCTSTR lpszName, const CString &Value) {
            int Index = Add(CStringPair());
            Last().Name = lpszName;
            Last().Value = Value;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::AddPair(const CString &Name, const CString &Value) {
            int Index = Add(CStringPair());
            Last().Name = Name;
            Last().Value = Value;
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::Delete(int Index) {
            m_pList.Delete(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CStringPairs::GetCount() const {
            return m_pList.GetCount();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::SetCount(int NewCount) {
            int LCount = GetCount();
            if (NewCount > LCount) {
                for (int I = LCount; I < NewCount; ++I)
                    Add(CStringPair());
            } else {
                for (int I = LCount - 1; I >= NewCount; --I)
                    Delete(I);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        CStringPair &CStringPairs::First() {
            return m_pList.First();
        }
        //--------------------------------------------------------------------------------------------------------------

        CStringPair &CStringPairs::Last() {
            return m_pList.Last();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CStringPairs::Assign(const CStringPairs &Value) {
            Clear();
            for (int I = 0; I < Value.GetCount(); ++I) {
                Add(Value[I]);
            }
        }
    }
}
}