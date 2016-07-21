#include "windowCreator.h"

int main(int argc, char** argv) {
    sf::Window* window = allocateWindow("dafln test", 800, 450);
    deallocateWindow(window);
}