#include <iostream>
#include <cmath>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

void update() {

}

void init(sf::Uint8* pixels, int width, int height) {

	float u = 0.f;
	float v = 0.f;

	float incr = 1.0f / width;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			pixels[y * width * 4 + 4*x] = static_cast<int>(255.999 * u);
			pixels[y * width * 4 + 4*x + 1] = static_cast<int>(255.999 * v);
			pixels[y * width * 4 + 4*x + 2] = 0;
			
			// Alpha
			pixels[y * width * 4 + 4*x + 3] = 255;
		
			u += incr;
		}

		u = 0;
		v += incr;
	}
}


// Given as input the pixels for a texture and a array of float values
// that represent the scalar field values), write a grayscale representation
// of the scalar values into the buffer.
void plotScalarField(sf::Uint8* pixels, int width, int height,
					float* scalarField, float maxVal, float minVal) {

	float intvLen = maxVal - minVal;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float normalized = (scalarField[x + y * width] - minVal) / intvLen;
			int grayScale = static_cast<int>(255.999 * normalized);
			pixels[y * width * 4 + 4 * x] = grayScale;
			pixels[y * width * 4 + 4 * x + 1] = grayScale;
			pixels[y * width * 4 + 4 * x + 2] = grayScale;


			pixels[y * width * 4 + 4 * x + 3] = 255;
		}
	}

}

int main() {

	sf::RenderWindow window(sf::VideoMode(400, 400), "Window");

	// Synchronize refresh rate with monitor
	// window.setVerticalSynEnabled(true);


	// Run 30 FPS
	// 33ms per frame
	// window.setFramerateLimit(2);

	sf::Clock clock;
	sf::Time timePerUpdate = sf::milliseconds(1000.0f / 2);


	// Create texture to draw
	int width = 40;
	int height = 40;
	
	sf::Texture texture;
	texture.create(width, height);
	texture.setSmooth(false);

	sf::Uint8* pixels = new sf::Uint8[width * height * 4];
	texture.update(pixels);


	float* scalarField = new float[width * height];

	float incr = 20.0f / width;
	float cY = 1;
	float cX = -1;

	auto scalarFunc = [](float x, float y) {return std::cos(x+y);};

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			scalarField[y * width + x] = scalarFunc(cX, cY);

			cX += incr;
		}

		cY -= incr;
		cX = -1;
	}


	// init(pixels, width, height);
	plotScalarField(pixels, width, height, scalarField, 1, -1);

	sf::Sprite sprite;
	sprite.setScale(400.0f / width, 400.0f / height);
	sprite.setTexture(texture);

	// Poll events
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		texture.update(pixels);

		window.clear();
		window.draw(sprite);
		window.display();

		sf::Time elapsed = clock.restart();
		if (elapsed < timePerUpdate) {
			int sleepAmount = timePerUpdate.asMilliseconds() - elapsed.asMilliseconds();
			sf::sleep(sf::milliseconds(sleepAmount));
			continue;
		}
	}

	free(scalarField);
	free(pixels);

	return 0;
}
