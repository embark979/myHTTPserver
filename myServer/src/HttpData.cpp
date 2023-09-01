#include "HttpData.h"

const string NULLINFO("NULL");

HttpData::HttpData( int clientfd ) : readIndex(HTTPDATA_BUFFERSIZE), dataEndIndex(HTTPDATA_BUFFERSIZE - 1), requestMethod(UNSUPPORT), 
                                     m_keepConnection(false), m_badRequest(false), prev(0), clientSocket(clientfd) {
    memset( dataBuffer, 0, HTTPDATA_BUFFERSIZE );
}

void HttpData::parseData() {
    parseStartLine();
    if( m_badRequest || requestMethod == UNSUPPORT ) return;
    parseHeader();
    if( requestMethod == POST ) parseBodyParamData();
}

void HttpData::parseStartLine() {
    string firstLine( std::move( parseOneLine() ) ); // 先解析请求行信息GET /html.txt HTTP/1.1, 存储在firstLine中
    vector<string> headerInfo;
    stringSplit( firstLine, headerInfo, " " );
    getRequestMethod( headerInfo );
    // 如果请求无法识别就不进行后续的url处理，即一旦被标记为badRequest或者UNSUPPORT，那么httpData中的大部分数据都将不会生成。因此在要继续访问httpData前需要进行一次检查
    if( m_badRequest || requestMethod == UNSUPPORT ) return;

    url = headerInfo[ 1 ];
    if( url == "/" ) url = "/index.html";

    parseUrlDataParamList();

    // 计算url的后缀
    auto suffixStartIndex = url.find_last_of('.');
    if( suffixStartIndex == string::npos ) urlResourceType = NULLINFO;
    else urlResourceType = url.substr( suffixStartIndex + 1 );

    url = RESOURCE_ROOT + url;
    version = headerInfo [ 2 ];
}

void HttpData::parseHeader() {
    string line;
    string headName;
    while( true ){
        line = parseOneLine();
        if( line == "" ) break;
        headName = getHeaderLineName( line );
        if( headName == "HOST" ) host = std::make_shared< string >( std::move( line ) );
        else if( headName == "User-Agent" ) userAgent = std::make_shared< string >( std::move( line ) );
        else if( headName == "Accept" ) accept = std::make_shared< string >( std::move( line ) );
        else if( headName == "Accept-Language" ) acceptLanguage = std::make_shared< string >( std::move( line ) );
        else if( headName == "Connection" ){
            connection = std::make_shared< string >( std::move( line ) );
            if( (*connection).size() >= 13 && (*connection).substr(12) == "keep-alive" ) m_keepConnection = true;
            }
        else if( headName == "Upgrade-Insecure-Requests" ) upgradeInsecurceRequests = std::make_shared< string >( std::move( line ) );
        else if( headName == "Content-Type" ) contentType = std::make_shared< string >( std::move( line ) );
        else if( headName == "Content-Length" ) contentLength = std::make_shared< string >( std::move( line ) );
    }
}

string HttpData::parseOneLine() {
    if( dataBufferEmpty() ) readRawDataFromSocket(); // 将数据读到dataBuffer中
    char lineBuffer[ LINE_BUFFERSIZE ];
    int index = 0;
    while( readIndex <= dataEndIndex ) {     
        lineBuffer[ index ] = dataBuffer[ readIndex ];
        ++index;
        ++readIndex;
        if( prev == '\r' && dataBuffer[ readIndex - 1 ] == '\n' ) { prev = 0; break;}
        prev = dataBuffer[ readIndex - 1 ];

        if( readIndex > dataEndIndex ) readRawDataFromSocket();
    }
    lineBuffer[ index - 2 ] = '\0';
    return string( lineBuffer );
}

void HttpData::readRawDataFromSocket() {
    int dataNum = recv(clientSocket, dataBuffer, HTTPDATA_BUFFERSIZE, 0);
    if( dataNum < 0 ) throw std::runtime_error("recv error!\n");
    if( dataNum == 0 ) throw SocketClosed();
    readIndex = 0;
    dataEndIndex = dataNum - 1;
}

