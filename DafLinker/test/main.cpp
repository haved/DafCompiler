#include <windowCreator.h>
#include <doWindowStuff.h>

int main(int argc, char** argv) {
    sf::Window* window = allocateWindow(getGoodName(), 800, 450);
	waitForClose(window);
    deallocateWindow(window);
}