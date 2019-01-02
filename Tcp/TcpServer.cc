#include "TcpServer.hpp"

void Usage(std::string proc_)
{ 
  //std::cout << "Usage: " << proc_ << "local_ip local_port" << std::endl;
  std::cout << "Usage: " << proc_ << "local_port" << std::endl; 
}
int main(int argc, char* argv[])
{
  //if(argc != 3)
  if(argc != 2)
  {
    Usage(argv[0]);
    exit(1);
  }

  //std::string ip = argv[1];
  //int port = atoi(argv[2]); 

  int port = atoi(argv[1]);
  //Server* sp = new Server(ip, port);
  Server* sp = new Server(port);
  sp->InItServer();
  sp->Run();

  delete sp;
  return 0;
}
