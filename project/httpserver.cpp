#include "threadpool.hpp"
#include "utils.hpp"

#define MAX_LISTEN 5
#define MAX_THREAD 5

class HttpServer
{
    private:
        int _serv_sock;
        ThreadPool* _tp;

    private:
        static bool HttpHandler(int sock)//http任务处理函数
        {
            RequestInfo info;
            HttpRequest req(sock);
            HttpResponse rsp(sock);

            //接收http头部
            if(req.RecvHttpHeaer(info) == false)
            {
                goto out;//一般不会使用goto，破坏逻辑
            }


            //解析http头部
            if(req.ParseHttpHeader(info) == false)
            {
                goto out;
            }

            //判断是否CGI
            if(info.RequestInfo::RequestIsCGI())
            {
                //执行CGI
                rsp.ProcessCGI(info);
            }
            else{
                //不是CGI请求，执行目录/文件下载响应
                rsp.ProcessFile(info);
            } 

            info._err_code = "404";
            rsp.ErrHandler(info);
            close(sock);
            return true;
out:
            rsp.ErrHandler(info);
            close(sock);
            return false;
        }
    //建立一个tcp服务端程序，接收新连接
    public:
        HttpServer():_serv_sock(-1), _tp(NULL) {}
        
        bool HttpServerInit(std::string &ip, int& port)//TCP服务端socket的初始化、线程池初始化
        {
            _serv_sock = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
            if(_serv_sock < 0)
            {
                LOG("sock error;%s\n", strerror(errno));
                return false;
            }

            int opt = 1;
            setsockopt(_serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));//地址重用setsockopt(),避免TIME_WAIT的时间消耗

            struct sockaddr_in lst_addr;
            lst_addr.sin_family = AF_INET;
            lst_addr.sin_port = htons(port);
            lst_addr.sin_addr.s_addr = inet_addr(ip.c_str());

            socklen_t len = sizeof(lst_addr);

            if(bind(_serv_sock, (struct sockaddr*)&lst_addr, len) < 0)
            {
                LOG("bind error:%s\n", strerror(errno));
                close(_serv_sock);
                return false;
            }

            if(listen(_serv_sock, MAX_LISTEN) < 0)
            {
                LOG("listen error:%s\n", strerror(errno));
                close(_serv_sock);
                return false;
            }

             //为新连接组织一个线程池任务，添加到线程
            _tp = new ThreadPool(MAX_THREAD);
            if(_tp == NULL)
            {
                LOG("thread pool creat error!\n");
                return false;
            }

            if(_tp->ThreadPoolInit() == false)
            {
                LOG("thread pool init error\n");
                return false;
            }

            return true;
        }

        //获取客户端连接--创建任务、任务入队
        bool start()
        {
            while(1)
            {
                struct sockaddr_in addr;
                socklen_t len;

                int cli_sock = accept(_serv_sock,(struct sockaddr*)&addr, &len);
                if(cli_sock < 0)
                {
                    LOG("accept error:%s\n",strerror(errno));
                    continue;
                }

                //创建任务
                HttpTask ht;
                ht.SetHttpTask(cli_sock, HttpHandler);
                _tp->PushTask(ht);
            }

            return  true;
        }
};

int main(int argc, char* argv[])
{
    std::string ip = argv[1];
    int port = atoi(argv[2]);

    HttpServer hs;

    signal(SIGPIPE, SIG_IGN);//服务端发送完毕关闭链接，若对方关闭链接我仍然发送，send出错，会收到sigpipe信号，默认处理终止进程
    if(hs.HttpServerInit(ip, port) == false)
    {
        return -1;
    }

    if(hs.start() == false)
    {
        return -1;
    }

    return 0;
}
