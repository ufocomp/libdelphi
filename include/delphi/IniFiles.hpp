/*++

Library name:

  libdelphi

Module Name:

  IniFiles.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_INIFILES_HPP
#define DELPHI_INIFILES_HPP

#define ARRAY_BOOLEAN_STRINGS { _T("false"), _T("true"), _T("0"), _T("1"), _T("no"), _T("yes"), _T("off"), _T("on") }

extern "C++" {

namespace Delphi {

    namespace IniFiles {

        class CCustomIniFile;

        //--------------------------------------------------------------------------------------------------------------

        //-- CCustomIniFile --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CCustomIniFile *Sender, LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszValue,
                LPCTSTR lpszDefault, int Line)> COnIniFileParseError;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CCustomIniFile: public CObject {
        private:

            CString m_FileName;

            COnIniFileParseError m_OnIniFileParseError;

        protected:

            class CKeyLine: public CObject {
            private:
                int m_Line;
            public:
                explicit CKeyLine(int Line): CObject(), m_Line(Line) {};
                int Line() const noexcept { return m_Line; };
            };

            const TCHAR SectionNameSeparator = '\\';

            virtual void InternalReadSections(LPCTSTR lpszSectionName, CStrings *Strings,
                    bool SubSectionNamesOnly, bool Recurse) const;

            virtual int GetKeyLine(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName) const abstract;

            void DoIniFileParseError(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszDefault) const;

        public:

            explicit CCustomIniFile(const CString &FileName);

            ~CCustomIniFile() override = default;

            bool SectionExists(LPCTSTR lpszSectionName) const;
            virtual bool ValueExists(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName) const;

            virtual CString ReadString(const CString &SectionName, const CString &KeyName, const CString &Default) const abstract;

            virtual CString ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault) const abstract;

            virtual void ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault,
                                CString &ReturnedString) const abstract;

            virtual DWORD ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault,
                    LPTSTR lpszReturnedString, DWORD nSize) const abstract;

            virtual BOOL WriteString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszString) abstract;
            virtual BOOL WriteString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, const CString &String) abstract;
            virtual BOOL WriteString(const CString &SectionName, const CString &KeyName, const CString &String) abstract;

            virtual INT ReadInteger(const CString &SectionName, const CString &KeyName, INT Default) const;
            virtual INT ReadInteger(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, INT Default) const;
            virtual BOOL WriteInteger(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, INT Value);

            virtual bool ReadBool(const CString &SectionName, const CString &KeyName, bool Default) const;
            virtual bool ReadBool(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, bool Default) const;
            virtual BOOL WriteBool(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, bool Value);

            virtual DOUBLE ReadFloat(const CString &SectionName, const CString &KeyName, DOUBLE Default) const;
            virtual DOUBLE ReadFloat(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, DOUBLE Default) const;
            virtual BOOL WriteFloat(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, DOUBLE Value);

            virtual CDateTime ReadDateTime(const CString &SectionName, const CString &KeyName, CDateTime Default) const;
            virtual CDateTime ReadDateTime(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CDateTime Default) const;
            virtual BOOL WriteDateTime(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CDateTime Value);

            virtual void ReadSection(LPCTSTR lpszSectionName, CStrings *Strings) const abstract;

            virtual void ReadSections(CStrings *Strings) const abstract;
            virtual void ReadSections(LPCTSTR lpszSectionName, CStrings *Strings) const;
            virtual void ReadSubSections(LPCTSTR lpszSectionName, CStrings *Strings, bool Recurse);

            virtual void ReadSectionValues(LPCTSTR lpszSectionName, CStrings *Strings) const abstract;

            virtual BOOL EraseSection(LPCTSTR lpszSectionName) abstract;
            virtual BOOL EraseIniFile() abstract;

            virtual void DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName) abstract;

            virtual void UpdateFile() abstract;

            CString &FileName() { return m_FileName; };
            const CString &FileName() const { return m_FileName; };

            void OnIniFileParseError(COnIniFileParseError && Value) { m_OnIniFileParseError = Value; };

        }; // class CCustomIniFile

        //--------------------------------------------------------------------------------------------------------------

        //- CStringHash ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CStringHash;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CStringHash: public CObject {
            typedef CObject inherited;

        private:

            size_t m_BucketSize;

            CList **m_Buckets;

            void ClearList(CList *List);

        protected:

            struct CHashItem {
            public:
                CString Key;
                int Value;

                CHashItem& operator= (const CHashItem& Item) = default;
            };

            int Find(const CString &Key, CHashItem* &AItem);
            int Find(LPCTSTR Key, CHashItem* &AItem);

            virtual unsigned HashOf(const CString &Key);
            virtual unsigned HashOf(LPCTSTR Key);

        public:

            explicit CStringHash(size_t Size = 256);

            ~CStringHash() override;

            inline static class CStringHash* Create(size_t Size = 256) { return new CStringHash(Size); };

            void Add(const CString &Key, int Value);
            void Add(LPCTSTR Key, int Value);

            void Clear();

            void Remove(const CString &Key);
            void Remove(LPCTSTR Key);

            bool Modify(const CString &Key, int Value);
            bool Modify(LPCTSTR Key, int Value);

            int ValueOf(const CString &Key);
            int ValueOf(LPCTSTR Key);
        };

        //--------------------------------------------------------------------------------------------------------------

        //- CHashedStringList ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHashedStringList;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CHashedStringList : public CStringList {
            typedef CStringList inherited;

        private:

            mutable CStringHash *m_ValueHash;
            mutable CStringHash *m_NameHash;

            mutable bool m_ValueHashValid;
            mutable bool m_NameHashValid;

            void UpdateValueHash() const;
            void UpdateNameHash() const;

        public:

            CHashedStringList(): CStringList() {
                m_ValueHash = nullptr;
                m_NameHash = nullptr;

                m_ValueHashValid = false;
                m_NameHashValid = false;
            };

            explicit CHashedStringList(bool OwnsObjects): CStringList(OwnsObjects) {
                m_ValueHash = nullptr;
                m_NameHash = nullptr;

                m_ValueHashValid = false;
                m_NameHashValid = false;
            };

            static class CHashedStringList* Create(bool OwnsObjects = false) {
                return new CHashedStringList(OwnsObjects);
            };

            ~CHashedStringList() override;

            int IndexOf(const CString &S) const override;
            int IndexOf(LPCTSTR S) const override;

            int IndexOfName(const CString &Name) const override;
            int IndexOfName(LPCTSTR Name) const override;

            void Changed();
        };

        //--------------------------------------------------------------------------------------------------------------

        //- CMemIniFile ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CMemIniFile;
        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CMemIniFile : public CCustomIniFile {
            typedef CCustomIniFile inherited;

        private:

            CStrings *m_Sections;

            bool m_Modified;
            bool m_AutoSave;
            bool m_CaseSensitive;

            CStrings *AddSection(LPCTSTR lpszSectionName);

            bool GetCaseSensitive() const;
            void SetCaseSensitive(bool Value);

            bool GetModified() const { return m_Modified; };
            void SetModified(bool Value) { m_Modified = Value; };

            bool GetAutoSave() const { return m_AutoSave; };
            void SetAutoSave(bool Value) { m_AutoSave = Value; };

        protected:

            void LoadValues();

            int GetKeyLine(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName) const override;

        public:

            explicit CMemIniFile(const CString &FileName);

            ~CMemIniFile() override;

            CString ReadString(const CString &SectionName, const CString &KeyName, const CString &Default) const override;

            CString ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault) const override;

            void ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault,
                    CString &ReturnedString) const override;

            DWORD ReadString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszDefault,
                             LPTSTR lpszReturnedString, DWORD nSize) const override;

            BOOL WriteString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszString) override;
            BOOL WriteString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, const CString &String) override;
            BOOL WriteString(const CString &SectionName, const CString &KeyName, const CString &String) override;

            void Clear();

            void DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName) override;

            BOOL EraseSection(LPCTSTR lpszSectionName) override;

            void GetStrings(CStrings *List);

            void ReadSection(LPCTSTR lpszSectionName, CStrings *Strings) const override;

            void ReadSections(CStrings *Strings) const override;

            void ReadSectionValues(LPCTSTR lpszSectionName, CStrings *Strings) const override;

            void Rename(const CString &NewFile, bool Reload);

            void SetStrings(CStrings *List);

            void UpdateFile() override;

            bool CaseSensitive() { return GetCaseSensitive(); };
            void CaseSensitive(bool Value) { SetCaseSensitive(Value); };

            bool Modified() { return GetModified(); };
            void Modified(bool Value) { SetModified(Value); };

            bool AutoSave() { return GetAutoSave(); };
            void AutoSave(bool Value) { SetAutoSave(Value); };

            inline void ReadSections(LPCTSTR lpszSectionName, CStrings *Strings) const override {
                CCustomIniFile::ReadSections(lpszSectionName, Strings);
            }

            CStrings &Sections(int Index);
            const CStrings &Sections(int Index) const;

            CStrings &Sections(LPCTSTR Section);
            const CStrings &Sections(LPCTSTR Section) const;

            virtual CMemIniFile &operator=(const CStrings &Strings) {
                if (Strings.Count() > 0) {
                    SetStrings((CStrings *) &Strings);
                }
                return *this;
            }

            CStrings &operator[] (int Index) { return Sections(Index); }
            const CStrings &operator[] (int Index) const { return Sections(Index); }

            const CStrings &operator[] (LPCTSTR Section) const { return Sections(Section); }

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CIniFile --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class LIB_DELPHI CIniFile : public CMemIniFile {
            typedef CMemIniFile inherited;

        public:

            explicit CIniFile(const CString &FileName): CMemIniFile(FileName) {};

            ~CIniFile() override = default;

            inline static class CIniFile* Create(const CString &FileName) { return new CIniFile(FileName); };

            BOOL EraseIniFile() override;

        };
    }
}

using namespace Delphi::IniFiles;
}


#endif //DELPHI_INIFILES_HPP
