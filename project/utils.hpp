#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdint>
#include <iostream>
#include <string>
#include <signal.h>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sstream>
#include <fcntl.h>
#include <dirent.h>

std::unordered_map<std::string, std::string> g_err_desc = {
    {"200", "OK"},
    {"400", "Bad Request"},
    {"403", "Forbidden"},
    {"404","Not Found"},
    {"405","Method Not Allowed"},
    {"413","Request Entity Too Large"},
    {"500", "Internal Server Error"}};

std::unordered_map<std::string, std::string> g_mime_type = {
    {"txt",     "application/octet-stream"},   
    {"html",    "text/html" },
    {"htm",     "text/html"},
    {"jpg",     "image/jpeg"},
    {"zip",     "application/zip"},
    {"mp3",     "audio/mpeg"},
    {"mpeg",    "video/mpeg"},
    {"unknow",  "application/octet-stream"},
}; 

#define MAX_HTTPHDR 4096
#define WWWROOT "www"
#define MAX_PATH 256
#define MAX_BUF 4096


#define LOG(...) do{\
    fprintf(stdout, __VA_ARGS__, fflush(stdout)); \
}while (0)

class Utils
{
    //提供公用的功能接口  
    public:

        static size_t Splist(std::string& src, const std::string& des, std::vector<std::string>& list)
        {
            int num = 0;
            size_t idx = 0;
            size_t pos;

            while(idx < src.length())
            {
                pos = src.find(des);//寻找des所在位置
                if(pos == std::string::npos)//直到末尾
                {
                    break;
                }

                list.push_back(src.substr(idx, pos - idx));
                num++;
                idx  = pos + des.length();
            }

            if(idx < src.length())
            {
                list.push_back(src.substr(idx));
                num++;    
            }
            return num;
        }

        static std::string GetErrDesc(const std::string& code)
        {
            auto it = g_err_desc.find(code);
            if(it == g_err_desc.end())
            {
                return "UnKnow Error!";
            }
            return it->second;
        }

        static void TimeToGMT(time_t t, std::string& gmt)
        {
            struct tm* mt = gmtime(&t);//系统时间存入结构体mt
            char tmp[128] = {0};
            size_t len = strftime(tmp, 127, "%a, %d, %b. %Y, %H:%M:%s GMT", mt);//完程时间格式的组织//为什么是127最后存放字符串的结尾标志
            gmt.assign(tmp, len);//将新的时间赋给gmt
        }

        static void DegitToStr(int64_t num, std::string& str)
        {
            std::stringstream ss;
            ss << num;
            str = ss.str();

        }                               

        static int64_t StrToDigit(const std::string& str)//字符串到数字的转换
        {
            int64_t num;
            std::stringstream ss;
            
            ss << str;
            ss >> num;
            return num;
        }

        static void MakeETag(int64_t ino, int64_t size, int64_t mtime, std::string& etag)
        {
            std::stringstream ss;
            // "ino-size-mtime"
            ss << "\"";
            ss << std::hex << ino;
            ss << "-";
            ss <<std::hex << size;
            ss << "-";
            ss << std::hex << mtime;
            ss << "\"";

            etag = ss.str();
        }

        static void GetMime(const std::string& file, std::string& mime) 
        {
            size_t pos;
            pos = file.find_last_of(".");
            if(pos == std::string::npos)
            {
                mime = g_mime_type["unknow"];
                return;             
            }

            std::string suffix = file.substr(pos + 1);//后缀
            auto it = g_mime_type.find(suffix);
            if(it == g_mime_type.end())
            {
                mime = g_mime_type["unknow"];
                return;
            }
            else{
                mime = it->second;
            }

        }

};    

class RequestInfo
{
    //包含HttpReauest解析出的请求信息  
    public:
        std::string _method;//请求方法
        std::string _version;//协议版本
        std::string _path_info;//资源路径
        std::string _path_phys;//资源实际路径
        std::string _query_string;//查询字符串
        std::unordered_map<std::string, std::string> _hdr_list;//整个头部的键值对
        struct stat _st;//获取文件(目录，文件类型，权限)信息

