#include "utils.hpp"

enum BoundryType
{  
    BOUNDRY_NO = 0,
    BOUNDRY_FIRST,
    BOUNDRY_MIDDLE,
    BOUNDRY_LAST,
    BOUNDRY_VIRTUAL
};

class Upload
{
    private:
        int _file_fd;
        int64_t content_length;
        std::string _file_name;
        std::string _f_boundry;
        std::string _m_boundry;
        std::string _l_boundry;
    private:
        //任何有可能是boundary的地方都需要停下来存储数据 
        int MatchBoundry(char* buf, int blen, int* boundry_pos)
        {
            //first-boundary：------boundry\r\n
            //middle-boundary：\r\n------boundary\r\n
            //last-boundary：\r\n------boundary--
            //从起始位置匹配first-boundary
            if(memcmp(buf, _f_boundry.c_str(), _f_boundry.length() < 0))//不能用字符串操作，因为文件中有0等字符
            {
                *boundry_pos = 0;
                return BOUNDRY_FIRST;
            }

            for(int i = 0; i < blen; i++)
            {
                //字符串剩余长度大于boundary长度,全部匹配           
                if((blen - i) > _m_boundry.length())
                {
                    if(!memcmp(buf + i, _m_boundry.c_str(), _m_boundry.length()))
                    {
                        *boundry_pos = i;
                        return BOUNDRY_MIDDLE;
                    }
                    else if(!memcmp(buf + i, _m_boundry.c_str(), _l_boundry.length()))
                    {
                        *boundry_pos = i;
                        return BOUNDRY_LAST;
                    }  
                }
                else{
                    //剩余长度小于boundary长度，防止出现半个boundary。进行部分匹配
                    int cmp_len = (blen - i) > _m_boundry.length() ? _m_boundry.length():blen - i;
                    if(!memcmp(buf + i,_l_boundry.c_str(), cmp_len))
                    {
                        *boundry_pos = i;
                        return BOUNDRY_VIRTUAL;
                    }

                    if(!memcmp(buf + i, _m_boundry.c_str(), cmp_len))
                    {
                        *boundry_pos = i;
                        return BOUNDRY_VIRTUAL;
                    }
                }
            }
            return BOUNDRY_NO;
        }

        bool GetFileName(char* buf, int* content_pos)
        {
            char* ptr = NULL;
            ptr = strstr(buf, "\r\n\r\n");
            if(ptr == NULL)
            {
                *content_pos = 0;
                return false;
            }

            std::string head;
            head.assign(buf, ptr - buf);
            *content_pos = ptr - buf + 4;

            std::string file_sep = "filename=\"";
            size_t pos = head.find(file_sep);
            if(pos == std::string::npos)
            {
                return false;
            }

            std::string file = head.substr(pos + file_sep.length());
            pos = file.find("\"");
            if(pos == std::string::npos)
            {
                return false;
            }
            file.erase(pos);
            _file_name = WWWROOT;
            _file_name += "/" + _file_name;

            return true;
        }

        bool CreatFile()
        {
            _file_fd = open(_file_name.c_str(), O_CREAT | O_WRONLY, 0664);
            if(_file_fd < 0)
            {
                fprintf(stderr, "open error:%s\n", strerror(errno));
                return false;
            }
            return true;
        }

        bool CloseFile()
        {

            if(_file_fd != -1)
            {
                close(_file_fd);
                _file_fd = -1;
            }
            return true;
        }

        bool WriteFile(char* buf, size_t len)
        {
            if(_file_fd != -1)
            {
                write(_file_fd,buf, len);
            }
            return true;
        }

    public:
        Upload():_file_fd(-1){}

