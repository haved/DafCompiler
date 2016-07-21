#include <windowCreator.h>

int main(int argc, char** argv) {
    sf::Window* window = allocateWindow("Test", 800, 450);
	waitForClose(window);
    deallocateWindow(window);
}