    public:
        std::string _err_code;

    public://判断请求类型
        bool  RequestIsCGI()
        {
            if((_method == "GET" && !_query_string.empty() )||(_method == "POST"))
            {
                return true;
            }
            return false;
        }
        void SetError(const std::string& code)
        {
            _err_code = code;
        }
};


class HttpRequest
{
    //http数据的接收接口
    //http数据的解析接口
    //对外提供能够获取处理结果的接口
    public:

        HttpRequest(int sock) :_cli_sock(sock){}

        bool RecvHttpHeaer(RequestInfo& info)//接收http请求头
        {
            LOG("into RecvHttpHeaer\n");
            char tmp[MAX_HTTPHDR] = {0};//请求头不宜过大                     

            while(1)
            {
                int ret = recv(_cli_sock, tmp, MAX_HTTPHDR, MSG_PEEK);//MSG_PEEK:从接收队列的起始地址返回数据， 不移除数据
                if(ret <= 0)//ret==0，对端断开连接; send：触发cengoalchild信号断开连接
                {
                    if(errno == EINTR || errno == EAGAIN)//中间有等待时间,被打断||当前缓冲区没有数据
                    {
                        continue;
                    }
                    info.SetError("500");
                    return false;
                }

                char* ptr = strstr(tmp, "\r\n\r\n");//为什么要看头部中是否包含\r\r\n\n,ptr即\r\n\r\n首次出现在tmp中的地址
                if((ptr == NULL) && (ret == MAX_HTTPHDR))
                {
                    info.SetError("413");
                    return false;
                }
                else if((ptr == NULL) && (ret < MAX_HTTPHDR))
                {
                    usleep(1000); //?
                    continue;
                }

                int hdr_len = ptr - tmp;//头部有效信息的长度
                _http_header.assign(tmp, hdr_len);//截取有效字符串
                recv(_cli_sock, tmp, hdr_len, 0);
                LOG("heder:[%s]\n", _http_header.c_str());
                break;
            }
            return true;
        }

        bool PathIsLegal(RequestInfo& info)
        {
            //疑问：关于www目录的逻辑思考
            //判断路径是否存在
            std::string file = WWWROOT + info._path_info;

            if(stat(info._path_info.c_str(), &info._st) < 0)//stat是什么意思
            {
                info.SetError("404");
                return false;
            }

            char tmp[MAX_PATH] = {0};
            realpath(file.c_str(), tmp);//realpath()函数
            info._path_phys = tmp;

            if(info._path_info.find(WWWROOT) == std::string::npos)
            {
                info.SetError("403");
                return false;
            }
            return true;
        }

        bool ParseFirstLine(std::vector<std::string>& line, RequestInfo& info)
        {
            //解析首行
            std::vector<std::string> line_list;

            if(Utils::Splist(line[0], " ", line_list) != 3)//首行： 方法 URL 版本
            {
                info.SetError("400");
                return false;
            }

            std::string url;
            info._method = line_list[0];
            url = line_list[1];
            info._version = line_list[2];

            if((info._method != "GET" || info._method != "POST") && (info._version != "HTTP/1.1" || info._version != "HTTP/1.0"))
            {
                //请求方法不被允许
                info.SetError("405");
                return false;
            }
            if(url.empty())//如何才算是URL不合法？
            {
                info.SetError("400");//服务器不理解请求的语法
                return false;
            }
            //url: /upload?key=val&key=val
            //可以封装
            size_t pos;
            pos = url.find("?");

            if(pos == std::string::npos)
            {
                info._path_info = url;
            }
            else{
                info._path_info = url.substr(0, pos);
                info._query_string = url.substr(pos + 1);
            }

            return PathIsLegal(info);
        }

