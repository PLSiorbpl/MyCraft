@echo off
echo Compiling Client
g++ src/Client.cpp -o Client -I../MyCraft/Include -IInclude -IInclude/asio -pthread -lws2_32 -lmswsock
echo Compiling Server
g++ src/Server.cpp -o server -I../MyCraft/Include -IInclude -IInclude/asio -pthread -lws2_32 -lmswsock
echo Ended
pause