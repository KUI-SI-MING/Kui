#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
using namespace std;

//void* thread1(void* arg)
//{
//  cout << "thread is runing" << endl;
//  int *p = new int;
//  *p = 1;
//  return (void*)p;
//}
//
//void* thread2(void* arg)
//{
//  cout << "thread is exiting" <<endl;
//  int* p = new int;
//  *p = 2;
//  pthread_exit((void*)p);
//}
//void* thread3(void* arg)
//{
//  while(1)
//  {
//    cout << "thread3 is runing..." << endl;
//    sleep(1);
//  }
//  return NULL;
//}
//int main (void)
//{
//  pthread_t tid;
//  void* ret;
//
//  //thread1 return
//  pthread_create(&tid, NULL, thread1, NULL);
//  pthread_join(tid, &ret);
//  cout << "thread return, thread id is:" << tid << " " <<  "return code is:" << (*(int*)ret) << endl;
//
//  //thread2 exit
//  pthread_create(&tid, NULL, thread2, NULL);
//  pthread_join(tid, &ret);
//  cout << "thread return, thread id is:" << tid << " " << "return code is:" << (*(int *)ret) << endl; 
//
//  //thread3 cancel by other
//  pthread_create(&tid, NULL, thread3, NULL);
//  sleep(3);
//  pthread_cancel(tid);
//  pthread_join(tid, &ret);
//  if(ret == PTHREAD_CANCELED)
//  {
//    icout << "thread return, thread id is:" << tid << " " << "return code:PTHREAD_CANCELED" <<endl;
//  }
//  else
//  {
//    cout << "threa return, thread id is:" << tid << "return code: NULL"<< endl;
//  }
//}

//void* thread_run(void* arg)
//{
//  pthread_detach(pthread_self());
//  cout << ((char*)arg) << endl;
//  return NULL;
//}
//
//int main(void)
//{
//  pthread_t tid;
//  if(pthread_create(&tid, NULL, thread_run, "thread1 run...") != 0)
//  {
//    cout << "creat thread failed" << endl;
//    return 1;
//  }
//
//  int ret = 0;
//  sleep(1);
//
//  if(pthread_join(tid, NULL) == 0)
//  {
//    cout << "pthread wait success" <<endl;
//    ret = 0;
//  }
//  else
//  {
//    cout << "pthread wait filaed" << endl;
//    ret = 1;
//  }
//   return ret;
//}


////售票系统
//int ticket = 100;
//
//pthread_mutex_t mutex;
//
//void *route(void *arg)
//{
//  char *id = (char*)arg;
//  
//  while  (1) 
//  {
//    pthread_mutex_lock(&mutex);
//    if ( ticket > 0  ) 
//    {
//      usleep(1000);
//      printf("%s sells ticket:%d\n", id, ticket);
//      ticket--;
//      pthread_mutex_unlock(&mutex);
//      // sched_yield(); 放弃CPU
//    } else {
//      pthread_mutex_unlock(&mutex);
//      break;
//
//    }
//
//  }
//
//}
//int main( void  )
//{
//  pthread_t t1, t2, t3, t4;
//  
//  pthread_mutex_init(&mutex, NULL);
//  pthread_create(&t1, NULL, route, "thread 1");
//  pthread_create(&t2, NULL, route, "thread 2");
//  pthread_create(&t3, NULL, route, "thread 3");
//  pthread_create(&t4, NULL, route, "thread 4");
//  
//  pthread_join(t1, NULL);
//  pthread_join(t2, NULL);
//  pthread_join(t3, NULL);
//  pthread_join(t4, NULL);
//  
//  pthread_mutex_destroy(&mutex);
//}

//条件变量
pthread_cond_t cond;
pthread_mutex_t mutex;

void *r1( void *arg  )
{
  while ( 1  ){
    pthread_cond_wait(&cond, &mutex);//等待
    printf("活动\n");
  }
}

void *r2(void *arg )
{
  while ( 1  ) {
    pthread_cond_signal(&cond);//唤醒
    sleep(1);
  }
}

int main( void  )
{
  pthread_t t1, t2;
  
  pthread_cond_init(&cond, NULL);
  pthread_mutex_init(&mutex, NULL);
  
  pthread_create(&t1, NULL, r1, (void*)"thread 1");//在该条件变量下等待的线程
  pthread_create(&t2, NULL, r2, (void*)"thread 2");//每隔1s通知条件变量下的线程
  
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

}




























