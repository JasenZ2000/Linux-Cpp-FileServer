src=$(wildcard src/*.cpp)

server:
	g++ -std=c++14 -pthread -g \
	$(src) \
	server.cpp \
	-o server
	
client:
	g++ src/util.cpp client.cpp -o client

clean:
	rm server && rm client