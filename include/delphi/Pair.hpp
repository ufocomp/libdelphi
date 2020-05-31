/*++

Library name:

  libdelphi

Module Name:

  Pair.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_PAIR_HPP
#define DELPHI_PAIR_HPP

extern "C++" {

namespace Delphi {

    namespace Classes {

        template<class ClassName>
        class TPair : public CObject {
            typedef ClassName *PClassName;

        private:

            CString m_Name;
            ClassName m_Value;
            CStringList m_Data;

        public:

            TPair() = default;

            TPair(const CString &Name, const ClassName &Value) {
                m_Name = Name;
                m_Value = Value;
            }

            TPair &operator=(const TPair &Pair) {
                if (this != &Pair) {
                    m_Name = Pair.m_Name;
                    m_Value = Pair.m_Value;
                    m_Data = Pair.m_Data;
                }
                return *this;
            }
            
            bool IsEmpty() { return m_Value.IsEmpty(); }

            CString &Name() {return m_Name; }
            const CString &Name() const {return m_Name; }

            ClassName &Value() {return m_Value; }
            const ClassName &Value() const {return m_Value; }

            CStringList &Data() {return m_Data; }
            const CStringList &Data() const {return m_Data; }

            inline bool operator!=(const TPair &Pair) { return m_Name != Pair.Name; }
            inline bool operator==(const TPair &Pair) { return m_Name == Pair.Name; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- TPairs ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        template<class ClassValue>
        class TPairs {
            typedef TPair<ClassValue> ClassPair;

        private:

            TList<ClassPair> m_pList;

            ClassPair m_Default;

            void Put(int Index, const ClassPair &Pair) {
                m_pList.Delete(Index);
                m_pList.Insert(Index, Pair);
            }

            ClassPair &Get(int Index) {
                return m_pList.Items(Index);
            }
            
            const ClassPair &Get(int Index) const {
                return m_pList.Items(Index);
            }

        protected:

            int GetCount() const {
                return m_pList.GetCount();
            }

            const ClassValue &GetValue(const CString &Name) const {
                int Index = IndexOfName(Name);
                if (Index != -1) {
                    const ClassPair &Pair = Get(Index);
                    return Pair.Value();
                }
                return m_Default.Value();
            }
            
            void SetValue(const CString &Name, const ClassValue &Value) {
                int Index = IndexOfName(Name);
                if (!Value.IsEmpty()) {
                    if (Index < 0) Index = Add(ClassPair());
                    Put(Index, ClassPair(Name, Value));
                } else {
                    if (Index >= 0)
                        Delete(Index);
                }
            }

        public:

            TPairs() = default;

            TPairs(const TPairs &Value) {
                Assign(Value);
            }

            ~TPairs() {
                Clear();
            }

            void Clear() {
                m_pList.Clear();
            }

            int IndexOfName(const CString &Name) const {
                for (int I = 0; I < GetCount(); ++I) {
                    const ClassPair &Pair = Get(I);
                    if (Pair.Name().Lower() == Name.Lower())
                        return I;
                }
                return -1;
            }

            void Insert(int Index, const ClassPair &Pair) {
                m_pList.Insert(Index, Pair);
            }

            int Add(const ClassPair &Pair) {
                int Result = GetCount();
                Insert(Result, Pair);
                return Result;
            }

            int AddPair(const CString &Name, const ClassValue &Value) {
                return Add(ClassPair(Name, Value));
            }

            void Delete(int Index) {
                m_pList.Delete(Index);
            }

            void SetCount(int NewCount) {
                int LCount = GetCount();
                if (NewCount > LCount) {
                    for (int I = LCount; I < NewCount; ++I)
                        Add(ClassPair());
                } else {
                    for (int I = LCount - 1; I >= NewCount; --I)
                        Delete(I);
                }
            }

            ClassPair &First() {
                return m_pList.First();
            }

            ClassPair &Last() {
                return m_pList.Last();
            }

            int Count() const { return GetCount(); }

            void Concat(const TPairs &Value) {
                for (int I = 0; I < Value.GetCount(); ++I) {
                    Add(Value[I]);
                }
            }

            void Assign(const TPairs &Value) {
                Clear();
                Concat(Value);
            }

            ClassPair &Default() { return m_Default; }
            const ClassPair &Default() const { return m_Default; }

            const ClassValue &Values(const CString &Name) const { return GetValue(Name); }
            void Values(const CString &Name, const ClassValue &Value) { SetValue(Name, Value); }

            ClassPair &Pairs(int Index) { return Get(Index); }
            const ClassPair &Pairs(int Index) const { return Get(Index); }

            void Pairs(int Index, const ClassPair &Pair) { Put(Index, Pair); }

            TPairs &operator=(const TPairs &Value) {
                if (this != &Value)
                    Assign(Value);
                return *this;
            }

            TPairs& operator<< (const TPairs &Value) {
                if (this != &Value)
                    Concat(Value);
                return *this;
            };

            ClassPair &operator[](int Index) { return Get(Index); }
            const ClassPair &operator[](int Index) const { return Get(Index); }

            ClassPair &operator[](LPCTSTR Name) { return Pairs(IndexOfName(Name)); }
            const ClassPair &operator[](LPCTSTR Name) const { return Pairs(IndexOfName(Name)); }

            ClassPair &operator[](const CString &Name) { return Pairs(IndexOfName(Name)); }
            const ClassPair &operator[](const CString &Name) const { return Pairs(IndexOfName(Name)); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringPairs ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPairs<CString> CStringPairs;

    }
}
}
#endif //DELPHI_PAIR_HPP
