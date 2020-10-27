/*++

Library name:

  libdelphi

Module Name:

  Message.hpp

Notices:

  Message class

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_MESSAGE_HPP
#define DELPHI_MESSAGE_HPP
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Message {

        //--------------------------------------------------------------------------------------------------------------

        //-- CMessage --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CMessage;

        typedef std::function<void (const CMessage &Message)> COnMessageEvent;
        typedef std::function<void (const CMessage &Message, const CString &Error)> COnMessageErrorEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CMessage: public CObject {
        private:

            CString m_MsgId;
            CString m_MessageId;

            CString m_From;
            CStringList m_To;

            CString m_Subject;
            CStringList m_Body;

            CString m_Content;

            bool m_Submitted;

            COnMessageEvent m_OnDone;
            COnMessageErrorEvent m_OnFail;

        protected:

            void DoDone();
            void DoFail(const CString &Error);

        public:

            CMessage();

            ~CMessage() override = default;

            void Assign(const CMessage& Message);

            void Clear();

            void Done();
            void Fail(const CString &Error);

            bool Submitted() const { return m_Submitted; }
            void Submitted(bool Value) { m_Submitted = Value; }

            CString &MsgId() { return m_MsgId; };
            const CString &MsgId() const { return m_MsgId; };

            CString &MessageId() { return m_MessageId; };
            const CString &MessageId() const { return m_MessageId; };

            CString &From() { return m_From; };
            const CString &From() const { return m_From; };

            CStringList &To() { return m_To; };
            const CStringList &To() const { return m_To; };

            CString &Subject() { return m_Subject; };
            const CString &Subject() const { return m_Subject; };

            CStringList &Body() { return m_Body; };
            const CStringList &Body() const { return m_Body; };

            CString &Content() { return m_Content; };
            const CString &Content() const { return m_Content; };

            const COnMessageEvent &OnDone() const { return m_OnDone; }
            void OnDone(COnMessageEvent && Value) { m_OnDone = Value; }

            const COnMessageErrorEvent &OnFail() const { return m_OnFail; }
            void OnFail(COnMessageErrorEvent && Value) { m_OnFail = Value; }

            CMessage& operator= (const CMessage &Value) {
                if (this != &Value) {
                    Assign(Value);
                }
                return *this;
            };

        };

        //--------------------------------------------------------------------------------------------------------------

        typedef TList<CMessage> CMessages;

    }
}

using namespace Delphi::Message;
}

#endif //DELPHI_MESSAGE_HPP
