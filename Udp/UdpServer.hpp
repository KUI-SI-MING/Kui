#pragma once

#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


class Sock{
  private:
    int sock;
    std::string ip;//ip
    int port;//端口号
  public:
    Sock(std::string &ip_, int &port_):ip(ip_), port(port_){}
    
    //创建套接字
    void Socket()
    {
      sock = socket(AF_INET,SOCK_DGRAM, 0);
      if(sock < 0)
      {
        std::cerr << "socket error" << std::endl;
        exit(2);
      }
    }

    //绑定
    void Bind()
    {
      //和当前主机的ip地址绑定
      struct sockaddr_in local_;
      local_.sin_family = AF_INET;
      local_.sin_port = htons(port);
      local_.sin_addr.s_addr = inet_addr(ip.c_str());

      if(bind(sock, (struct sockaddr*)&local_,sizeof(local_)) < 0)
      {
        std::cerr << "bing error" << std::endl;
        exit(3);
      }
    }

    //收消息
    void Recv(std::string &str_, struct sockaddr_in &peer, socklen_t &len)
    {
     char buf[1024];
     len = sizeof(sockaddr_in);
     ssize_t s = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&peer, &len);
     if(s > 0)
     {
       buf[s] = 0;
       str_ = buf; 
     }
    }

    //发消息
    void Send(std::string &str_, struct sockaddr_in &peer, socklen_t &len)
    {
      sendto(sock, str_.c_str(), str_.size(), 0, (struct sockaddr*)&peer, len);
    }

    ~Sock()
    {
      close(sock);
    }
};

class UdpServer{
  private:
    Sock sock;
 
  public:
    UdpServer(std::string ip_, int port_):sock(ip_, port_){}

    void InItServer()
    {
      sock.Socket();
      sock.Bind();
    }

    void Start()
    {
      struct sockaddr_in peer;
      socklen_t len; 
      std::string message;
      for(;;)
      {
        sock.Recv(message, peer, len);
        std::cout << "[" << inet_ntoa(peer.sin_addr) << ":" << ntohs(peer.sin_port) << "]# " << message << std::endl;
        message += " server";
        sock.Send(message, peer, len);
        std::cout << "server echo success!" << std::endl;
      }
    }

    ~UdpServer(){}
};