        bool ParseHttpHeader(RequestInfo& info)//解析http请求头
        {
            //http请求头解析
            //请求方法 URL 协议版本\r\n
            //key: val\r\nkey: val 
            //先解析首行， 剩余的key val放入哈希表中
            std::vector<std::string> hdr_list;
            Utils::Splist(_http_header, "\r\n", hdr_list);

            if(ParseFirstLine(hdr_list, info) == false)
            {
                return false;
            }

            hdr_list.erase(hdr_list.begin());//去掉首行
            for(size_t i = 0; i < hdr_list.size(); ++i)
            {
                size_t pos = hdr_list[i].find(": ");
                info._hdr_list[hdr_list[i].substr(0, pos)] = hdr_list[i].substr(pos +2);
            }

            return true;
        }
    private:
        int _cli_sock;
        std::string _http_header;
};

class HttpResponse
{
    //文件请求（文件下载、列表功能）接口
    //CGI请求接口
    public:
        HttpResponse(int sock):_cli_sock(sock){}

        bool InitResponse(RequestInfo& req_info)//初始化一些详细信息
        {
            //Last-Modified:
            Utils::TimeToGMT(req_info._st.st_mtime, _mtime);
            //ETag:
            Utils::MakeETag(req_info._st.st_ino, req_info._st.st_size, req_info._st.st_mtime, _etag);
            //Date:
            time_t t = time(NULL);
            Utils::TimeToGMT(t, _date);
            //文件大小
            Utils::DegitToStr(req_info._st.st_size, _fsize);

            Utils::GetMime(req_info._path_phys, _mime);
            return true;
        }

        bool SendData(const std::string& buf)
        {
            if(send(_cli_sock, buf.c_str(), buf.length(), 0) < 0)
            {
                return false;
            }
            return true;
        }


        bool SendCData(const std::string& buf)//分块传输
        {
            if(buf.empty())
            {
                return SendData("0\r\n\r\n");
            }
            
            std::stringstream ss;
            ss << std::hex << buf.length() << "\r\n";

            SendData(ss.str());
            SendData(buf);
            SendData("\r\n");

            return true;
        }

        //文件下载
        //当前请求类型不是CGI请求
        //当前文件类型（目录or非目录）
        bool ProcessFile(RequestInfo& info)//文件下载
        {
            std::string rsp_header;
            rsp_header = info._version + "200 OK\r\n";
            rsp_header += "Content-Type: " + _mime + "\r\n";
            rsp_header += "Connecction: close\r\n";
            rsp_header += "Content-Length: " + _fsize + "\r\n";

            rsp_header += "Etag: " + _etag + "\r\n";
            rsp_header += "Last-Modified: " + _mtime + "\r\n";
            rsp_header += "Date: " + _date + "\r\n\r\n";
            SendData(rsp_header);

            int fd = open(info._path_phys.c_str(),O_RDONLY);
            if(fd < 0)
            {
                info._err_code = "400";
                ErrHandler(info);
                return false;
            }

            int rlen = 0;
            char tmp[MAX_BUF] = {0};
            while((rlen = read(fd, tmp, MAX_BUF)) > 0)
            {
                //tmp[rlen] = '\0';
                //SendData(tmp);//dd if=/dev/zero of=./hello.dat bs=100M count=1当文件全为空，全为0，当发送数据时相当于什么都没发送出去所以不能用string发送
                send(_cli_sock, tmp, rlen, 0);
            }

            close(fd);
            return true;
        }

