all: normalExec staticExec dynamicExec

normalExec: bin/windowCreator.o bin/main.o
	echo "bin/windowCreator.o bin/main.o -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o normalExec

staticExec: lib/libWC.a bin/main.o
	echo "bin/main.o lib/libWC.a -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o staticExec

lib/libWC.a: lib/ bin/windowCreator.o
	echo "bin/windowCreator.o" > Linkfile
	dafln -o lib/libWC.a -static

dynamicExec: lib/libWC.so.1.2 bin/main.o
	echo "bin/main.o lib/libWC.so.1.2 -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o dynamicExec -rpath ./lib

lib/libWC.so.1.2: lib/ bin/windowCreatorDynamic.o
	echo "bin/windowCreatorDynamic.o" > Linkfile
	dafln -o lib/libWC.so.1.2 -shared -soname libWC.so.1.2

bin/windowCreator.o: bin/ src/windowCreator.cpp
	g++ -c src/windowCreator.cpp -o bin/windowCreator.o

bin/windowCreatorDynamic.o: bin/ src/windowCreator.cpp
	g++ -fPIC -c src/windowCreator.cpp -o bin/windowCreatorDynamic.o -I include

bin/main.o: bin/ main.cpp
	g++ -c main.cpp -o bin/main.o -I ./include

bin/:
	mkdir bin

lib/:
	mkdir lib

clean:
	rm -r bin lib
	rm normalExec staticExec dynamicExec
