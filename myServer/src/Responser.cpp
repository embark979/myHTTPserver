#include "Responser.h"

Responser::Responser( const HttpData &_httpData ) : httpData( _httpData ), clientSocket( httpData.getClientSocket() ), isClose( false ) {}

void Responser::sendStaticFileToClient() {
    struct stat staticFileState;
    int ret = stat( httpData.getUrl().c_str(), &staticFileState );
    ssize_t fileSize = staticFileState.st_size;

    if ( ret == -1 ) {
        if ( errno == EACCES ) sendForBidden();
        else sendNotFound(); 
    }
    else {
        char buf [ PAGE_BUFFER_SIZE ];
        fisrtLine_200OK( buf );
        if ( httpData.keepConnection() ) header_keepAlive( buf );
        if ( httpData.getUrlResourceType() == "png") {header_pngContentType( buf );}
        else header_htmlContentType( buf );

        header_contentLength( buf, fileSize );
        header_body( buf );
        ret = send( clientSocket, buf, strlen( buf ), MSG_NOSIGNAL );
        if( ret == -1 ) throw SocketClosed();
        serversStaticFile();
    }
}

void Responser::fisrtLine_200OK( char buf[] ) {
    sprintf(buf, "HTTP/1.1 200 OK\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    
}

void Responser::sendForBidden() {
    char buf[1024];
    sprintf(buf, "HTTP/1.1 403 Forbidden\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw std::runtime_error("send page to client error!\n");
}

void Responser::sendNotFound() {
    char buf[1024];
    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw SocketClosed();
}

void Responser::header_keepAlive( char buf[] ) {
    sprintf( buf, "%s%s", buf, "Connection: keep-alive\r\n" );
    sprintf( buf, "%s%s", buf, "Keep-Alive: \r\n" ); // rfc2068标准说http1.1的keep alive header默认没有参数
}

void Responser::header_pngContentType( char buf[] ) {
    sprintf(buf, "%s%s", buf, "Content-Type: image/png\r\n" );
}

void Responser::header_htmlContentType( char buf[] ) {
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
}

void Responser::header_contentLength( char buf[], size_t fileSize ) {
    sprintf( buf, "%s%s%zu%s", buf, "Content-Length: ", fileSize, "\r\n" );
}

void Responser::header_body( char buf[] ) {
    sprintf(buf, "%s%s", buf, "\r\n" );  // 和page body的分界线
}

void Responser::serversStaticFile() {
    FILE *resource = NULL;
    char readBuf [ PAGE_BUFFER_SIZE ];
    if ( httpData.getUrlResourceType() != "html"){ // 读取二进制文件
        resource = fopen( httpData.getUrl().c_str() , "rb");
        int count = 1;
        while ( true )
        {
            count = fread( readBuf, 1, PAGE_BUFFER_SIZE, resource ); // 第一个size_t指的是接受数据的内存区域的每个元素的大小，这里是char[]所以填1；第二个size_t指的有多少个元素
            if( count == 0 ) break;
            else{
                int ret = send( clientSocket, readBuf, count, MSG_NOSIGNAL );
                if( ret == -1 ) throw SocketClosed();
            }
        } 
    }
    else { // 读取文本文件
        resource = fopen( httpData.getUrl().c_str() , "r");  // 以只读方法打开url指定的文件
        memset( readBuf, 0, PAGE_BUFFER_SIZE );
        fgets(readBuf, PAGE_BUFFER_SIZE, resource); 
        while (!feof(resource))
        {   
            fgets(readBuf, PAGE_BUFFER_SIZE, resource);
            int ret = send( clientSocket, readBuf, strlen( readBuf ), MSG_NOSIGNAL );
            if( ret == -1 ) throw SocketClosed();
        }
    }
    fclose( resource );
}

void Responser::sendMemoryPage() {
    char buf [ PAGE_BUFFER_SIZE ];
    fisrtLine_200OK( buf );
    header_htmlContentType( buf );
    sprintf( buf, "%s%s", buf, memory_index_page );
    send( clientSocket, buf, strlen( buf ), 0 );
}