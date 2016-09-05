compile:server client
server:	
	g++ my_server.cpp -w -o server
client:
	g++ my_client.cpp -w -o client
clean:
	rm -rf client server

