#ifndef __HTTPDATA__
#define __HTTPDATA__

#include <stdio.h>
#include <cstring>
#include <memory>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <unordered_map>

#include "utils.h"

using std::shared_ptr; using std::string; using std::vector;
// 存储资源的根目录路径
extern const string NULLINFO;

const char RESOURCE_ROOT[] = "www";
const int HTTPDATA_BUFFERSIZE = 1024; 
const int LINE_BUFFERSIZE = 256;

class HttpData {
public:
    HttpData( int clientfd );
    enum RequestMethod{ UNSUPPORT, GET, POST };
    void parseData();
    const string& getUrl() const { return url; }
    const string& getRequestMethod_string() const { return requestMethod_string; }
    const int getRequestMethod() const { return requestMethod; }
    const string& getVersion() const { return version; }
    const string& getUserAgent() const { return userAgent ? *userAgent : NULLINFO; }
    const string& getUrlResourceType() const { return urlResourceType; }
    int getContentLength() const { return contentLength ? numeralContentLength() : 0; }
    int getParamCount() const { return clientParamData ? (*clientParamData).size() : 0; }
    int getClientSocket() const { return clientSocket; }
    const shared_ptr< std::unordered_map< string, string > > getClientParamData() const { return clientParamData; }
    bool keepConnection() const { return m_keepConnection; }
    bool badRequest() const { return m_badRequest; }

private:
    void parseStartLine(); // 开始解析
    void parseHeader(); // 解析Http头部文件
    string parseOneLine(); // 解析行
    string getHeaderLineName( string &s );
    int numeralContentLength() const; // 将content-length字符串转换为数字
    void readRawDataFromSocket(); // 读取数据
    inline bool dataBufferEmpty() {return readIndex > dataEndIndex || readIndex >= HTTPDATA_BUFFERSIZE;};
    void stringSplit(const string& s, vector<string>& tokens, const string& delimiters = " ");
    void getRequestMethod( const vector<string>& headerInfo );
    void parseUrlDataParamList();
    void parseStringParamTo_htable_clientParamData( string & str ); // 存储键值对
    void parseBodyParamData();

    int clientSocket;
    char dataBuffer[ HTTPDATA_BUFFERSIZE ];
    int readIndex, dataEndIndex; // 指向未被读取的数据的第一个字符

    char prev;

    RequestMethod requestMethod;
    string requestMethod_string, url, urlResourceType, version;
    shared_ptr<string> host, userAgent, accept, acceptLanguage, acceptEncoding, connection, upgradeInsecurceRequests, contentType, contentLength; // 弄成堆上对象，因为有可能为空。用智能指针来管理
    shared_ptr<std::unordered_map<string, string>> clientParamData;  // 利用哈希表保存post/get中的键值对
    bool m_keepConnection, m_badRequest;
};

#endif