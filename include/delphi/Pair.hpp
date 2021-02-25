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

        template<class ClassValue>
        class TPair: public CObject {
            typedef ClassValue *PClassValue;

        private:

            CString m_Name;
            ClassValue m_Value;
            CStringList m_Data;

        public:

            TPair() = default;
            ~TPair() override = default;

            TPair(const CString &Name, const ClassValue &Value) {
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

            CString &Name() { return m_Name; }
            const CString &Name() const { return m_Name; }

            ClassValue &Value() { return m_Value; }
            const ClassValue &Value() const { return m_Value; }

            CStringList &Data() { return m_Data; }
            const CStringList &Data() const { return m_Data; }

            inline bool operator!=(const TPair &Pair) { return m_Name != Pair.Name; }
            inline bool operator==(const TPair &Pair) { return m_Name == Pair.Name; }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- TPairs ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        template<class ClassValue>
        class TPairs: public CObject {
            typedef TPairs<ClassValue> ClassPairs;
            typedef TPair<ClassValue> ClassPair;
            typedef TList<ClassPair> ClassList;

            typedef ClassPair& reference;
            typedef ClassPair* pointer;
            typedef const ClassPair& const_reference;
            typedef const ClassPair* const_pointer;

        private:

            ClassList m_pList;

            ClassPair m_Default;

            void Put(int Index, const ClassPair &Pair) {
                m_pList.Delete(Index);
                m_pList.Insert(Index, Pair);
            }

            ClassPair &Get(int Index) {
                if (Index == -1)
                    return m_Default;
                return m_pList.Items(Index);
            }

            const ClassPair &Get(int Index) const {
                if (Index == -1)
                    return m_Default;
                return m_pList.Items(Index);
            }

        protected:

            int GetCount() const {
                return m_pList.GetCount();
            }

            ClassValue &GetValue(const CString &Name) {
                int Index = IndexOfName(Name);
                if (Index != -1) {
                    ClassPair &Pair = Get(Index);
                    return Pair.Value();
                }
                return m_Default.Value();
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

            typedef TEnumerator<ClassPair, ClassPairs> Enumerator;
            typedef const TEnumerator<ClassPair, ClassPairs> ConstEnumerator;

            TPairs() = default;

            TPairs(const TPairs &Value) {
                Assign(Value);
            }

            TPairs(TPairs &&Value) {
                Assign(Value);
            }

            ~TPairs() override {
                Clear();
            }

            void Clear() {
                m_pList.Clear();
            }

            ClassList *Expand() {
                return m_pList.Expand();
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

            reference begin() {
                return m_pList.First();
            }

            const_reference cbegin() const {
                return m_pList.First();
            }

            reference end() {
                return m_pList.Last();
            }

            const_reference cend() const {
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
                m_Default = Value.m_Default;
            }

            ClassPair &Default() { return m_Default; }
            const ClassPair &Default() const { return m_Default; }

            ClassValue &DefaultValue() { return m_Default.Value(); }
            const ClassValue &DefaultValue() const { return m_Default.Value(); }

            ClassValue &Values(const CString &Name) { return GetValue(Name); }
            const ClassValue &Values(const CString &Name) const { return GetValue(Name); }

            void Values(const CString &Name, const ClassValue &Value) { SetValue(Name, Value); }

            ClassPair &Items(int Index) { return Get(Index); }
            const ClassPair &Items(int Index) const { return Get(Index); }

            void Items(int Index, const ClassPair &Pair) { Put(Index, Pair); }

            ClassPair &Pairs(const CString &Name) { return Get(IndexOfName(Name)); }
            const ClassPair &Pairs(const CString &Name) const { return Get(IndexOfName(Name)); }

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

            ClassPair &operator[](int Index) { return Items(Index); }
            const ClassPair &operator[](int Index) const { return Items(Index); }

            ClassValue &operator[](const CString &Name) { return Values(Name); }
            const ClassValue &operator[](const CString &Name) const { return Values(Name); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringPairs ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef TPair<CString> CStringPair;
        typedef TPairs<CString> CStringPairs;
        typedef TPairs<CStringList> CStringListPairs;

    }
}
}
#endif //DELPHI_PAIR_HPP
