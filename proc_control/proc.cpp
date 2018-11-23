#include <iostream>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

//进程创建
//fork()
//int main()
//{
//  pid_t pid;
//  //获取父进程的pid
//  cout << "Before: pid is " << getpid() << endl;
//
//  pid = fork();
//  if(pid == -1)
//  {
//    perror("fork()");
//    exit(1);
//  }
//  cout << "After: pid is" << getpid() << "fork return :" << pid << endl;
//  sleep(1);
//
//  return 0;
//}

////vfork()
//int glob = 10;
//int main()
//{
//  pid_t pid;
//
//  if((pid = vfork()) == -1){
//    perror("vfork()");
//    exit(1);
//  }
//  if(pid == 0){//child
//    sleep(5);
//    glob = 9;
//
//    cout << "child glob = " << glob << endl;
//    exit(0);
//  }
//  else{//parent
//    cout << "parnent glob = " << glob << endl;
//  }
//
//  return 0;
//}

////进程终止
//int main()
//{
// //exit();
// cout << "I am exit()";
// exit(0);
//
// //_exit()
// cout << "I am _exit()";
// _exit(0);
//}

////进程等待
//int main()
//{
//  //wait
//  //获取进程状态
//  pid_t pid;
//
//  if((pid = fork()) == -1)
//  {
//    perror("fork()");
//    exit(1);
//  }
//
//  if(0 == pid)
//  {//child
//    cout << "child pid is: " << getpid() << endl;
//    sleep(20);//沉睡10秒后退出
//    exit(10);
//  }
//  else
//  {//parents
//    int st;//用于获取子进程的退出状态
//    int ret = wait(&st);//&st输出型参数
//
//    if(ret > 0 && (st & 0x7f) == 0)
//    {//子进程正常退出
//        cout << "child exit num:" << ((st >> 8) & 0xFF) << endl; //st是32位的位图，获取其高8位
//    }
//    else if(ret > 0)
//    {//异常退出
//      cout << "signal num:" << (st & 0x7f) << endl;
//    }
//  }
//  return 0;
//}

////进程阻塞式等待的测试
//int main()
//{
//  pid_t pid = fork();
//  if(pid < 0)
//  {
//    cout << __FUNCTION__ << "fork error" << endl;
//    return 1;
//  }
//  else if(pid == 0)
//  {
//    //child
//    cout << "child is runing, the pid is:" << getpid() << endl;
//    sleep(5);
//    exit(25);
//  }
//  else
//  {
//    int status = 0;
//    pid_t ret = waitpid(-1, &status, 0);//阻塞式等待
//    cout << "this is test for wait" << endl;
//    if(WIFEXITED(status) && ret == pid)
//    {
//      cout << "wait child 5s success, child retrun num is:" << WIFEXITED(status) << endl;
//    }
//    else
//    {
//      cout << "wait child filed,signal num is:" << (status & 0x7F) << endl;
//      return 1;
//    }
//  }
//  return 0;
//}

//进程非阻塞式等待的测试
int main()
{
  pid_t pid = fork();
  if(pid < 0)
  {
    cout << __FUNCTION__ << "fork error" << endl;
    return 1;
  }
  else if(pid == 0)
  {
    //child
    cout << "child is runing, the pid is:" << getpid() << endl;
    sleep(5);
    exit(25);
  }
  else
  {
    int status = 0;
    pid_t ret = 0; 
    do
    {
      ret = waitpid(-1, &status, WNOHANG);//轮询
      if(ret == 0)
      {
        cout << "child is runing" << endl;
      }
      sleep(1);
    }while(0 == ret);

    if(WIFEXITED(status) && ret == pid)
    {
      cout << "wait child 5s success, child retrun num is:" << WIFEXITED(status) << endl;
    }
    else
    {
      cout << "wait child filed,signal num is:" << (status & 0x7F) << endl;
      return 1;
    }
  }
  return 0;
}





