#ifndef __RESPONSER__
#define __RESPONSER__

#include "HttpData.h"
#include "Pages.h"
#include "utils.h"

const int PAGE_BUFFER_SIZE = 2048;

class Responser {
public:
    Responser( const HttpData &_httpData );
    ~Responser() {};
    void sendStaticFileToClient(); // 发送url对应的静态网页给客户机
    void sendForBidden(); // 发送403网页给客户机
    void sendNotFound(); // 发送404网页给客户机
    void sendMemoryPage(); // 发送内存中的网页给客户机

private:
    void fisrtLine_200OK( char [] ); // 200 OK 的响应头
    void header_keepAlive( char buf[] ); // 返回接受keep-alive请求
    void header_pngContentType( char buf[] ); // 设置返回文件类型为png
    void header_htmlContentType( char buf[] ); // 设置返回文件类型为静态HTML
    void header_contentLength( char [] , size_t); // 设置返回文件的长度
    void header_body( char [] ); // header和body的分界线
    void serversStaticFile(); // 服务器上的静态文件
    int cgiContentLength(); // 计算server部分输入CGI的字符长度，方便实现长链接

    const HttpData &httpData;
    int clientSocket;
    bool isClose;
};

#endif