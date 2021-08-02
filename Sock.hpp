#pragma once
#include<iostream>
#include<stdlib.h>
#include<sstream>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/sendfile.h>

#include<strings.h>
#include<vector>

#include"Log.hpp"

#define BACK_LOG 8

class sock{
  public:
    static int Sock()
    {
      int _sock = socket(AF_INET,SOCK_STREAM,0);
      if(_sock < 0)
      {        
        LOG(fatal,"socket create error"); 
        exit(Socketerr);
      }
      return _sock;
    }

    static void Bind(int sock,int port)
    {
      struct sockaddr_in local;
      bzero(&local,sizeof(local));
      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      local.sin_addr.s_addr = htonl(INADDR_ANY);

      if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
      {
        LOG(fatal,"Bind error");
        exit(Binderr);
      }
    }

    static void Listen(int lsock)
    {
      if(listen(lsock,BACK_LOG) < 0)
      {
        LOG(fatal,"Listenerr");
        exit(Listenerr);
      }
    }

    static void Setsockopt(int sock)
    {
      int opt=1;
      setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    }

    static void GetLine(int sock,std::string &line)
    {
      char c='Z';
      while(c!='\n')
      {
        ssize_t s = recv(sock,&c,1,0);
        if(s>0)
        {
          if(c == '\r')
          {
            ssize_t ss = recv(sock,&c,1,MSG_PEEK);
            // \r\n
            if(ss>0 && c=='\n')
            {
              recv(sock,&c,1,0);
            }
            else
              // \r
            {
              c = '\n';
            }
          }
          if(c != '\n')
            line += c;
        }
      }
    }

};

