
set(mycpp
    channel.cpp
    socket.cpp
    poller.cpp
    eventloop.cpp
    eventloopthread.cpp
    eventloopthreadpool.cpp
    tcpconnection.cpp
    tcpserver.cpp
    threadpool.cpp
    echoserver.cpp
    timer.cpp
    timermanage.cpp
    httpsession.cpp
    httpserver.cpp
    main.cpp
)

add_executable(WebServer ${mycpp})
