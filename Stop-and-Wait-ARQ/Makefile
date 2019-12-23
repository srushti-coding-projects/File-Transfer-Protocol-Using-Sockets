all:
	g++ server.cpp unrel_sendto.cpp -Wl,-wrap=sendto -Wl,-wrap=recvfrom -o server -lrt
	g++ client.cpp unrel_sendto.cpp -Wl,-wrap=sendto -Wl,-wrap=recvfrom -o client -lrt

reliable:
	g++ -o client client.cpp
	g++ -o server server.cpp -lrt

clean:	
	client server client_server.txt

