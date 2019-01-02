#include "UdpServer.hpp"

//用户手册
void Usage(std::string proc_)
{
 std::cout << "Usage: " << proc_ << "ip port" << std::endl;
}

//server ip port
int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    Usage(argv[0]);
    exit(1);
  }
  UdpServer* sp = new UdpServer(argv[1], atoi(argv[2]));
  sp->InItServer();
  sp->Start();

  delete sp;
  return 0;
}
