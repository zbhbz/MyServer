#include <iostream>
#include "httpserver.h"
#include "eventloop.h"
#include <signal.h>

myserver::EventLoop *lp;

static void sighandler1(int sig_no)
{
      exit(0);
}
static void sighandler2(int sig_no)
{
    lp->quit();
}

int main()
{
    signal(SIGUSR1, sighandler1);
    signal(SIGUSR2, sighandler2);
    signal(SIGINT, sighandler2);
    signal(SIGPIPE, SIG_IGN);

    const int port = 1998;
    const int iothreadnum = 4;
    const int workthreadnum = 2;

    myserver::EventLoop loop;
    lp = &loop;

    myserver::HttpServer server(&loop, port, iothreadnum, workthreadnum);
    server.start();
    try {
        loop.loop();
    } catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
