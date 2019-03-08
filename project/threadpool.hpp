#include<iostream>
#include <queue>
#include "utils.hpp"
#include <pthread.h>

using namespace std;

typedef bool (*Handler)(int sock);
class HttpTask
{
    //http请求处理的任务
    //包含成员socket
    //包含任务处理函数
    private:
        int _cli_sock;
        Handler TaskHandler;

    public:
        HttpTask() :_cli_sock(-1){}
        
        HttpTask(int sock, Handler handler) :_cli_sock(sock), TaskHandler(handler){}

        void SetHttpTask(int sock, Handler handler)
        {
            _cli_sock = sock;
            TaskHandler = handler;
        }
        
        void Handler()
        {
            TaskHandler(_cli_sock);
        }
};

class ThreadPool
{
    //线程池类
    //创建指定数量的线程
    //创建一个线程安全的任务队列
    //提供任务的入队， 出队，线程池销毁/初始化接口
    public:
        ThreadPool(int max) :_max_thr(max), _cur_thr(0), _is_stop(false){}
        
        ~ThreadPool()              
        {
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond);
        }

        bool ThreadPoolInit()
        {
            //线程创建、互斥锁、条件变量
            pthread_t tid;
            for (int i = 0; i < _max_thr; i++)
            {
                int ret = pthread_create(&tid, NULL, thr_start, this);

                if (ret != 0)
                {
                    LOG("thread create error\n");
                    return false;
                }

                pthread_detach(tid);
                _cur_thr++;
            }
            
            if(pthread_mutex_init(&_mutex, NULL) != 0)
            {
                LOG("init mutex error\n");
                return false;

            }
            if(pthread_cond_init(&_cond, NULL) != 0)
            {
                LOG("init cond error\n");
                return false;
            }
            return true;
        }

        //线程安全的入队
        bool PushTask(HttpTask& tt)
        {
            QueueLock();
            
            _task_queue.push(tt);
            QueueUnLock();

            ThreadWakeUpAll();//注意需要唤醒线程
            return true;
        }

        bool PopTask(HttpTask &tt)//线程安全的任务出对,任务的出队是在线程接口中调用，但是线程接口中在出对之前就会进行加锁，所以这里不需要加锁
        {
            tt = _task_queue.front();
            _task_queue.pop();
            return true;
        }

        bool ThreadPoolStop()//销毁线程池
        {
            if(!IsStop())
            {
                _is_stop = true;
            }
            
            while(_cur_thr > 0)
            {
                ThreadWakeUpAll();
                usleep(1000);
            }
            return true;//不太清楚逻辑
        }

    private:
        static void* thr_start(void* arg)//完成线程获取任务,并执行任务
        {
            //循环获取任务
            while(1)
            {
                ThreadPool* tp = (ThreadPool*)arg; 

                tp->QueueLock();
                if(tp->QueueIsEmpty())
                {
                    tp->ThreadWait();
                }

                HttpTask ht;
                tp->PushTask(ht); 
                tp->QueueUnLock();

                ht.Handler();
                return NULL;

            }

        }

    private:
        int _max_thr;//最大线程数
        int _cur_thr;//当前线程数
        bool _is_stop;

        std::queue<HttpTask> _task_queue;
        pthread_mutex_t _mutex;
        pthread_cond_t _cond;

    private:
        void QueueLock()
        {
            pthread_mutex_lock(&_mutex);
        }

        void QueueUnLock()
        {
            pthread_mutex_unlock(&_mutex);
        }

        bool IsStop()
        {
            return _is_stop;
        }

        void ThreadExit()
        {
            _cur_thr--;
            pthread_exit(NULL);
        }

        void ThreadWait()
        {
            if(IsStop())
            {
                //销毁线程池，无需等待,解锁后直接退出
                QueueLock();
                ThreadExit();
            }

            pthread_cond_wait(&_cond, &_mutex);
        }

        void ThreadWakeUp()
        {
            pthread_cond_signal(&_cond);
        }

        void ThreadWakeUpAll()
        {
            pthread_cond_broadcast(&_cond);
        }

        bool QueueIsEmpty()
        {
            return _task_queue.empty();
        }
};

