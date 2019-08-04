#ifndef HTTPSESSION_
#define HTTPSESSION_

#include <string>
#include <map>
#include <sstream>
namespace myserver {

typedef struct _HttpRequestContext {
        std::string method;
        std::string url;
        std::string version;
        std::map<std::string, std::string> header;
        std::string body;
}HttpRequestContext;

class HttpSession {
public:
    HttpSession();

    bool parseHttpRequest(std::string& str, HttpRequestContext& request);
    void httpProcess(const HttpRequestContext& request, std::string &response);
    void httpError(const int err_num, const std::string& err_msg, const HttpRequestContext& request, std::string &response);

    bool keepAlive() const {return keepalive_;}
private:
    bool keepalive_;

    //解析报文相关成员
    HttpRequestContext httprequestcontext_;
    bool praseresult_;

    //Http响应报文相关成员
    std::string responsecontext_;
    std::string responsebody_;

    std::string errormsg_;

    std::string path_;
    std::string querystring_;
    std::string body_;
};
}
#endif //HTTPSESSION_