        //初始化boundary信息
        bool InitUploadInfo()
        {
            umask(0);
            char* ptrl = getenv("Content-Length");
            if(ptrl == NULL)
            {
                fprintf(stderr, "have no content-length\n");
                return false;
            }
            content_length = Utils::StrToDigit("*ptrl");

            char* ptr = getenv("Content-Type");
            if(ptr == NULL)
            {
                fprintf(stderr, "have no content-type\n");
                return false;
            }

            std::string boundry_sep = "boundry=";
            std::string content_type = ptr;
            size_t pos = content_type.find(boundry_sep);
            if(pos == std::string::npos)
            {
                fprintf(stderr, "have no boundry=\n");
                return false;

            }

            std::string boundry;
            boundry = content_type.substr(pos + boundry_sep.length());
            _f_boundry = "--" + boundry;
            _m_boundry = "\r\n" + _f_boundry + "\r\n";
            _l_boundry = "\r\n" + _f_boundry + "--";

            return true;
        }    

        //匹配boundary信息
        bool ProcessUpload()
        {
            int64_t tlen = 0, blen = 0;
            char buf[MAX_BUF] = {0};

            while(tlen < content_length)
            {
                int len = read(0, buf + blen, MAX_BUF - blen);
                blen += len;//当前buf中数据的长度
                int boundry_pos,content_pos;

                int flag = MatchBoundry(buf, blen, &boundry_pos);
                if(flag == BOUNDRY_FIRST)
                {
                    //1.从boundary头文件中获取文件名
                    //2.若获取文件名成功，则创建文件，打开文件
                    //3.将头信息从buf中移除，剩下的数据进行新的匹配
                    if(GetFileName(buf, &content_pos))
                    {
                        CreatFile();
                        blen -= content_pos;
                        memmove(buf, buf + content_pos, blen);
                    }
                    else{
                        if(content_pos == 0)
                            continue;
                        blen -= _f_boundry.length();
                        memmove(buf, buf + _f_boundry.length(), blen);
                    }


                }

                while(1)
                {
                    int flag = MatchBoundry(buf, blen, &boundry_pos);
                    if(flag != BOUNDRY_MIDDLE)
                    {

                        break;

                    }

                    //匹配middle-boundary成功
                    //1.将boundary之前数据写入文件，将数据从buf中移除
                    //2.关闭文件
                    //3.看boundary头信息中是否有文件名，雷同first-boundary
                    WriteFile(buf, boundry_pos);
                    CloseFile();
                    blen -= (boundry_pos);
                    memmove(buf, buf + boundry_pos, blen);


                    if(GetFileName(buf, &content_pos))                                                                                              
                    {                                                                                                                               
                        CreatFile();                                                                                                                
                        blen -= content_pos;                                                                                                        
                        memmove(buf, buf + content_pos, blen);                                                                                      
                    }                                                                                                                               
                    else{                                             
                        if(content_pos == 0)
                        {
                            break;
                        }
                        blen -= _f_boundry.length();                                                                                                
                        memmove(buf, buf + _m_boundry.length(), blen);                                                                              
                    } 

                    flag = MatchBoundry(buf, blen, &boundry_pos);
                    if(flag == BOUNDRY_LAST)
                    {
                        //1.last-boundary匹配成功
                        //2.将boundary之前的数据写入文件，关闭文件，退出
                        WriteFile(buf, boundry_pos);
                        CloseFile();
                        return true;

                    }

                    flag = MatchBoundry(buf, blen, &boundry_pos);
                    if(flag == BOUNDRY_VIRTUAL)
                    {
                        //1.将类似boundary位置之前的数据写入文件
                        //2.移除之前的数据
                        //3.剩下的数据不动， 重新继续接受数据，补全后匹配
                        WriteFile(buf, boundry_pos);
                        blen = (boundry_pos);
                        memmove(buf, buf + boundry_pos, blen);
                        continue;
                    }

                    flag = MatchBoundry(buf, blen, &boundry_pos);
                    if(flag == BOUNDRY_NO)
                    {
                        //直接将buf中所有写入文件
                        WriteFile(buf, blen);
                        blen = 0;
                    }
                    tlen += len;
                }
                return false;
            }
        }
};



int main()
{
    Upload upload;
    if(upload.InitUploadInfo() == false)
    {
        return 0;
    }

    std::string rsp_body; 
    if(upload.ProcessUpload() == false)
    {
        rsp_body = "<html><body><h1>FALILIED!</h1></body></html>";

    }
    else 
    {
        rsp_body = "<html><body><h1>SUCCESS!</h1></body></html>";

    }

    return 0;
}
