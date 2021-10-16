g++ -pthread src/*.cpp -o out/node.o && clear && out/node.o 0 example_topology/testinitcosts0 logs/test0.log
gcc src/manager.c -o out/manager.o && clear && out/manager.o 0 cost 1 100