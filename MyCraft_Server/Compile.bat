@echo off
echo Compiling Client
g++ src/Client.cpp -o Client -I Include -I Include/asio -pthread -lws2_32
echo Compiling Server
g++ src/Server.cpp -o server -I Include -I Include/asio -pthread -lws2_32
echo Ended
pause