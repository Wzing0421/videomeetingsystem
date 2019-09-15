#designed by lee
objects: main.o UDPSocket.o
	g++  main.o UDPSocket.o -o /home/ljw/workspace/udpsend/UDPProject/myvideo
	rm main.o UDPSocket.o
UDPSocket.o: UDPSocket.cpp UDPSocket.h
	g++ -c UDPSocket.cpp 
main.o: main.cpp UDPSocket.h
	g++ -c main.cpp
clean:
	rm myvideo
