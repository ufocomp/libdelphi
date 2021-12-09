/*++

Library name:

  libdelphi

Module Name:

  Message.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Message.hpp"
//----------------------------------------------------------------------------------------------------------------------

#define MESSAGE_CRLF_LINEFEED "\r\n"

extern "C++" {

namespace Delphi {

    namespace Message {

        //--------------------------------------------------------------------------------------------------------------

        //-- CMessage --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CMessage::CMessage(): CObject() {
            m_Body.LineBreak(MESSAGE_CRLF_LINEFEED);
            m_Body.NameValueSeparator(':');
            m_Body.QuoteChar('\0');
            m_Body.Delimiter('\n');

            m_Submitted = false;

            m_OnDone = nullptr;
            m_OnFail = nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::Assign(const CMessage &Message) {
            m_Session = Message.m_Session;
            m_MessageId = Message.m_MessageId;
            m_From = Message.m_From;
            m_To = Message.m_To;
            m_Subject = Message.m_Subject;
            m_Body = Message.m_Body;
            m_Content = Message.m_Content;

            m_Submitted = Message.m_Submitted;

            m_OnDone = Message.m_OnDone;
            m_OnFail = Message.m_OnFail;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::Clear() {
            m_Session.Clear();
            m_MessageId.Clear();
            m_From.Clear();
            m_To.Clear();
            m_Subject.Clear();
            m_Body.Clear();
            m_Content.Clear();
            m_Submitted = false;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::Done() {
            DoDone();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::Fail(const CString &Error) {
            DoFail(Error);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::DoDone() {
            if (m_OnDone != nullptr) {
                m_OnDone(*this);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CMessage::DoFail(const CString &Error) {
            if (m_OnFail != nullptr) {
                m_OnFail(*this, Error);
            }
        }
    }
}
}
