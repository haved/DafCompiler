all: normalExec staticExec dynamicExec

normalExec: bin/windowCreator.o bin/main.o bin/doWindowStuffDynamic.o
	echo "bin/doWindowStuffDynamic.o bin/windowCreator.o bin/main.o -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o normalExec

staticExec: lib/libWC.a bin/main.o
	echo "bin/main.o lib/libWC.a bin/doWindowStuffDynamic.o -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o staticExec

lib/libWC.a: lib/ bin/windowCreator.o
	echo "bin/windowCreator.o" > Linkfile
	dafln -o lib/libWC.a -static

dynamicExec: lib/libDWS.so bin/windowCreator.o bin/main.o
	echo "bin/main.o bin/windowCreator.o -L ./lib -lDWS -lsfml-graphics -lsfml-window -lsfml-system -lGL" > Linkfile
	dafln -o dynamicExec -rpath ./lib

lib/libDWS.so: lib/ bin/doWindowStuffDynamic.o
	echo "bin/doWindowStuffDynamic.o" > Linkfile
	dafln -o lib/libDWS.so -shared -soname libDWS.so

bin/windowCreator.o: bin/ src/windowCreator.cpp
	g++ -c src/windowCreator.cpp -o bin/windowCreator.o

bin/doWindowStuffDynamic.o: bin/ src/doWindowStuff.cpp
	g++ -fPIC -c src/doWindowStuff.cpp -o bin/doWindowStuffDynamic.o -I include

bin/main.o: bin/ main.cpp
	g++ -c main.cpp -o bin/main.o -I ./include

bin/:
	mkdir bin

lib/:
	mkdir lib

clean:
	rm -r bin lib
	rm normalExec staticExec dynamicExec
