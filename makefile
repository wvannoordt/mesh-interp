main: setup
	g++ -O3 -I. -c AveragedFlowField.cpp -o obj/AveragedFlowField.o
	g++ -O3 -I. -c P3dMesh.cpp -o obj/P3dMesh.o
	g++ -O3 -I. -c main.cc -o obj/main.o
	g++ ./obj/*.o -o program

run: main
	./program

setup:
	mkdir -p ./obj

clean:
	rm -rf ./obj
	rm -f program
