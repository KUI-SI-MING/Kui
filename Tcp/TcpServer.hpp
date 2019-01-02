#pragma once

#include <iostream>
#include <string.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

class Sock
{
  private:
    int listen_sock;
    //std::string ip;//可以省略
    int port;

  public:
    //Sock(const std::string &ip_, const int &port_):ip(ip_), port(port_), listen_sock(-1){}
    Sock(const int &port_):port(port_),listen_sock(-1)
  {}
    void Socket()
    {
      listen_sock = socket(AF_INET, SOCK_STREAM, 0);
      if(listen_sock < 0)
      {
        std::cerr << "listen_socket error" << std::endl;
        exit(2);
      }
    }

    void Bind()
    {
      struct sockaddr_in local;
      bzero(&local, sizeof(local));
      local.sin_family = AF_INET;
      local.sin_port = htons(port);

      //local.sin_addr.s_addr = inet_addr(ip.c_str());
      local.sin_addr.s_addr = htonl(INADDR_ANY);//服务器多个ip
      
      if(bind(listen_sock, (struct sockaddr*)&local, sizeof(local)) < 0)
      {
        std::cerr << "bind,error" << std::endl;
        exit(3);
      }
    }

    //监听
    void Listen()
    {
      if(listen(listen_sock, 5) < 0)
      {
        std::cerr << "listen error" << std::endl;
        exit(4);
      }
    }

    int Accept()
    {
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      int sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
      if(sock < 0)
      {  
        std::cerr << "accept error" << std::endl;
        return -1;
      }
    }
    
    ~Sock()
    {
      close(listen_sock);
    }
    
};

class Server
{
  private:
    Sock sock;
    struct data_t{
      Server* sp;
      int sock;
    };
  public:
   //Server(const std::string &ip_, const int &port_):sock(ip_, port_)
   Server(const int &port_):sock(port_) 
    {
    }

    void InItServer()
    {
      signal(SIGCHLD, SIG_IGN); //多请求 忽略信号
      sock.Socket();
      sock.Bind();
      sock.Listen(); 
    }

    void Serveice(int sock)
    {
      for(;;)
      {
        //流式
        char buf[1024];
        ssize_t s = read(sock, buf, sizeof(buf) - 1);
        if(s > 0)
        {
                              
          buf[s] = 0;
          std::cout << buf << std::endl;
          write(sock, buf, strlen(buf));
        }
        else if(s == 0)
        {
          std::cout << "client is quit" << std::endl;
          break;
        }
        else
        {
          std::cerr << "read erroi" << std::endl;
          break;
        }
      }

      //关闭文件描述符
      close(sock);
    }

    static void* ThreadRun(void* arg) //类成员函数有隐藏指针传入
    {
      pthread_detach(pthread_self());
      data_t *d = (data_t*)arg;
      Server *sp = d->sp;
      int sock = d->sock;
      delete d;
      sp->Serveice(sock);
    }
    void Run()
    {
      for(;;)
      {
        int new_sock = sock.Accept();
        if(new_sock < 0)
        {
          continue;
        }
        std::cout << "Get A New Client..." << std::endl;
        
        //多进程
        // pid_t id = fork();
       // if(id == 0)
       // {
       //   Serveice(new_sock);
       //   exit(0);
       // }
       // close(new_sock);
       
       //多线程
       pthread_t tid;
       data_t* d = new data_t;
       d->sp = this;
       d->sock = new_sock;
       pthread_create(&tid, NULL, ThreadRun, (void*)d);
      }
    }
      ~Server()
      {
      }
};
