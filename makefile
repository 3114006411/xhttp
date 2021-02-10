xhttp:server.cpp
	g++ $+ -g  -o $@ -I../xsocket  -std=c++11 -lpthread -lxsocket -L../xsocket
clean:
	rm -f  xhttp
