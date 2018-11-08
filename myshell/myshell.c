/*************************************************************************
	> File Name: myshell.c
	> Author: kuisi
	> Mail: kuisi5495@163.com 
	> Created Time: 2018年11月06日 星期二 16时41分17秒
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024
#define NUM 16

int main ()
{

	char *myargv[NUM];//存放分开的命令
	char cmd[MAX];//存放命令
	while(1)
	{
		printf("[kui@localhost myshell]# ");//修改提示符
		//scanf("%s", cmd);//获取命令scanf以空格作分隔符
		fgets(cmd, sizeof(cmd), stdin);
		cmd[strlen(cmd) - 1] = '\0';//避免引入\n
	
		int i = 0;
		myargv[i++] = strtok(cmd, " ");// ls -a -b -c -d
		char *ret = NULL;
		while(ret = strtok(NULL, " "))//strtok函数？
		{
			myargv[i++] = ret;
		}
		myargv[i] = NULL;

		pid_t id = fork();
		if(id == 0)
		{
			execvp(myargv[0], myargv);
			exit(1);
		}//child
		else
		{
			waitpid(id, NULL, 0);
		}//parent
	}




	return 0;
}

