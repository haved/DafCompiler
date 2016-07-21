#include <SFML/Window.hpp>

sf::Window* allocateWindow(const char* title, int width, int height);
void deallocateWindow(sf::Window* window);