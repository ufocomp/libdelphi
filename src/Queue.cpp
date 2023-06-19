/*++

Library name:

  libdelphi

Module Name:

  Queue.cpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#include "delphi.hpp"
#include "delphi/Queue.hpp"
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Delphi {

    namespace Classes {

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueueItem ------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CQueueItem::CQueueItem(CCollection *ACollection, Pointer AQueue) : CCollectionItem(ACollection) {
            m_pQueue = AQueue;
            m_pItems = new CList();
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem::~CQueueItem() {
            delete m_pItems;
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CQueueItem::Get(int Index) {
            return m_pItems->Items(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueItem::Put(int Index, Pointer Value) {
            m_pItems->Insert(Index, Value);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueItem::Add(Pointer Item) {
            return m_pItems->Add(Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueItem::Insert(int Index, Pointer Item) {
            m_pItems->Insert(Index, Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueItem::Delete(int Index) {
            m_pItems->Delete(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueItem::Remove(Pointer Item) {
            int Index = IndexOf(Item);
            if (Index >= 0)
                Delete(Index);
            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueItem::IndexOf(Pointer Item) {
            return m_pItems->IndexOf(Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CQueueItem::First() {
            Pointer P = nullptr;

            if (m_pItems->Count() > 0)
                P = m_pItems->Items(0);

            return P;
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CQueueItem::Last() {
            Pointer P = nullptr;

            if (m_pItems->Count() > 0)
                P = m_pItems->Items(m_pItems->Count() - 1);

            return P;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueue ----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CQueue::CQueue(): CCollection(this) {

        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem *CQueue::GetItem(int Index) const {
            return (CQueueItem *) inherited::GetItem(Index);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueue::SetItem(int Index, const CQueueItem *Item) {
            inherited::SetItem(Index, (CQueueItem *) Item);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueue::IndexOf(Pointer Queue) const {
            int Index = 0;

            while ((Index < Count()) && (Items(Index)->Queue() != Queue))
                Index++;

            if (Index == Count())
                Index = -1;

            return Index;
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem *CQueue::Add(Pointer Queue) {
            auto Item = new CQueueItem(this, Queue);
            inherited::Added((CCollectionItem *) Item);
            return Item;
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem *CQueue::Insert(int Index, Pointer Queue) {
            CQueueItem *Item = Add(Queue);
            Item->Index(Index);
            return Item;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueue::Remove(Pointer Queue) {
            const int i = IndexOf(Queue);
            if (i >= 0)
                Delete(i);
            return i;
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem *CQueue::First() const {
            return GetItem(0);
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueItem *CQueue::Last() const {
            return GetItem(Count() - 1);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueue::AddToQueue(Pointer Queue, Pointer P) {
            CQueueItem *Item;
            const int i = IndexOf(Queue);

            if (i == -1)
                Item = Add(Queue);
            else
                Item = GetItem(i);

            return Item->Add(P);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueue::InsertToQueue(Pointer Queue, int Index, Pointer P) {
            CQueueItem *Item;
            const int i = IndexOf(Queue);

            if (i == -1)
                Item = Add(Queue);
            else
                Item = GetItem(i);

            Item->Insert(Index, P);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueue::RemoveFromQueue(Pointer Queue, Pointer P) {
            CQueueItem *Item;
            const int i = IndexOf(Queue);

            if (i >= 0) {
                Item = GetItem(i);
                Item->Remove(P);
                if (Item->Count() == 0)
                    Delete(i);
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CQueue::FirstItem(Pointer Queue) const {
            const int i = IndexOf(Queue);
            if (i != -1)
                return Items(i)->First();
            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        Pointer CQueue::LastItem(Pointer Queue) const {
            const int i = IndexOf(Queue);
            if (i != -1)
                return Items(i)->Last();
            return nullptr;
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueue::CountItems(Pointer Queue) const {
            const int i = IndexOf(Queue);
            if (i != -1)
                return Items(i)->Count();
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueueHandler ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CQueueHandler::CQueueHandler(CQueueCollection *ACollection, COnQueueHandlerEvent && Handler, bool Allow):
                CPollConnection(ACollection->ptrQueueManager()), m_Allow(Allow) {

            m_pCollection = ACollection;
            m_Handler = Handler;

            AddToQueue();
        }
        //--------------------------------------------------------------------------------------------------------------

        CQueueHandler::~CQueueHandler() {
            RemoveFromQueue();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueHandler::Close() {
            m_Allow = false;
            RemoveFromQueue();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueHandler::SetAllow(bool Value) {
            if (m_Allow != Value) {
                m_Allow = Value;
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueHandler::AddToQueue() {
            return m_pCollection->AddToQueue(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueHandler::RemoveFromQueue() {
            m_pCollection->RemoveFromQueue(this);
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CQueueHandler::Handler() {
            if (m_Allow && m_Handler) {
                m_Handler(this);
                return true;
            }
            return false;
        }

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueueCollection ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CQueueCollection::CQueueCollection(size_t AMaxQueue) {
            m_MaxQueue = AMaxQueue;
            m_Progress = 0;
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueCollection::DeleteHandler(CQueueHandler *AHandler) {
            delete AHandler;
            if (m_Progress > 0)
                DecProgress();
            UnloadQueue();
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueCollection::AddToQueue(CQueueHandler *AHandler) {
            return m_Queue.AddToQueue(this, AHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueCollection::InsertToQueue(int Index, CQueueHandler *AHandler) {
            m_Queue.InsertToQueue(this, Index, AHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CQueueCollection::RemoveFromQueue(CQueueHandler *AHandler) {
            m_Queue.RemoveFromQueue(this, AHandler);
        }
        //--------------------------------------------------------------------------------------------------------------

        int CQueueCollection::CountItems() {
            return m_Queue.CountItems(this);
        }
        //--------------------------------------------------------------------------------------------------------------

    }
}
}
