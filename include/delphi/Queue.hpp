/*++

Library name:

  libdelphi

Module Name:

  Queue.hpp

Notices:

  Delphi classes for C++

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef DELPHI_QUEUE_HPP
#define DELPHI_QUEUE_HPP

extern "C++" {

namespace Delphi {

    namespace Classes {

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

        //-- CQueueHandler ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CQueueCollection;
        class CQueueHandler;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CQueueHandler *Handler)> COnQueueHandlerEvent;
        //--------------------------------------------------------------------------------------------------------------

        class CQueueHandler: public CPollConnection {
        private:

            CQueueCollection *m_pCollection;

            COnQueueHandlerEvent m_Handler;

            bool m_Allow;

            int AddToQueue();
            void RemoveFromQueue();

        protected:

            void SetAllow(bool Value);

        public:

            CQueueHandler(CQueueCollection *ACollection, COnQueueHandlerEvent && Handler, bool Allow = true);

            ~CQueueHandler() override;

            void Close() override;

            bool Handler();

            bool Allow() const { return m_Allow; };
            void Allow(bool Value) { SetAllow(Value); };

            CQueueCollection *QueueCollection() const { return m_pCollection; };

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CQueueCollection ------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        typedef CPollManager CQueueManager;
        //--------------------------------------------------------------------------------------------------------------

        class CQueueCollection {
        protected:

            size_t m_Progress;
            size_t m_MaxQueue;

            CQueue m_Queue;
            CQueueManager m_QueueManager;

        public:

            explicit CQueueCollection(size_t AMaxQueue);

            void DeleteHandler(CQueueHandler *AHandler);

            int AddToQueue(CQueueHandler *AHandler);
            void InsertToQueue(int Index, CQueueHandler *AHandler);
            void RemoveFromQueue(CQueueHandler *AHandler);

            virtual void UnloadQueue() abstract;

            void IncProgress() { m_Progress++; }
            void DecProgress() { m_Progress--; }

            const size_t &MaxQueue() const { return m_MaxQueue; }

            CQueue &Queue() { return m_Queue; }
            const CQueue &Queue() const { return m_Queue; }

            CQueueManager *ptrQueueManager() { return &m_QueueManager; }
            CQueueManager &QueueManager() { return m_QueueManager; }
            const CQueueManager &QueueManager() const { return m_QueueManager; }

        };

    }

}

}
#endif //DELPHI_QUEUE_HPP
