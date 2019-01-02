#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>

void Usage(std::string proc_)
{
  std::cout << "Usage: " << proc_ << "server_ip server_port" << std::endl;
}
//client server_ip
int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    Usage(argv[0]);
    exit(1);
  }

  //创建套接字
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0)
  {
    std::cerr << "socket error" << std::endl;
    exit(2);
  }

  //发送数据
  std::string message;
  struct sockaddr_in server;
  bzero(&server,sizeof(server));
  
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[2]));
  server.sin_addr.s_addr = inet_addr(argv[1]);

  char buf[1024];
  for(;;)
  {                                                                     \
    socklen_t len = sizeof(server);
    std::cout << "Please Enter##";
    std::cin >> message;
    
    sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&server, sizeof(server));
    ssize_t s = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&server, &len);
    buf[s] = 0;
    std::cout << "server echo##" << buf << std::endl;
  }
  return 0;
}
