#include "windowCreator.h"
#include <iostream>

sf::Window* allocateWindow(const char* title, int width, int height) {
    std::cout << "Creating window!" << std::endl;
    return new sf::Window(sf::VideoMode(width, height, 32), title);
}
void deallocateWindow(sf::Window* window) {
    delete window;
}