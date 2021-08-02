#pragma once 
#include <iostream>
#include <pthread.h>
#include <queue>
#include "protocol.hpp"

typedef void*(*handler_t)(int);

class Task{
  private:
    int sock;
    handler_t handle;
  public:
    Task(int sk):sock(sk),handle(HandleEntry::handle_http)
  {}

    void run()
    {
      handle(sock);
    }
    ~Task()
    {}
};

class ThreadPool{
  private:
    int num;
    std::queue<Task*> q;
    pthread_mutex_t lock;
    pthread_cond_t cond;
  private:
    void LockQueue()
    {
      pthread_mutex_lock(&lock);
    }

    void UnLockQueue()
    {
      pthread_mutex_unlock(&lock);
    }

    bool IsEmpty()
    {
      return q.empty();
    }

    void ThreadWait()
    {
      pthread_cond_wait(&cond,&lock);
    }

    void ThreadWakeUp()
    {
      pthread_cond_signal(&cond);
    }
  public:
    ThreadPool(int n = 2):num(n)
  {}

    static void * Routinue(void * args)
    {
      ThreadPool *tp = (ThreadPool*)args;
      while(true)
      {
        tp->LockQueue();
        while(tp->IsEmpty()){
          tp->ThreadWait();
        }
        Task *tk = tp->poptask();
        tp->UnLockQueue();

        //取出任务直接解锁，执行的任务放在锁外，提升效率。
        tk->run();

        delete tk;
      }
      return nullptr;
    }

    void InitThreadPool()
    {
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond,nullptr);
      pthread_t tid;
      for(int i=0;i < num;i++)
      {
        pthread_create(&tid,nullptr,Routinue,this);         //类对象默认带一个this指针参数，不符合pthread_create的参数设定，改为静态函数，传this指针
        pthread_detach(tid);
      }
    }
    void push_task(Task * in)
    {
      LockQueue();
      q.push(in);
      UnLockQueue();
      ThreadWakeUp();
    }

    Task* poptask()
    {
      //无需再加锁，调用该函数的地方已经加锁。
      Task* tmp = q.front();
      q.pop();
      return tmp;
    }

    ~ThreadPool()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond);
    }
};
