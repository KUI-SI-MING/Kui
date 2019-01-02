#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

void Usage(std::string proc)
{
  std::cout << "Usage: " << proc << "Server_ip Server_port" << std::endl;
}
int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    Usage(argv[0]);
    exit(1);
  }

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0)
  {
    std::cerr << "socket error " << std::endl;
    exit(2);
  }

  //客户端一般不绑定
  struct sockaddr_in peer;
  socklen_t len = sizeof(peer);
  bzero(&peer, sizeof(peer));
  peer.sin_family = AF_INET;
  peer.sin_port = htons(atoi(argv[2]));
  peer.sin_addr.s_addr = inet_addr(argv[1]);


  if(connect(sock, (struct sockaddr*)&peer, len) < 0)
  {
    std::cout<< "connect error" << std::endl;
    exit(3);
  }

  char buf[1024];
  std::string message;
  for(;;)
  {
    std::cout << "Please Enter## ";
    std::cin >> message;

    write(sock, message.c_str(), message.size());
    ssize_t s = read(sock, buf, sizeof(buf) - 1);
    if(s > 0)
    {
      buf[s] = 0;
      std::cout << "Server echo##" << buf << std::endl;
    }
  }
  close(sock);
  return 0;
}
