TCP=$(wildcard tcp/*.cpp)

server:
	g++ -std=c++14 -pthread -g \
	$(TCP) \
	echo_server.cpp \
	-o server
	
client:
	g++ -pthread client.cpp -o client

clean:
	rm server && rm client
