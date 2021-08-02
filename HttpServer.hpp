#pragma once
#include "Sock.hpp"
#include "Log.hpp"
#include "protocol.hpp"
#include "pthreadpool.hpp"

class http_server{
  private:
    int lsock;
    int port;
    ThreadPool* tp;   
    static pthread_mutex_t svr_lock ;
    static http_server* http_svr;

  private:
    http_server(int _port = 8081)
      :lsock(-1),port(_port) {}

    http_server(const http_server& src) = delete ;
    http_server operator =(const http_server & src) = delete ;

  public:
    static http_server* GetInstance(int port = 8081)
    {
      //懒汉模式：需要创建对象时，再创建，优点，启动速度快;由于考虑多线程模式下，与饿汉相比情况较复杂。
      //双检查：第一层为了保证效率，减少锁的竞争，第二层为了保证线程安全。
      if(http_svr == nullptr)
      {
        pthread_mutex_lock(&svr_lock);
        if(http_svr == nullptr)
        {
          http_svr = new http_server(port);         
        }
        pthread_mutex_unlock(&svr_lock);
      }
      return http_svr;
    }

    void Init()
    {
      lsock = sock::Sock();
      sock::Setsockopt(lsock);
      sock::Bind(lsock,port);
      sock::Listen(lsock);

      tp = new ThreadPool(4);
      tp->InitThreadPool();
    }

    int Accept()
    {
      struct sockaddr_in peer;
      socklen_t len;
      int sock = accept(lsock,(struct sockaddr*)&peer,&len);
      return sock;
    }

    void start()
    {
      for(;;)
      {
        int sock = Accept(); 
        if(sock < 0)
        {
          LOG(warning,"Accept error");
          continue;
        }

        //线程池版本
        Task* tk = new Task(sock);          //线程池中的线程按序进行任务处理，不会出现下面的情况。
        tp->push_task(tk);

        //多线程根据请求数量来处理。
        //可能线程创建还未启动，但是来了新的链接，sock 的值变成了新链接的值，这个旧线程将处理新sock的值，导致出错。
        /*
        int *sockcpy=new int (sock);
        pthread_t tid;
        pthread_create(&tid,nullptr,HandleEntry::handle_http,sockcpy);
        pthread_detach(tid);*/

      }
    }
    
    ~http_server()
    {
      if(lsock >= 0)
      {
        close(lsock);
      }
    }

};
 pthread_mutex_t http_server:: svr_lock = PTHREAD_MUTEX_INITIALIZER;
 http_server* http_server:: http_svr = nullptr;

