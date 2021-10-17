all: node.o node_send.o node_recv.o logger.o packet.o resident.o main.o node manager

main.o: src/main.cpp
	g++ -c -pthread src/main.cpp -o out/main.o

node.o: src/node.cpp
	g++ -c src/node.cpp -o out/node.o

node_send.o: src/node_send.cpp
	g++ -c src/node_send.cpp -o out/node_send.o

node_recv.o: src/node_recv.cpp
	g++ -c src/node_recv.cpp -o out/node_recv.o

logger.o: src/logger.cpp
	g++ -c src/logger.cpp -o out/logger.o

packet.o: src/packet.cpp
	g++ -c src/packet.cpp -o out/packet.o

resident.o: src/resident.cpp
	g++ -c src/resident.cpp -o out/resident.o

node: out/*
	g++ -pthread out/* -o bin/node

manager: src/manager.c
	gcc src/manager.c -o bin/manager

.PHONY: clean
clean:
	rm out/*
	rm bin/*