        //文件列表
        bool ProcessList(RequestInfo& info)
        {
            //组织头部
            //首行
            //Content-Type：text/html\r\n
            //Etag: \r\n
            //Date: \r\n
            //Transfer-Encoding：chunked\r\n\\1.1版本
            //Connection: closer\r\n\r\n
            //空行
            //正文:每一个目录下的文件都要组织一个HTML标签信息
            
            std::string rsp_header;
            rsp_header = info._version + "200 OK\r\n";
            rsp_header += "Content-Type: text/html\r\n";
            rsp_header += "Connection： close\r\n";
            if(info._version == "HTTP/1.1")
            {
                rsp_header += "Transfer-Encoding: chunked\r\n";
            }

            rsp_header += "Etag: " + _etag + "\r\n";
            rsp_header += "Last-Modified: " + _mtime + "\r\n";
            rsp_header += "Date: " + _date + "\r\n\r\n";
            //头部组织完毕
            SendData(rsp_header);

            std::string rsp_body;
            rsp_body = "<html><head>";
            rsp_body += "<title>Index of " + info._path_info + "</title>";
            rsp_body += "<meta charset = 'UTF-8'>";
            rsp_body += "</head><body>";

            rsp_body += "<h1>Index of " + info._path_info + "</h1>";
            rsp_body += "<form action='/upload' method='POST' ";
            rsp_body += "enctype='mutlipart/form-data'>";
            rsp_body += "<input type='text' name='FileUpload' value='abc'/>";
            rsp_body += "<input type='file' name='FileUpload' />";
            rsp_body += "<input type='submit' value='上传' />";
            rsp_body += "</form>";
            rsp_body += "<hr /><ol>";
            
		if(info._version == "HTTP/1.1")
            {
              SendCData(rsp_body);
            }
            else{
                SendData(rsp_body);
            }

            std::string path = info._path_phys;
            struct dirent** p_dirent = NULL;//存储目录信息
            //获取目录下的每一个文件，组织为html信息，chunke传输
            size_t num = scandir(info._path_phys.c_str(), &p_dirent, 0, alphasort);
            for(size_t i = 0; i < num; i++)
            {
                std::string file_html;
                std::string file = info._path_phys + p_dirent[i]->d_name;//当前文件的全路径
                struct stat st;
                if(stat(file.c_str(), &st) < 0)//获取失败
                {
                    continue;
                }

                //组织信息
                std::string mtime; 
                Utils::TimeToGMT(st.st_mtime, mtime);
                std::string mime;
                Utils::GetMime(p_dirent[i]->d_name, mime);
                std::string fsize;
                Utils::DegitToStr(st.st_size/ 1024, fsize);
                
                file_html += "<li><strong><a href='" + info._path_info;//相对根目录起始
                file_html += p_dirent[i]->d_name; 
                file_html += "'>";
                file_html += p_dirent[i]->d_name;//展示信息 
                file_html  += "</a></strong>";
                file_html += "<br /><small>";
                file_html += "modified: " + mtime + "<br />";
                file_html += mime + "-" + fsize + "kbytes" + "<br />";//文件类型 + 文件大小
                file_html = "<br /></small></li>";
                 SendCData(file_html);

            }

            rsp_body = "</ol><hr /></body></html>";
            SendCData(rsp_body);
            SendCData("");
            return true;
        }
        //CGI请求处理
        bool ProcessCGI(RequestInfo& info)
        {
            //创建管道
            //使用外部程序完成CGI请求处理---文件上传
            //将http头信息和正文全部交给子进程处理
            //使用环境变量传递头信息
            //使用管道传递正文数据
            //使用管道结束CGI程序的处理结果
            //流程：创建管道 创建子进程 设置子进程环境变量 程序替换
            

            int in[2];//用于向子进程传递正文数据
            int out[2];//用于从子进程获取数据
            if(pipe(in) || pipe(out))
            {
                info._err_code = "500";
                ErrHandler(info);
                return false;
            }

            int pid = fork();
            if(pid < 0)
            {
                info._err_code = "500";
                ErrHandler(info);
                return false;
            }
            else if (pid == 0)
            {
                //设置环境变量
                setenv("METHOD", info._method.c_str(), 1);
                setenv("VERSION", info._version.c_str(), 1);
                setenv("PATH_INFO", info._path_info.c_str(), 1);
                setenv("QUERY_STRING", info._query_string.c_str(), 1);

                for(auto it: info._hdr_list)
                {
                    setenv(it.first.c_str(), it.second.c_str(), 1);
                }

                close(in[1]);
                close(out[0]);
                execl(info._path_phys.c_str(), info._path_phys.c_str(), NULL);
                exit(0);
            }
            close(in[0]);
            close(out[1]);
            //父进程
            //1.通过in管道将正文数据传递给子进程
            auto it = info._hdr_list.find("Content-Length");

            //it为空则不需要提交正文数据给子进程
            if(it != info._hdr_list.end())
            {
                char buf[MAX_BUF] = {0};
                int64_t content_length = Utils::StrToDigit(it->second);
                LOG("content length is:[%d]\n", content_length);

                int64_t tlen = 0;
                while(tlen < content_length)
                {
                    int len = MAX_BUF > (content_length - tlen) ? (content_length - tlen) : MAX_BUF;
                    int rlen = recv(_cli_sock, buf, len, 0);
                    if(rlen < 0)
                    {
                        //响应错误信息给客户端
                        return false;
                    }
                    if(write(in[1], buf, rlen) < 0)
                    {
                        return false;
                    }
                    tlen += rlen;
                }
            }
            //2.通过out管道读取子进程的处理结果直到返回0
            //3.将处理结果组织http数据，返回给客户端
           std::string rsp_header;
            rsp_header = info._version + "200 OK\r\n";
            rsp_header += "Content-Type: text/html\r\n";
            rsp_header += "Connecction: close\r\n";
            rsp_header += "Etag: " + _etag + "\r\n";
            rsp_header += "Last-Modified: " + _mtime + "\r\n";
            rsp_header += "Date: " + _date + "\r\n\r\n";
            SendData(rsp_header);

            std::string rsp_body;
            while(1)
            {
                char buf[MAX_BUF] = {0};
                int rlen = read(out[1], buf, MAX_BUF);
                if(rlen == 0)
                    break;
                send(_cli_sock, buf, rlen, 0);
            }

            rsp_body = "<html><body><h1>SUCCESS!</h1></body></html>";
            SendData(rsp_body);

            close(in[0]);
            close(out[1]);
            dup2(in[0], 0);//子进程从标准输入读取正文数据
            dup2(out[1], 1);//子进程直接打印处理结果传递给父进程
            execl(info._path_phys.c_str(), info._path_phys.c_str(), NULL);
            
            exit(0);
            return 0;
        }

