#include "HttpServer.hpp"
#include "Log.hpp"

static void Usage(std::string _porc)
{
  std::cout << "Usage: \n\t";
  std::cout << "method one: "<<_porc << "port\n\t";
  std::cout << "method two: "<<_porc << std::endl;

}

int main(int argc, char *argv[])
{
  if(argc != 1 && argc != 2){
    Usage(argv[0]);
    return 1;

  }
  http_server *svr = nullptr;
  if(argc == 1){
    svr = http_server::GetInstance(8081);

  }
  else{
    svr = http_server::GetInstance(atoi(argv[1]));

  }
  daemon(1, 1);          //守护进程化，将该进程变为守护进程，参数参考该函数文档。
  svr->Init();
  svr->start();
  return 0;

}
