
#include "httpsession.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

namespace myserver {

HttpSession::HttpSession() :keepalive_(true), praseresult_(false) {

}

bool HttpSession::parseHttpRequest(std::string& str, HttpRequestContext& request) {
    std::string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0, pos_colon;
    std::string key, value;
    bool praseresult = false;
    // first line
    if ((next = str.find(crlf, prev)) != std::string::npos) {
        std::string firstline(str.substr(prev, next-prev));
        prev = next;
        std::stringstream ss(firstline);
        ss >> request.method;
        ss >> request.url;
        ss >> request.version;
    } else {
        std::cout << "error in http firstline parse..." << std::endl;
        std::cout << "msg:" << str << std::endl;
        str.clear();
        return praseresult;
    }
    // header
    size_t pos_crlfcrlf = 0;
    if ((pos_crlfcrlf = str.find(crlfcrlf, prev)) != std::string::npos) {
        while (prev != pos_crlfcrlf) {
            next = str.find(crlf, prev+2);
            pos_colon = str.find(":", prev+2);
            key = str.substr(prev+2, pos_colon - prev -2);
            value = str.substr(pos_colon+2, next - pos_colon-2);
            prev = next;
            request.header[key] = value;
        }
    } else {
        std::cout << "error in http hearder parse..." << std::endl;
        std::cout << "msg:" << str << std::endl;
        str.clear();
        return praseresult;
    }
    request.body = str.substr(pos_crlfcrlf + 4);
    praseresult = true;
    str.clear();
    return praseresult;
}

void HttpSession::httpProcess(const HttpRequestContext& request, std::string &response) {
    std::string responsebody;
    std::string errormsg;
    std::string path;
    std::string querystring;

    if (request.method == "GET") {
        ;
    } else if (request.method == "POST") {
        ;
    } else {
        std::cout << "undefine http method" << std::endl;
        httpError(501, "undefine http method", request, response);
        return;
    }

    size_t pos = request.url.find("?");
    if(pos != std::string::npos) {
        path = request.url.substr(0, pos);
        querystring = request.url.substr(pos+1);
    } else {
        path = request.url;
    }

    auto iter = request.header.find("Connection");
    if(iter != request.header.end()) {
        keepalive_ = (iter->second == "Keep-Alive");
    } else {
        if(request.version == "HTTP/1.1") {
            keepalive_ = true;
        } else {
            keepalive_ = false;
        }
    }

    if("/" == path) {
        path = "/index.html";
    } else if("/hello" == path) {
        // for Wenbbench
        std::string filetype("text/html");
        responsebody = ("hello world");
        response += request.version + " 200 OK\r\n";
        response += "Server: Chen Shuaihao's NetServer/0.1\r\n";
        response += "Content-Type: " + filetype + "; charset=utf-8\r\n";
        if(iter != request.header.end())
        {
            response += "Connection: " + iter->second + "\r\n";
        }
        response += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
        response += "\r\n";
        response += responsebody;
        return;
    }
    else
    {
        ;
    }

    path.insert(0,".");
    FILE* fp = NULL;
    if((fp = fopen(path.c_str(), "rb")) == NULL)
    {
        //perror("error fopen");
        //404 NOT FOUND
        httpError(404, "Not Found", request, response);
        return;
    }
    else
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        while(fread(buffer, sizeof(buffer), 1, fp) == 1)//可以mmap内存映射优化
        {
            responsebody.append(buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        if(feof(fp))
        {
            responsebody.append(buffer);
        }
        else
        {
            std::cout << "error fread" << std::endl;
        }
        fclose(fp);
    }

    std::string filetype("text/html"); //暂时固定为html
    response += request.version + " 200 OK\r\n";
    response += "Server: myserver\r\n";
    response += "Content-Type: " + filetype + "; charset=utf-8\r\n";
    if(iter != request.header.end())
    {
        response += "Connection: " + iter->second + "\r\n";
    }
    response += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
    response += "\r\n";
    response += responsebody;
}

void HttpSession::httpError(const int err_num, const std::string& err_msg, const HttpRequestContext& request, std::string &response) {
    std::string responsebody;
    responsebody += "<html><title>Error</title>";
    responsebody += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
    responsebody += "<style>body{backgrhttpversionound-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
    responsebody += "<body bgcolor=\"ffffff\"><h1>";
    responsebody += std::to_string(err_num) + " " + err_msg;
    responsebody += "</h1><hr><em> ... </em>\n</body></html>";

    std::string httpversion = request.version.empty() ? "HTTP/1.1" : request.version;

    response += httpversion + " " + std::to_string(err_num) + " " + err_msg + "\r\n";
    response += "Server : My Http Server\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Connection: Keep-Alive\r\n";
    response += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
    response += "\r\n";
    response += responsebody;
}

}
