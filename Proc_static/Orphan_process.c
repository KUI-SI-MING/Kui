#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  pid_t id = fork();
  if(id < 0)
  {
    perror("fork");
    return 1;
  }
  else if(id == 0){//child
    printf("I am child, pid is [%d]\n", getpid());
    sleep(10);
  }else{
    printf("I am parent, pid is [%d]\n", getpid());
    sleep(3);
    exit(0);
  }
  return 0;
}
