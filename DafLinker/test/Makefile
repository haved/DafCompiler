all: openWindow

openWindow: bin/windowCreator.o bin/main.o
	echo "bin/windowCreator.o bin/main.o -lsfml-window -lsfml-graphics -lsfml-system" > Linkfile
	dafln -o openWindow

bin/windowCreator.o: bin/
	g++ -c windowCreator.cpp -o bin/windowCreator.o

bin/main.o: bin/
	g++ -c main.cpp -o bin/main.o

bin/:
	mkdir bin
