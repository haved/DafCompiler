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

dynamicExec: lib/libWC.so bin/main.o
	echo "bin/main.o -L lib/ -lWC -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o dynamicExec -rpath .

lib/libWC.so: lib/ bin/windowCreatorDynamic.o
	echo "bin/windowCreatorDynamic.o" > Linkfile
	dafln -o lib/libWC.so.1.2 -shared -soname libWC.so.1.0
	rm libWC.so.1.0
	ln -s lib/libWC.so.1.2 libWC.so.1.0

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
	rm normalExec staticExec dynamicExec libWC.so.1.0
