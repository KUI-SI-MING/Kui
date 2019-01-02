.RHONY:all
all:server client
cc=g++

client:TcpClient.cc
	$(cc) -o $@ $^
server:TcpServer.cc
	$(cc) -o $@ $^ -lpthread

.PHONE:clean
clean:
	rm -f client server
