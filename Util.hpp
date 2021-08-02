#pragma once 

#include"Sock.hpp" 

class Util{
  public:

    static void string_parse(string &request_line,string &method,string&uri,string &version)
    {
      std::stringstream line(request_line);
      line >> method >> uri >> version;
    }

    static void get_request_hander_kv(string &line,string &k,string &v)
    {
      size_t pos = line.find(": ");
      if(pos!=string::npos)
      {
        k=line.substr(0,pos);
        v=line.substr(pos+2);
      }
    }
};
