ch_encoder:ch_encoder.o UDPSocket.o
	g++ -std=c++11 -w  -o ch_encoder check_encoder.o UDPSocket.o  -lavformat -lavutil
	rm -rf check_encoder.o UDPSocket.o
UDPSocket.o:include/UDPSocket.cpp include/UDPSocket.h
	g++ -c include/UDPSocket.cpp
ch_encoder.o:src/check_encoder.cpp include/recv_json.h include/UDPSocket.h 
	g++ -std=c++11 -c -w src/check_encoder.cpp #-lavformat -lavutil 
clean:
	rm -rf ch_encoder
