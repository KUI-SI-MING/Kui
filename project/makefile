all:httpserver upload
	
httpserver:threadpool.hpp  httpserver.cpp utils.hpp
	g++ -g -std=c++0x $^ -o $@ -pthread -fpermissive
upload:upload.cpp utils.hpp
	g++ -g -std=c++0x $^ -o $@

.PHONY:clean

clean:
	rm -f httpserver upload                                   
