/*++

Library name:

  libdelphi

Module Name:

  Classes.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_CLASSES_HPP
#define DELPHI_CLASSES_HPP

extern "C++" {

namespace Delphi {

    namespace Classes {

        #define MaxBufSize 0xF000
        #define MAX_ERROR_STR 2048
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CObject;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CObject *Sender)> CNotifyEvent;
        //--------------------------------------------------------------------------------------------------------------

        typedef DWORD (*LPTHREAD_START_ROUTINE) (LPVOID lpThreadParameter);
        //--------------------------------------------------------------------------------------------------------------

        typedef LPTHREAD_START_ROUTINE TThreadFunc;
        //--------------------------------------------------------------------------------------------------------------

        class CHeap;
        class CSysError;
        //--------------------------------------------------------------------------------------------------------------

        extern LIB_DELPHI pid_t MainThreadID;
        //--------------------------------------------------------------------------------------------------------------

        class CDefaultLocale;
        //--------------------------------------------------------------------------------------------------------------

        extern LIB_DELPHI CDefaultLocale DefaultLocale;
        //--------------------------------------------------------------------------------------------------------------

        extern LIB_DELPHI CHeap *GHeap;
        extern LIB_DELPHI CSysError *GSysError;
        //--------------------------------------------------------------------------------------------------------------

        typedef unsigned (*PTHREAD_START)(void *);
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CObject {
        public:

            CObject() = default;
            virtual ~CObject() = default;

            inline static class CObject *Create() { return new CObject(); };
            inline virtual void Destroy() { delete this; };

            void Free() { if (this != nullptr) Destroy(); };
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHeap -----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CHeap {
        private:

            HANDLE m_hHandle;

            int m_Options;

            size_t m_InitialSize;
            size_t m_MaximumSize;

        protected:

            void Create();
            void Destroy();

        public:

            CHeap();

            virtual ~CHeap();

            static inline class CHeap *CreateHeap() { return GHeap = new CHeap(); };

            static inline void DestroyHeap() { delete GHeap; };

            void Initialize();

            Pointer Alloc(unsigned long ulFlags, size_t ulSize);

            Pointer ReAlloc(unsigned long ulFlags, Pointer lpMem, size_t ulNewSize, size_t ulOldSize);

            Pointer Free(unsigned long ulFlags, Pointer lpMem, size_t ulSize);

            static unsigned long Size(unsigned long ulFlags, Pointer lpMem);

            HANDLE GetHandle() { return m_hHandle; }

            int GetOptions() const { return m_Options; }

            void SetOptions(int Value) { m_Options = Value; }

            size_t GetInitialSize() const { return m_InitialSize; }

            void SetInitialSize(size_t Value) { m_InitialSize = Value; }

            size_t GetMaximumSize() const { return m_MaximumSize; }

            void SetMaximumSize(size_t Value) { m_MaximumSize = Value; }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CDefaultLocale --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CDefaultLocale {
        private:

            locale_t m_Locale;

            LPCSTR m_LocaleName;

            int m_Category;

            int m_CategoryMask;

        public:

            explicit CDefaultLocale(locale_t ALocale = LC_GLOBAL_LOCALE) noexcept;

            explicit CDefaultLocale(LPCSTR Locale);
            ~CDefaultLocale();

            locale_t Locale() { return m_Locale; };

            LPCSTR LocaleName() { return m_LocaleName; };

            int Category() const { return m_Category; };
            void Category(int Value) { m_Category = Value; };

            int CategoryMask() const { return m_CategoryMask; };
            void CategoryMask(int Value) { m_CategoryMask = Value; };

            void SetLocale(LPCSTR Locale);
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHeapComponent --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CHeapComponent {
        public:
            CHeapComponent();
            virtual ~CHeapComponent();
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CList -----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        enum CDuplicates {
            dupIgnore, dupAccept, dupError
        };
        //--------------------------------------------------------------------------------------------------------------

        #define MaxListSize 134217727
        #define HEAP_ZERO_MEMORY    1
        //--------------------------------------------------------------------------------------------------------------

        #define SListCapacityError  _T("List capacity out of bounds (%d)")
        #define SListCountError     _T("List count out of bounds (%d)")
        #define SListIndexError     _T("List index out of bounds (%d)")
        //--------------------------------------------------------------------------------------------------------------

        typedef Pointer *PPointerList;

        typedef int (*ListSortCompare)(Pointer Item1, Pointer Item2);
        //--------------------------------------------------------------------------------------------------------------

        enum ListNotification {
            lnAdded, lnExtracted, lnDeleted
        };

        enum ListAssignOp {
            laCopy, laAnd, laOr, laXor, laSrcUnique, laDestUnique
        };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CList: public CObject, public CHeapComponent {
        private:

            PPointerList m_pList;

            int m_nCount;
            int m_nCapacity;

        protected:

            Pointer Get(int Index) const;

            void Put(int Index, Pointer Item);

            virtual void Grow();
            virtual void Notify(Pointer Ptr, ListNotification Action);

        public:

            CList();

            CList(const CList &List);

            ~CList() override;

            inline static class CList *Create() { return new CList(); };

            int Add(Pointer Item);

            void Clear();

            void Delete(int Index);

            void Exchange(int Index1, int Index2);

            CList *Expand();

            Pointer Extract(Pointer Item);

            Pointer First() const;

            int IndexOf(Pointer Item) const;

            void Insert(int Index, Pointer Item);

            Pointer Last() const;

            void Move(int CurIndex, int NewIndex);

            int Remove(Pointer Item);

            void Pack();

            void Sort(ListSortCompare Compare);

            void Assign(const CList &Source, ListAssignOp Operator = laCopy);

            int GetCapacity() const noexcept { return m_nCapacity; }
            void SetCapacity(int NewCapacity);

            int GetCount() const noexcept { return m_nCount; }
            void SetCount(int NewCount);

            int Capacity() const noexcept { return GetCapacity(); }
            int Count() const noexcept { return GetCount(); }

            PPointerList GetList() { return m_pList; }

            Pointer Items(int Index) const { return Get(Index); }
            void Items(int Index, Pointer Value) { Put(Index, Value); }

            Pointer operator[](int Index) const { return Get(Index); }

            CList& operator= (const CList& List) {
                if (this != &List) {
                    Assign(List);
                }
                return *this;
            };

        }; // CList

        //--------------------------------------------------------------------------------------------------------------

        enum COperation { opInsert, opRemove };

        //--------------------------------------------------------------------------------------------------------------

        //-- CPersistent -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CPersistent: public CObject
        {
        private:

            CPersistent *m_pOwner;

            int m_Tag;

        protected:

            virtual CPersistent *GetOwner() { return m_pOwner; };
            virtual CPersistent *GetOwner() const { return m_pOwner; };

        public:

            explicit CPersistent(CPersistent *AOwner): CObject(), m_pOwner(AOwner) {
                m_Tag = 0;
            };

            ~CPersistent() override = default;

            int Tag() const { return m_Tag; }
            void Tag(int Value) { m_Tag = Value; }

        }; // CPersistent

        //--------------------------------------------------------------------------------------------------------------

        //-- CCollection -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCollectionItem;
        //--------------------------------------------------------------------------------------------------------------

        enum CCollectionNotification { cnAdded, cnExtracting, cnDeleting };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCollection: public CPersistent
        {
            friend CCollectionItem;

        private:

            CList m_Items;

            int m_UpdateCount;
            int m_NextId;

            void InsertItem(CCollectionItem *Item);
            void RemoveItem(CCollectionItem *Item);

        protected:

            virtual void Added(CCollectionItem *Item);
            virtual void Deleting(CCollectionItem *Item);
            virtual void Notify(CCollectionItem *Item, CCollectionNotification Action);

            static int GetAttrCount();
            void Changed();

            virtual CCollectionItem *GetItem(int Index) const;
            virtual void SetItem(int Index, CCollectionItem *Value);

            virtual void SetItemName(CCollectionItem *Item);
            virtual void Update(CCollectionItem *Item);

            int GetCount() const;
            int GetNextId() const { return m_NextId; };
            int GetUpdateCount() const { return m_UpdateCount; };

        public:

            explicit CCollection(CPersistent *AOwner);

            ~CCollection() override;

            CPersistent *Owner();

            virtual void BeginUpdate();
            virtual void EndUpdate();

            void Clear();
            virtual void Delete(int Index);

            CCollectionItem *FindItemId(int Id);

            int Count() const { return GetCount(); };

            virtual CCollectionItem *Items(int Index) const { return GetItem(Index); }
            virtual void Items(int Index, CCollectionItem *Value) { SetItem(Index, Value); }

            virtual CCollectionItem *operator[] (int Index) const { return Items(Index); };

        }; // CCollection

        //--------------------------------------------------------------------------------------------------------------

        //-- CCollectionItem -------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCollectionItem: public CPersistent
        {
            typedef CPersistent inherited;
            friend CCollection;

        private:

            CCollection *m_Collection;
            int m_Id;

            int GetIndex();

        protected:

            void Changed(bool AllItems);
            CPersistent *GetOwner() override;
            void SetCollection(CCollection *Value);
            void SetIndex(int Value);

        public:

            explicit CCollectionItem(CCollection *Collection);

            ~CCollectionItem() override;

            CCollection *Collection() { return m_Collection; };
            void Collection(CCollection *Value) { SetCollection(Value); };

            int Id() const { return m_Id; };

            int Index() { return GetIndex(); };
            void Index(int Value) { SetIndex(Value); };

        }; // CCollectionItem

        //--------------------------------------------------------------------------------------------------------------

        //-- CStream ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define soFromBeginning   SEEK_SET
        #define soFromCurrent     SEEK_CUR
        #define soFromEnd         SEEK_END
        //--------------------------------------------------------------------------------------------------------------

        enum CSeekOrigin { soBeginning = SEEK_SET, soCurrent = SEEK_CUR, soEnd = SEEK_END };
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CStream: public CHeapComponent
        {
        private:

            off_t GetPosition() const;
            void SetPosition(off_t Pos) const;

        protected:

            virtual off_t GetSize() const;
            virtual void SetSize(size_t NewSize);

        public:

            CStream(): CHeapComponent() {};
            ~CStream() override = default;

            virtual size_t Read(void *Buffer, size_t Count) const abstract;
            virtual size_t Write(const void *Buffer, size_t Count) abstract;

            virtual off_t Seek(off_t Offset, unsigned short Origin) const abstract;
            virtual off_t Seek(off_t Offset, CSeekOrigin Origin) const;

            void ReadBuffer(void *Buffer, size_t Count) const;
            void WriteBuffer(const void *Buffer, size_t Count);

            size_t CopyFrom(CStream *Source, size_t Count);

            off_t Position() const { return GetPosition(); };
            void Position(off_t Value) { SetPosition(Value); };

            virtual size_t Size() const { return GetSize(); };
            void Size(size_t Value) { SetSize(Value); };

        }; // CStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CHandleStream ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define INVALID_HANDLE_VALUE            (-1)
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CHandleStream : public CStream
        {
            typedef CStream inherited;

        protected:

            int m_Handle;

            void CreateHandle(int AHandle);

            void SetSize(size_t NewSize) override;

        public:

            explicit CHandleStream(int AHandle): inherited(), m_Handle(INVALID_HANDLE_VALUE) { CreateHandle(AHandle); };

            ~CHandleStream() override = default;

            size_t Read(void *Buffer, size_t Count) const override;

            size_t Write(const void *Buffer, size_t Count) override;

            off_t Seek(off_t Offset, unsigned short Origin) const override;

        }; // CHandleStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CFileStream -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define OF_CREATE                       (O_CREAT | O_WRONLY | O_TRUNC)
        #define FILE_ATTRIBUTE_NORMAL           0644
        //--------------------------------------------------------------------------------------------------------------

        #define fmOpenRead                      (O_RDONLY)
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CFileStream : public CHandleStream
        {
            typedef CHandleStream inherited;

        public:

            CFileStream(LPCTSTR lpszFileName, unsigned short Mode);

            ~CFileStream() override;

            inline static class CFileStream *CreateAs(LPCTSTR lpszFileName, unsigned short Mode) {
                return new CFileStream(lpszFileName, Mode);
            };

        }; // CFileStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomMemoryStream ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CMemoryStream;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCustomMemoryStream: public CStream
        {
            friend CMemoryStream;

        private:

            Pointer m_Memory;

            size_t m_Size;
            mutable off_t m_Position;

        protected:

            void SetPointer(Pointer Ptr, size_t Size);

        public:

            CCustomMemoryStream();

            ~CCustomMemoryStream() override = default;

            inline off_t Seek(off_t Offset, CSeekOrigin Origin) const override { return CStream::Seek(Offset, Origin); };

            size_t Read(Pointer Buffer, size_t Count) const override;

            off_t Seek(off_t Offset, unsigned short Origin) const override;

            void SaveToStream(CStream &Stream) const;
            void SaveToFile(LPCTSTR lpszFileName);

            virtual Pointer Memory() const { return m_Memory; }

        }; // CCustomMemoryStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CMemoryStream ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CMemoryStream: public CCustomMemoryStream
        {
            typedef CCustomMemoryStream inherited;

        private:

            size_t m_Capacity;

            void SetCapacity(size_t NewCapacity);

        protected:

            virtual void *Realloc(size_t &NewCapacity);

            size_t Capacity() const { return m_Capacity; };
            void Capacity(size_t Value) { SetCapacity(Value); };

        public:

            CMemoryStream();

            explicit CMemoryStream(size_t ASize): CMemoryStream() {
                CMemoryStream::SetSize(ASize);
            };

            ~CMemoryStream() override;

            inline static class CMemoryStream *Create() { return new CMemoryStream; };

            virtual void Clear();

            void LoadFromStream(CStream &Stream);
            void LoadFromFile(LPCTSTR lpszFileName);

            void SetSize(size_t NewSize) override;

            size_t Write(const void *Buffer, size_t Count) override;

        }; // CMemoryStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomStringStream ---------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CStringStream;
        class CCustomString;
        class CString;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCustomStringStream: public CStream
        {
            friend CStringStream;
            friend CCustomString;
            friend CString;

        private:

            LPTSTR m_Data;

            size_t m_Size;
            mutable size_t m_Position;

        protected:

            void SetData(LPTSTR Data, size_t Size);

            off_t GetSize() const override { return (off_t) m_Size; };

        public:

            CCustomStringStream();

            ~CCustomStringStream() override = default;

            inline off_t Seek(off_t Offset, CSeekOrigin Origin) const override { return CStream::Seek(Offset, Origin); };

            size_t Read(Pointer Buffer, size_t Count) const override;

            off_t Seek(off_t Offset, unsigned short Origin) const override;

            void SaveToStream(CStream &Stream) const;
            void SaveToFile(LPCTSTR lpszFileName) const;

            LPTSTR Data() { return m_Data; };
            LPCTSTR Data() const noexcept { return m_Data; };

            LPTSTR StrPos() { return m_Data + m_Position; };
            LPCTSTR StrPos() const noexcept { return m_Data + m_Position; };

        }; // CCustomStringStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringStream ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CStringStream: public CCustomStringStream
        {
            typedef CCustomStringStream inherited;

        private:

            size_t m_Capacity;

            void SetCapacity(size_t NewCapacity);

        protected:

            virtual LPTSTR Realloc(size_t &NewCapacity);

            void SetSize(size_t NewSize) override;

            void Capacity(size_t Value) { SetCapacity(Value); };

        public:

            CStringStream();

            ~CStringStream() override;

            virtual void Clear();

            virtual void SetLength(size_t NewLength) abstract;

            void LoadFromStream(CStream &Stream);

            void LoadFromFile(LPCTSTR lpszFileName);
            void LoadFromFile(const CString& FileName);

            size_t Write(const void *Buffer, size_t Count) override;

            size_t Capacity() const noexcept { return m_Capacity; };

        }; // CStringStream

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomString ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CCustomString: public CStringStream {
            typedef CStringStream inherited;

        private:

            size_t m_Length;

            void WriteStr(LPCTSTR Str, size_t Length = 0);

            void SetSize(size_t NewSize) override;

        protected:

            size_t GetLength() const noexcept { return m_Length; };

            void SetStr(LPCTSTR Str, size_t Length = 0);
            void AddStr(LPCTSTR Str, size_t Length = 0);

            void SetChar(TCHAR C, size_t Length = 1);
            void AddChar(TCHAR C, size_t Length = 1);

            LPCTSTR Str() const noexcept {
                if (Assigned(m_Data))
                    m_Data[m_Length] = '\0';
                return m_Data;
            };

        public:

            CCustomString();

            void Clear() override;

            void SetLength(size_t NewLength) override;

            size_t Length() const noexcept { return GetLength(); };

            size_t Size() const noexcept override { return GetSize(); };

            TCHAR GetChar(size_t Index) const;

            bool IsEmpty() const noexcept { return m_Data == nullptr; };

            size_t Truncate();
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CString ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define MaxFormatStringLength 2048
        //--------------------------------------------------------------------------------------------------------------

        class CString: public CCustomString {
            typedef TCHAR value_type;
            typedef size_t size_type;
            typedef value_type& reference;
            typedef const value_type& const_reference;

        private:

            size_t m_MaxFormatSize;

        public:

            CString();

            CString(const CString& S);

            CString(const std::string& str);

            CString(LPCTSTR Str, size_t Length = 0);

            CString(TCHAR C);

            explicit CString(CStream &Stream);

            explicit CString(size_t Length, TCHAR C);

            void Create(const CString& S);
            void Create(LPCTSTR Str, size_t Length = 0);
            void Create(TCHAR C);
            void Create(size_t Length, TCHAR C);

            void Append(const CString& S);
            void Append(LPCTSTR Str, size_t Length = 0);
            void Append(TCHAR C);
            void Append(size_t Length, TCHAR C);

            size_t Copy (LPTSTR Str, size_t Len, size_t Pos = 0) const;

            CString &Format(LPCTSTR pszFormat, ...);
            CString &Format(LPCTSTR pszFormat, va_list argList);

            size_t MaxFormatSize() const { return m_MaxFormatSize; }
            CString &MaxFormatSize(size_t Value);

            size_t Find(const CString& S, size_t Pos = 0) const;
            size_t Find(LPCTSTR Str, size_t Pos = 0) const;
            size_t Find(LPCTSTR Str, size_t Pos, size_t Length) const;
            size_t Find(TCHAR C, size_t Pos = 0) const;

            value_type GetFront() const { return GetChar(0); };
            value_type GetBack() const { return GetChar(Length() - 1); };

            int Compare(const CString& S) const;
            int Compare(const std::string& S) const;
            int Compare(LPCTSTR Str) const;

            CString Trim(TCHAR TrimChar = ' ') const;
            CString TrimLeft(TCHAR TrimChar = ' ') const;
            CString TrimRight(TCHAR TrimChar = ' ') const;

            CString SubString(size_t StartIndex, size_t Length = npos) const;

            CString Upper() const;
            CString Lower() const;

            void clear() { Clear(); };

            bool empty() const noexcept { return IsEmpty(); };

            size_type size() const noexcept { return GetSize(); };
            size_type length() const noexcept { return Length(); };
            size_type capacity() const noexcept { return Capacity(); };

            value_type front() const { return GetFront(); };
            value_type back() const { return GetBack(); };

            value_type at(size_t Index) const { return GetChar(Index); };

            CString substr (size_t pos = 0, size_t len = npos) const { return SubString(pos, len); };

            LPCTSTR data() const noexcept { return Data(); }

            LPCTSTR c_str() const noexcept { return Str(); }

            template <class T>
            int compare(T Value) const { return Compare(Value); }

            template <class T>
            void append (T Value) { Append(Value); }

            template <class T>
            size_t find (T Value, size_t Pos = 0) const { return Find(Value, Pos); }
            size_t find (LPCTSTR Str, size_t Pos, size_t Length) const { return Find(Str, Pos, Length); }

            operator std::basic_string<char>() const {
                if (IsEmpty())
                    return {};
                return { Data(), Size() };
            };

            explicit operator const char *() const {
                return IsEmpty() ? "" : c_str();
            };

            CString& operator= (const CString& S) {
                if (this != &S) {
                    Clear();
                    Create(S);
                }
                return *this;
            };

            CString& operator= (LPCTSTR Value) {
                Clear();
                Create(Value);
                return *this;
            };

            CString& operator= (TCHAR Value) {
                Clear();
                Create(Value);
                return *this;
            };

            CString& operator= (const std::string& str) {
                Clear();
                Create(str.c_str());
                return *this;
            }

            CString& operator= (bool Value) {
                Clear();
                Create(Value ? "true" : "false");
                return *this;
            };

            CString& operator= (int Value) {
                Clear();
                TCHAR szValue[_INT_T_LEN + 1] = {0};
                Create(IntToStr(Value, szValue, _INT_T_LEN));
                return *this;
            };

            CString& operator= (float Value) {
                Clear();
                TCHAR szValue[_INT_T_LEN + 1] = {0};
                Create(FloatToStr(Value, szValue, _INT_T_LEN));
                return *this;
            };

            CString& operator= (double Value) {
                Clear();
                TCHAR szValue[_INT_T_LEN + 1] = {0};
                Create(FloatToStr(Value, szValue, _INT_T_LEN));
                return *this;
            };

            CString& operator+= (const CString& S) {
                if (this != &S) Append(S);
                return *this;
            };

            friend CString operator+= (const CString& LS, const CString& RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            };

            CString& operator+= (LPCTSTR Value) {
                Append(Value);
                return *this;
            };

            CString& operator+= (TCHAR Value) {
                Append(1, Value);
                return *this;
            };

            template <typename T>
            friend CString operator+= (T LS, const CString& RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            template <typename T>
            friend CString operator+= (const CString& LS, T RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            friend CString operator+ (const CString& LS, const CString& RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            friend CString operator+ (LPCTSTR LS, const CString& RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            friend CString operator+ (TCHAR LS, const CString& RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            friend CString operator+ (const CString& LS, LPCTSTR RS) {
                CString S(LS);
                S.Append(RS);
                return S;
            }

            friend CString operator+ (const CString& LS, TCHAR RS) {
                CString S(LS);
                S.Append(1, RS);
                return S;
            }

            template <class T>
            inline bool operator== (T Value) { return Compare(Value) == 0; }

            template <class T>
            inline bool operator== (T Value) const { return Compare(Value) == 0; }

            template <class T>
            inline bool operator!= (T Value) { return Compare(Value) != 0; }

            template <class T>
            inline bool operator!= (T Value) const { return Compare(Value) != 0; }

            template <class T>
            inline bool operator< (T Value) { return Compare(Value) < 0; }

            template <class T>
            inline bool operator< (T Value) const { return Compare(Value) < 0; }

            template <class T>
            inline bool operator> (T Value) { return Compare(Value) > 0; }

            template <class T>
            inline bool operator> (T Value) const { return Compare(Value) > 0; }

            template <class T>
            inline bool operator<= (T Value) { return Compare(Value) <= 0; }

            template <class T>
            inline bool operator<= (T Value) const { return Compare(Value) <= 0; }

            template <class T>
            inline bool operator>= (T Value) { return Compare(Value) >= 0; }

            template <class T>
            inline bool operator>= (T Value) const { return Compare(Value) >= 0; }

            CString& operator<< (const CString& S) {
                if (this != &S) Append(S);
                return *this;
            }

            CString& operator<< (const std::string& s) {
                Append(s.c_str());
                return *this;
            }

            CString& operator<< (int Value) {
                TCHAR szValue[_INT_T_LEN + 1] = {0};
                IntToStr(Value, szValue, _INT_T_LEN);
                Append(szValue);
                return *this;
            }

            CString& operator<< (size_t Value) {
                TCHAR szValue[_INT_T_LEN + 1] = {0};
                IntToStr(Value, szValue, _INT_T_LEN, 16);
                Append(szValue);
                return *this;
            }

            template <class T>
            CString& operator<< (T Value) {
                Append(Value);
                return *this;
            }

            template <class T>
            friend CString& operator>> (T LS, CString& RS) {
                RS.Append(LS);
                return RS;
            }

            friend tostream& operator<< (tostream& Out, const CString& S) {
                return Out << (S.IsEmpty() ? "" : S.c_str());
            }

            friend tistream& operator>> (tistream& In, CString& S) {
                TCHAR C;
                while (In.get(C) && C != '\n')
                    S.Append(C);
                return In;
            }

            reference operator[] (size_type Index) { return m_Data[Index]; }
            const_reference operator[] (size_type Index) const { return m_Data[Index]; }

            static const size_type npos = static_cast<size_t>(-1);
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CStrings --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define sLineBreak          _T("\n")

        #define sDelimiter          _T(',')
        #define sQuoteChar          _T('"')
        #define sNameValueSeparator _T('=')

        class CStrings;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CStrings: public CPersistent {
            typedef CPersistent inherited;
            typedef LPCTSTR reference;

        private:

            LPCTSTR m_LineBreak;

            TCHAR m_Delimiter;
            TCHAR m_QuoteChar;

            CString m_NameValueSeparator;

            bool m_StrictDelimiter;
            int m_UpdateCount;

            virtual CString GetName(int Index) const abstract;

            virtual CString GetValue(const CString &Name) const abstract;
            virtual CString GetValue(reference Name) const abstract;

            void SetValue(const CString &Name, const CString &Value);
            void SetValue(reference Name, reference Value);

            CString GetValueFromIndex(int Index) const;

            void SetValueFromIndex(int Index, const CString &Value);
            void SetValueFromIndex(int Index, reference Value);

        protected:

            TCHAR GetDelimiter() const;
            void SetDelimiter(TCHAR Value);
            LPCTSTR GetLineBreak() const;
            void SetLineBreak(LPCTSTR Value);
            TCHAR GetQuoteChar() const;
            void SetQuoteChar(TCHAR Value);
            const CString &GetNameValueSeparator() const;
            void SetNameValueSeparator(const CString &Value);
            bool GetStrictDelimiter() const;
            void SetStrictDelimiter(bool Value);

            static void Error(const CString &Msg, int Data);
            virtual CString &Get(int Index) abstract;
            virtual const CString &Get(int Index) const abstract;
            virtual int GetCapacity() const noexcept;
            virtual int GetCount() const noexcept abstract;
            virtual CObject* GetObject(int Index) const;
            virtual CObject* GetObject(const CString& Name) const;
            virtual CObject* GetObject(reference Name) const;
            virtual void Put(int Index, const CString &S);
            virtual void Put(int Index, reference Str);
            virtual void PutObject(int Index, CObject* AObject);
            virtual void SetCapacity(int NewCapacity);
            void SetText(const CString &Value);
            void SetUpdateState(bool Updating);
            int UpdateCount() const { return m_UpdateCount; };
            static int CompareStrings(const CString &S1, const CString &S2);

        public:

            CStrings();

            ~CStrings() override = default;

            virtual int Add(const CString &S);
            virtual int Add(reference Str);
            virtual int Add(TCHAR C);
            virtual int AddObject(const CString &S, CObject* AObject);
            virtual int AddObject(reference Str, CObject* AObject);
            virtual int AddObject(TCHAR C, CObject* AObject);
            virtual int AddPair(const CString &Name, const CString &Value);
            virtual int AddPair(reference Name, reference Value);
            virtual int AddPair(reference Name, const CString &Value);
            virtual int AddPair(const CString &Name, reference Value);
            void Append(const CString &S);
            void Append(reference Str);
            virtual void AddStrings(const CStrings& Strings);
            virtual void Assign(const CStrings& Source);
            virtual void SetStrings(const CStrings& Source);
            void BeginUpdate();
            virtual void Clear() abstract;
            virtual void Delete(int Index) abstract;
            void EndUpdate();
            bool Equals(CStrings* Strings);
            virtual void Exchange(int Index1, int Index2);
            virtual bool GetTextStr(LPTSTR Buffer, size_t &SizeBuf);
            virtual CString GetText() const;
            virtual int IndexOf(const CString &S) const;
            virtual int IndexOf(reference Str) const;
            virtual int IndexOfName(const CString &Name) const;
            virtual int IndexOfName(reference Name) const;
            virtual int IndexOfObject(CObject* AObject) const;
            virtual void Insert(int Index, const CString &S) abstract;
            virtual void Insert(int Index, reference Str) abstract;
            virtual void Insert(int Index, TCHAR C) abstract;
            virtual void InsertObject(int Index, const CString &S, CObject* AObject);
            virtual void InsertObject(int Index, reference Str, CObject* AObject);
            virtual void InsertObject(int Index, TCHAR C, CObject* AObject);
            virtual void LoadFromFile(LPCTSTR lpszFileName);
            virtual void LoadFromStream(CStream &Stream);
            virtual void Move(int CurIndex, int NewIndex);
            virtual void SaveToFile(LPCTSTR lpszFileName) const;
            virtual void SaveToStream(CStream &Stream) const;
            virtual void SetTextStr(LPCTSTR Text, size_t Size);

            int Capacity() const noexcept { return GetCapacity(); };
            void Capacity(int NewCapacity) { SetCapacity(NewCapacity); };

            int Count() const noexcept { return GetCount(); };
            TCHAR Delimiter() const { return GetDelimiter(); };
            void Delimiter(TCHAR Value) { SetDelimiter(Value); };

            LPCTSTR LineBreak() const { return GetLineBreak(); };
            void LineBreak(LPCTSTR Value) { SetLineBreak(Value); };

            CString Names(int Index) const { return GetName(Index); };

            CObject *Objects(int Index) const { return GetObject(Index); };
            CObject *Objects(const CString &Name) const { return GetObject(Name); };
            CObject *Objects(reference Name) const { return GetObject(Name); };

            void Objects(int Index, CObject *Value) { return PutObject(Index, Value); };

            TCHAR QuoteChar() const { return GetQuoteChar(); }
            void QuoteChar(TCHAR Value) { SetQuoteChar(Value); };

            CString Values(int Index) const { return GetValueFromIndex(Index); };
            CString Values(const CString &Name) const { return GetValue(Name); };

            void Values(const CString &Name, const CString &Value) { SetValue(Name, Value); };
            void Values(reference Name, reference Value) { SetValue(Name, Value); };

            CString ValueFromIndex(int Index) const { return GetValueFromIndex(Index); };

            void ValueFromIndex(int Index, const CString &Value) { SetValueFromIndex(Index, Value); };
            void ValueFromIndex(int Index, reference Value) { SetValueFromIndex(Index, Value); };

            const CString &NameValueSeparator() const { return GetNameValueSeparator(); };
            void NameValueSeparator(const CString &Value) { SetNameValueSeparator(Value); };

            bool StrictDelimiter() const { return GetStrictDelimiter(); };
            void StrictDelimiter(bool Value) { SetStrictDelimiter(Value); };

            CString &Strings(int Index) { return Get(Index); };
            const CString &Strings(int Index) const { return Get(Index); };

            void Strings(int Index, const CString &Value) { return Put(Index, Value); };

            CString Text() const { return GetText(); };
            void Text(const CString &Value) { SetText(Value); };

            CString &First() { return Get(0); };
            const CString &First() const { return Get(0); };

            CString &front() { return Get(0); };
            const CString &front() const { return Get(0); };

            CString &Last() { return Get(GetCount() - 1); };
            const CString &Last() const { return Get(GetCount() - 1); };

            CString &back() { return Get(GetCount() - 1); };
            const CString &back() const { return Get(GetCount() - 1); };

            CStrings &operator=(const CStrings &Strings) {
                if (&Strings != this) {
                    Assign(Strings);
                }
                return *this;
            }

            virtual CStrings &operator=(const CString &String) {
                if (!String.IsEmpty()) {
                    SetText(String);
                }
                return *this;
            }

            virtual CStrings &operator=(reference Str) {
                if (Str != nullptr) {
                    size_t Size = strlen(Str);
                    SetTextStr(Str, Size);
                }
                return *this;
            }

            virtual CStrings& operator<< (reference Str) {
                Add(Str);
                return *this;
            }

            virtual CStrings& operator<< (const CString& S) {
                Add(S);
                return *this;
            }

            virtual CStrings& operator<< (const std::string& s) {
                Add(s.c_str());
                return *this;
            }

            virtual CString &operator[] (int Index) { return Strings(Index); }
            virtual const CString &operator[] (int Index) const { return Strings(Index); }

            virtual CString operator[] (const CString &Name) const { return Values(Name); }
            virtual CString operator[] (reference Name) const { return Values(Name); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CStringList -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CStringList;
        //--------------------------------------------------------------------------------------------------------------

        typedef struct StringItem {
            CString     String;
            CObject    *Object = nullptr;
        } CStringItem, *PStringItem;
        //--------------------------------------------------------------------------------------------------------------

        typedef PStringItem *PStringItemList;
        //--------------------------------------------------------------------------------------------------------------

        typedef int (*PStringListSortCompare)(CStringList *List, int Index1, int Index2);
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CStringList: public CHeapComponent, public CStrings {
            typedef CStrings inherited;
            typedef LPCTSTR reference;

        private:

            PStringItemList m_pList;

            int m_nCount;
            int m_nCapacity;

            bool m_fOwnsObjects;

            virtual void Grow();
            //void QuickSort(int L, int R, PStringListSortCompare SCompare);

            CString GetName(int Index) const override;

            CString GetValue(const CString &Name) const override;
            CString GetValue(reference Name) const override;

        protected:

            CString &Get(int Index) override;
            const CString &Get(int Index) const override;

            void Put(int Index, const CString &S) override;
            void Put(int Index, reference Str) override;

            int GetCapacity() const noexcept override { return m_nCapacity; }
            void SetCapacity(int NewCapacity) override;

            int GetCount() const noexcept override { return m_nCount; }

            CObject* GetObject(int Index) const override;
            void PutObject(int Index, CObject* AObject) override;

            virtual void InsertItem(int Index, const CString &S, CObject *AObject);
            virtual void InsertItem(int Index, reference Str, CObject *AObject);
            virtual void InsertItem(int Index, TCHAR C, CObject *AObject);

        public:

            CStringList();

            CStringList(const CStringList& List): CStringList() {
                if (&List != this) {
                    CStringList::Assign(List);
                }
            }

            explicit CStringList(const CString& String): CStringList() {
                if (!String.IsEmpty()) {
                    SetText(String);
                }
            }

            explicit CStringList(bool AOwnsObjects);

            inline static class CStringList* Create(bool AOwnsObjects = false) { return new CStringList(AOwnsObjects); };

            ~CStringList() override;

            void Assign(const CStrings& Source) override;

            void Clear() override;

            virtual void Update(int Index);

            void Delete(int Index) override;

            int Add(const CString &S) override;
            int AddObject(const CString &S, CObject* AObject) override;

            int Add(reference Str) override;
            int AddObject(reference Str, CObject* AObject) override;

            int Add(TCHAR C) override;
            int AddObject(TCHAR C, CObject* AObject) override;

            void Insert(int Index, const CString &S) override;
            void InsertObject(int Index, const CString &S, CObject* AObject) override;

            void Insert(int Index, reference Str) override;
            void InsertObject(int Index, reference Str, CObject* AObject) override;

            void Insert(int Index, TCHAR C) override;
            void InsertObject(int Index, TCHAR C, CObject* AObject) override;

            bool OwnsObjects() const { return m_fOwnsObjects; };

            CStringList &operator=(const CStringList &Strings) {
                if (&Strings != this) {
                    Assign(Strings);
                }
                return *this;
            }

            CStringList &operator=(const CString &String) override {
                if (!String.IsEmpty()) {
                    SetText(String);
                }
                return *this;
            }

            CStringList &operator=(reference Str) override {
                if (Str != nullptr) {
                    size_t Size = strlen(Str);
                    SetTextStr(Str, Size);
                }
                return *this;
            }

            CStringList &operator<<(const CStringList &Strings) {
                if (&Strings != this) {
                    AddStrings(Strings);
                }
                return *this;
            }

            CStringList& operator<< (reference Str) override {
                Add(Str);
                return *this;
            }

            CStringList& operator<< (const CString& S) override {
                Add(S);
                return *this;
            }

            CStringList& operator<< (const std::string& s) override {
                Add(s.c_str());
                return *this;
            }

        }; // CStringList

        //--------------------------------------------------------------------------------------------------------------

        //-- CFile -----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        #define INVALID_FILE         (-1)
        #define FILE_ERROR           (-1)
        //--------------------------------------------------------------------------------------------------------------

        #define FILE_RDONLY          O_RDONLY
        #define FILE_WRONLY          O_WRONLY
        #define FILE_RDWR            O_RDWR
        #define FILE_CREATE_OR_OPEN  O_CREAT
        #define FILE_OPEN            0
        #define FILE_TRUNCATE        (O_CREAT|O_TRUNC)
        #define FILE_APPEND          (O_WRONLY|O_APPEND)
        #define FILE_NONBLOCK        O_NONBLOCK
        //--------------------------------------------------------------------------------------------------------------

        #define FILE_DEFAULT_ACCESS  0644
        #define FILE_OWNER_ACCESS    0600
        //--------------------------------------------------------------------------------------------------------------

#ifdef _GLIBCXX_FUNCTIONAL
        typedef std::function<void (Pointer Sender, int Error, LPCTSTR lpFormat, va_list args)> COnFilerErrorEvent;
#else
        typedef void (* COnFilerErrorEvent) (Pointer Sender, int Error, LPCTSTR lpFormat, va_list args);
#endif
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CFile: public CObject {

            int m_hHandle;

            int m_iFlags;

            off_t m_uOffset;

            CString m_FileName;

            COnFilerErrorEvent m_OnFilerError;

            size_t GetSize() const;

        private:

            void DoFilerError(int AError, LPCTSTR lpFormat, ...);

        public:

            explicit CFile(const CString &FileName, int AFlags);

            ~CFile() override;

            int Handle() const { return m_hHandle; }

            const CString &FileName() const { return m_FileName; }

            void Open();
            void Close(bool ASafe = false);

            ssize_t Read(char *buf, ssize_t size, off_t offset);
            ssize_t Write(char *buf, ssize_t size, off_t offset);

            size_t Size() const { return GetSize(); }
            off_t Offset() const { return m_uOffset; }

            const COnFilerErrorEvent &OnFilerError() const { return m_OnFilerError; };
            void OnFilerError(COnFilerErrorEvent && Value) { m_OnFilerError = Value; };

        }; // class CFile

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueueItem ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CQueueItem: public CCollectionItem
        {
        private:

            Pointer m_pQueue;
            CList *m_pItems;

        protected:

            Pointer Get(int Index);
            void Put(int Index, Pointer Value);

        public:

            CQueueItem(CCollection *ACollection, Pointer AQueue);

            ~CQueueItem() override;

            int Add(Pointer Item);
            void Insert(int Index, Pointer Item);
            void Delete(int Index);

            int IndexOf(Pointer Item);
            int Remove(Pointer Item);

            Pointer First();
            Pointer Last();

            Pointer Queue() { return m_pQueue; }

            int Count() { return m_pItems->Count(); }

            Pointer Item(int Index) { return Get(Index); }
            void Item(int Index, Pointer Value) { Put(Index, Value); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueue ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CQueue: public CCollection {
            typedef CCollection inherited;

        protected:

            CQueueItem *GetItem(int Index) const override;
            void SetItem(int Index, const CQueueItem *Item);

        public:

            CQueue();

            int IndexOf(Pointer Queue);
            int Remove(Pointer Queue);

            CQueueItem *Add(Pointer Queue);
            CQueueItem *Insert(int Index, Pointer Queue);
            CQueueItem *First();
            CQueueItem *Last();

            int AddToQueue(Pointer Queue, Pointer P);
            void InsertToQueue(Pointer Queue, int Index, Pointer P);
            void RemoveFromQueue(Pointer Queue, Pointer P);

            Pointer FirstItem(Pointer Queue);
            Pointer LastItem(Pointer Queue);

            CQueueItem *Items(int Index) const override { return GetItem(Index); }
            void Items(int Index, CQueueItem *Value) { SetItem(Index, Value); }

            CQueueItem *operator [] (int Index) const override { return GetItem(Index); };
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSysError -------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSysError: public CStringList {
        private:

            CString     m_UnknownError;

        protected:

            void Create();

        public:

            CSysError(): CStringList(), m_UnknownError(_T("Unknown Error")) { Create(); };

            ~CSysError() override = default;

            static class CSysError *CreateSysError() { return new CSysError(); };

            static void DestroySysError() { delete GSysError; };

            const CString& StrError(int AError);
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CSysErrorComponent ----------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CSysErrorComponent {
        public:
            CSysErrorComponent();
            ~CSysErrorComponent();
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CThread ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CThread;
        //--------------------------------------------------------------------------------------------------------------

        typedef void (CThreadMethod)();
        //--------------------------------------------------------------------------------------------------------------

#pragma pack (push, 4)

        typedef struct SynchronizeRecord {
            PVOID Thread;
            CThreadMethod *Method;
            PVOID SynchronizeException;

            SynchronizeRecord() {
                Thread = nullptr;
                Method = nullptr;
                SynchronizeException = nullptr;
            }

            SynchronizeRecord(const SynchronizeRecord &Record): SynchronizeRecord() {
                Assign(Record);
            }

            void Assign(const SynchronizeRecord &Record) {
                Thread = Record.Thread;
                Method = Record.Method;
                SynchronizeException = Record.SynchronizeException;
            }

            SynchronizeRecord& operator= (const SynchronizeRecord &Record) {
                if (this != &Record) {
                    Assign(Record);
                }
                return *this;
            };

        } CSynchronizeRecord, *PSynchronizeRecord;
        //--------------------------------------------------------------------------------------------------------------

#pragma pack (pop)

        typedef struct tagSyncProc {
            CSynchronizeRecord SyncRec;
            pthread_cond_t Signal;
        } CSyncProc, *PSyncProc;
        //--------------------------------------------------------------------------------------------------------------

        LPVOID ThreadProc(LPVOID lpParameter);
        //--------------------------------------------------------------------------------------------------------------

        enum CThreadPriority { tpIdle, tpLowest, tpLower, tpNormal, tpHigher, tpHighest, tpTimeCritical };
        //--------------------------------------------------------------------------------------------------------------

        void AddThread();
        void RemoveThread();
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CThread: public CObject
        {
            friend LPVOID ThreadProc(LPVOID lpParameter);
            //friend bool CheckSynchronize(int Timeout = 0);

        private:

            pthread_t           m_hHandle;
            pthread_attr_t      m_hAttr {};

            pthread_mutex_t     m_SuspendMutex {};
            pthread_cond_t      m_ResumeCond {};

            pid_t               m_nThreadId;

            CSynchronizeRecord  m_Synchronize;
            CNotifyEvent        m_OnTerminate;

            int                 m_nReturnValue;

            bool                m_bCreateSuspended;
            bool                m_bTerminated;
            bool                m_bSuspended;
            bool                m_bFreeOnTerminate;
            bool                m_bFinished;

            void CallOnTerminate();

            static void CreateSyncList();
            static void Synchronize(const CSynchronizeRecord &ASyncRec);

            static CThreadPriority GetPriority();
            void SetPriority(CThreadPriority Value);

            void SetSuspended(bool Value);

        protected:

            void AfterConstruction();
            static void CheckThreadError(int ErrCode);
            static void CheckThreadError(bool Success);

            virtual void DoTerminate();
            virtual void Execute() abstract;
            void Synchronize(CThreadMethod *AMethod);

            int ReturnValue() const { return m_nReturnValue; };
            void ReturnValue(int Value) { m_nReturnValue = Value; };

            bool Terminated() const { return m_bTerminated; };

        public:

            explicit CThread(bool CreateSuspended);
            ~CThread() override;

            void Resume();
            void Suspend();

            static void Lock();
            static void Unlock();

            virtual void Terminate();
            int WaitFor();

            static void Synchronize(CThread *AThread, CThreadMethod *AMethod);

            pthread_t Handle() const { return m_hHandle; };

            pid_t ThreadId() const { return m_nThreadId; };

            bool Suspended() const { return m_bSuspended; };
            void Suspended(bool Value) { SetSuspended(Value); }

            static CThreadPriority Priority() { return GetPriority(); }
            void Priority(CThreadPriority Value) { SetPriority(Value); }

            bool FreeOnTerminate() const { return m_bFreeOnTerminate; };
            void FreeOnTerminate(bool Value) { m_bFreeOnTerminate = Value; };

            const CNotifyEvent& OnTerminate() { return m_OnTerminate; };
            void OnTerminate(CNotifyEvent && Value) { m_OnTerminate = Value; };

        }; // CThread

        //--------------------------------------------------------------------------------------------------------------

        //-- CThreadList -----------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CThreadList
        {

        private:

            CList               m_List;
            pthread_mutex_t     m_Lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
            CDuplicates         m_Duplicates;

        public:

            CThreadList();
            virtual ~CThreadList() = default;

            void Add(CThread *Item);
            void Clear();

            const CList &LockList();
            CList *ptrLockList();

            void Remove(CThread *Item);
            void UnlockList();

            const CList &List() const { return m_List; };

            CDuplicates Duplicates() { return m_Duplicates; };
            void Duplicates(CDuplicates Value) { m_Duplicates = Value; };

        }; // CThreadList

    }
}

using namespace Delphi::Classes;
}
#endif //DELPHI_CLASSES_HPP