#designed by lee
objects: main.o UDPSocket.o
	g++  main.o UDPSocket.o -o myrecv
	rm main.o UDPSocket.o
UDPSocket.o: include/UDPSocket.cpp include/UDPSocket.h
	g++ -c include/UDPSocket.cpp 
main.o: src/main.cpp include/UDPSocket.h
	g++ -std=c++11 -c src/main.cpp -pthread
clean:
	rm myrecv