        //处理错误响应
        bool ErrHandler(RequestInfo& info)
        {
            std::string rsp_header;
            //处理错误响应
            //首行 协议版本 状态码 状态描述
            //头部 Content-Length Date
            //空行
            //正文 rsp_body = "<html><body><h1>404;<h1></body></html>"
            rsp_header = info._version + " " + info._err_code + " ";
            rsp_header += Utils::GetErrDesc(info._err_code) + "\r\n";

            time_t t = time(NULL);//获取当前时间
            std::string gmt;
            Utils::TimeToGMT(t, gmt);//转换成字符串时间
            rsp_header += "Date: " + gmt + "\r\n";

            std::string rsp_body;
            rsp_body  = "<html><body><h1>" + info._err_code;
            rsp_body += "<h1></body></html>";

            std::string cont_len;
            Utils::DegitToStr(rsp_body.length(), cont_len);
            rsp_body += "Content-Length: " +  cont_len + "\r\n\r\n";    

            send(_cli_sock, rsp_header.c_str(), rsp_header.length(), 0);
            send(_cli_sock, rsp_body.c_str(), rsp_header.length(), 0);
            
            return true;
        }

        bool FileIsDir(RequestInfo& info)
        {
            if(info._st.st_mode & S_IFDIR)
            {
                if(info._path_info.back() != '/')
                {
                    info._path_info.push_back('/');
                }
                if(info._path_phys.back() != '/')
                {
                    info._path_phys.push_back('/');
                }
                return true;
            }

            return false;
        }
              
        bool FileHandler(RequestInfo& info)
        {
           InitResponse(info);
           if(FileIsDir(info))//判断请求文件是否是目录
           {
               ProcessList(info);//文件列表
           }else{
               ProcessFile(info);//文件下载
           }
        }

    public:
        //ETag: "inode-fsize-mtime"\r\n
        std::string _etag;//请求的是否是源文件

    private:
        int _cli_sock;
        std::string _mtime;//最后一次修改时间
        std::string _date;//系统的响应时间
        std::string _fsize;
        std::string _mime;
};
#endif // __UTILS_H__