string HttpData::getHeaderLineName( string &s ) {
    int i = 0;
    while(s[ i ] != ':') ++i;
    return s.substr(0, i);
}

void HttpData::stringSplit(const string& s, vector<string>& tokens, const string& delimiters) { // 分割字符串。默认用空格分割字符串
	string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	string::size_type pos = s.find_first_of(delimiters, lastPos);
	while ( string::npos != pos || string::npos != lastPos ) {
		tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}

void HttpData::getRequestMethod( const vector<string>& headerInfo ) {
    if( headerInfo.size() == 0 ) { // 应对空请求
        requestMethod = RequestMethod::UNSUPPORT; requestMethod_string = "UNSUPPORT";
        m_badRequest = true;
        return;
    }

    if( headerInfo[0] == "GET" ) { requestMethod = RequestMethod::GET; requestMethod_string = "GET"; }
    else if( headerInfo[0] == "POST" ) { requestMethod = RequestMethod::POST; requestMethod_string = "POST"; }
    else { requestMethod = RequestMethod::UNSUPPORT; requestMethod_string = "UNSUPPORT"; }
}

void HttpData::parseUrlDataParamList() {
    int pos = 0;
    while ( pos < url.size() && url[ pos ] != '?' ) ++pos; // 寻找请求参数开始位置
    if ( pos == url.size() || pos + 1 >= url.size() ) return; 
    // 请求参数通常会放在 URL 的查询字符串（Query String）中，查询字符串以问号（?）开头，后面是多个用 & 符号分隔的键值对。
    // 每个键值对由一个键和一个值组成，中间用等号（=）连接。
    // 如果在？之后没有请求资源直接return
    string paramStr = url.substr( pos + 1 ); // 存放请求的资源
    url = url.substr(0, pos);
    paramStr.push_back('&'); // 加入&到结尾方便分割
    parseStringParamTo_htable_clientParamData( paramStr );
}

void HttpData::parseStringParamTo_htable_clientParamData( string & str ) {
    if( clientParamData == nullptr ) clientParamData = std::make_shared< std::unordered_map< string, string> >();
    int cur = 0, prev = 0; // 用于分割&
    int start = 0 , mid = 0, end = 0; // 用于分割=
    while( cur < str.size() ) { // 注意cur的退出条件, 值有可能为空
        while( str[ cur ] != '&' ) ++cur; // 每一个键值对都以&结尾
        start = prev;
        mid = start;
        while( str[ mid ] != '=' ) ++mid;
        end = cur - 1;
        if( end == mid ) (*clientParamData)[ str.substr ( start, mid - start ) ] = "";
        // str.substr ( start, mid - start )===键
        // ""===值
        else (*clientParamData)[ str.substr ( start, mid - start ) ] = str.substr( mid + 1, end - mid );
        ++cur;
        prev = cur;
    } 
}

int HttpData::numeralContentLength() const { //当只有请求体存在时, Content-Length 字段才出现在请求头中，以便接收方可以正确地读取请求体。
    int pos = 0;
    while( (*contentLength)[pos] != ' ' ) ++pos;
    ++pos;
    return std::stoi( (*contentLength).substr( pos, (*contentLength).size() - pos ) ); // 用于将字符串转换为整数类型，并返回转换后的整数值
}

void HttpData::parseBodyParamData() {
    int contentLength = numeralContentLength();
    char* data = new char[ contentLength + 2 ];

    int index = 0;
    // 读取剩余的所有字节数据到变量data
    while( readIndex <= dataEndIndex ) {
        data[ index ]= dataBuffer[ readIndex ];
        ++index;
        ++readIndex;
        if( index == contentLength ) break;
        if( readIndex > dataEndIndex ) readRawDataFromSocket();
    }
    // fixme 目前只考虑了application/x-www-form-urlencoded的post数据格式
    data[ contentLength ] = '&'; // 手动将最后一位设置为&方便后续的处理
    data[ contentLength + 1 ] = 0;

    string s_data( data );
    parseStringParamTo_htable_clientParamData( s_data );
    delete data;
}