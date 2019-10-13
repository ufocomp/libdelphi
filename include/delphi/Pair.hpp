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

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringPair -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef struct CStringPair {

            CString Name;
            CString Value;

            CStringPair &operator=(const CStringPair &Pair) {
                if (this != &Pair) {
                    this->Name = Pair.Name;
                    this->Value = Pair.Value;
                }
                return *this;
            };

            inline bool operator!=(const CStringPair &Pair) { return Name != Pair.Name; };

            inline bool operator==(const CStringPair &Pair) { return Name == Pair.Name; };

        } CStringPair, *PStringPair;

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringPairs ----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CStringPairs {
        private:

            TList<CStringPair> m_pList;

            CString m_NullValue;

            void Put(int Index, const CStringPair &Item);

            CStringPair &Get(int Index);

            const CStringPair &Get(int Index) const;

        protected:

            int GetCount() const;

            const CString &GetValue(const CString &Name) const;

            const CString &GetValue(LPCTSTR Name) const;

        public:

            CStringPairs() = default;

            CStringPairs(const CStringPairs &Value) {
                Assign(Value);
            };

            ~CStringPairs();

            void Clear();

            int IndexOfName(const CString &Name) const;

            int IndexOfName(LPCTSTR Name) const;

            void Insert(int Index, const CStringPair &Item);

            int Add(const CStringPair &Pair);

            int AddPair(LPCTSTR lpszName, LPCTSTR lpszValue);

            int AddPair(LPCTSTR lpszName, const CString &Value);

            int AddPair(const CString &Name, const CString &Value);

            void Delete(int Index);

            void SetCount(int NewCount);

            CStringPair &First();

            CStringPair &Last();

            int Count() const { return GetCount(); }

            void Assign(const CStringPairs &Value);

            const CString &Values(const CString &Name) const { return GetValue(Name); };

            const CString &Values(LPCTSTR Name) const { return GetValue(Name); };

            CStringPair &Pairs(int Index) { return Get(Index); }

            const CStringPair &Pairs(int Index) const { return Get(Index); }

            void Pairs(int Index, const CStringPair &Item) { Put(Index, Item); }

            CStringPairs &operator=(const CStringPairs &Value) {
                if (this != &Value)
                    Assign(Value);
                return *this;
            };

            CStringPair &operator[](int Index) { return Get(Index); }

            const CStringPair &operator[](int Index) const { return Get(Index); }

            CStringPair &operator[](LPCTSTR Name) { return Pairs(IndexOfName(Name)); }

            const CStringPair &operator[](LPCTSTR Name) const { return Pairs(IndexOfName(Name)); }

            CStringPair &operator[](const CString &Name) { return Pairs(IndexOfName(Name)); }

            const CStringPair &operator[](const CString &Name) const { return Pairs(IndexOfName(Name)); }

        };

    }
}
}
#endif //DELPHI_PAIR_HPP
