#include<iostream>
#include<string>
#include<strings.h>
#include<stdlib.h>
#include<unistd.h>

using std::string ; 

void cal(string &query_string)
{
  string part1;
  string part2;

  int x = 0;
  int y = 0;

  size_t pos = query_string.find('&');
  if(pos != string::npos)
  {
    part1 = query_string.substr(0,pos);
    part2 = query_string.substr(pos+1);
  }
  pos = part1.find('=');
  if(pos!=string::npos)
  {
    x = atoi(part1.substr(pos+1).c_str());
  }
  pos = part2.find('=');
  if(pos != string::npos)
  {
    y = atoi(part2.substr(pos+1).c_str());
  }

  std::cout << "<html>" << std::endl;
  std::cout <<"<h1>"<< x << " + " << y << " = " << x + y << "</h1>"<< std::endl;
  std::cout << x << " - " << y << " = " << x - y << std::endl;
  std::cout << x << " * " << y << " = " << x * y << std::endl;
  std::cout << x << " / " << y << " = " << x / y << std::endl;
  std::cout << "</html>" << std::endl;

}

int main()
{
  string method;
  string query_string;
  if(getenv("METHOD"))
  {
    method = getenv("METHOD");
  }
  else{
    return 1;
  }

  if(strcasecmp(method.c_str(),"GET") == 0)
  {
    query_string = getenv("QUERY_STRING");
  }
  else if(strcasecmp(method.c_str(),"POST") == 0)
  {
    //替换进程并不会替换pcb里的内容（file_struct），只会替换代码与数据。
    //POST -> 0 -> 读取
    //GET  -> 1 -> 写入
    int cl = atoi(getenv("CONTENT-LENGTH"));
    char c = 'Z';
    while(cl)
    {
      read(0,&c,1);
      query_string += c;
      cl--;
    }
  }
    cal(query_string);
  //测试通过重定向读取到的query_string
  //std::cout<<"hello cgi  2"<<std::endl;
  //std::cout<<"usr cgi #"<< query_string << std::endl;
  return 0;
}
