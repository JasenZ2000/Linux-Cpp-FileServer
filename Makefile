server:
	g++ util.cpp client.cpp -o client && \
	g++ util.cpp server.cpp Epoll.cpp Socket.cpp InetAddress.cpp -o server
clean:
	rm server && rm client