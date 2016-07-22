#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>

sf::Window* allocateWindow(const char* title, int width, int height) {
    std::cout << "Creating window!" << std::endl;
    return new sf::Window(sf::VideoMode(width, height, 32), title);
}
void deallocateWindow(sf::Window* window) {
    delete window;
}
void waitForClose(sf::Window* window) {
	glClearColor(0.4f, 0.7f, 1.0f, 0);
	while(window->isOpen()) {
		sf::Event event;
		while(window->pollEvent(event))
			if(event.type == sf::Event::Closed)
				window->close();
		glClear(GL_COLOR_BUFFER_BIT);
		window->display();
	}
}
int main() {
	sf::Window* window = allocateWindow("internal", 600, 400);
	waitForClose(window);
	deallocateWindow(window);
	return 0;
}