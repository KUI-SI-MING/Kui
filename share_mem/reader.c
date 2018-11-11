#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main ()
{
  key_t k = ftok(".", 0x7777);
  if(k < 0){
    printf("ftok error\n");
    return 1;
  }

  //创建共享内存
  int shmid = shmget(k, 4096, IPC_CREAT);
  if(shmid < 0){
    printf("shmget error\n");
  }
  sleep(3);
 
  //挂接
  char *buf = shmat(shmid, NULL, 0);
  sleep(3);

  while(1)
  {
    printf("%s\n", buf);
    sleep(1);
  }
  
  //去除关联
  shmdt(buf);
  sleep(3);

  return 0;
}
