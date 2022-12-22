#include <iostream>
#include <cmath>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


// Size of the simulation grid (NxN)
static const int N = 40;

// Velocity
// x component of the velocity vector field
static float u[(N+2)*(N+2)];
static float u_prev[(N+2)*(N+2)];

// y component of the velocity vector field
static float v[(N+2)*(N+2)];
static float v_prev[(N+2)*(N+2)];


// Density
// density scalar field
static float density[(N+2)*(N+2)];
static float density_prev[(N+2)*(N+2)];

#define INDX(i,j) ((i) + (N+2) * (j))


void initFluid() {
	
	float incStep = 1.0f / N;

	// Current y value
	float cY = 0.5f;

	// Curent x value
	float cX = -0.5f;

	for (int y = 0; y < N; y++) {
		for (int x = 0; x < N; x++) {
			

			// Density is only a 2D sphere in a small spot
			if ((cX+.25f) * (cX+.25f) + cY * cY <= 0.08) {
				density[INDX(x+1,y+1)] = 1.0f;
			}

			u[INDX(x+1,y+1)] = -10*cY*std::sin(1.57*cY);
			v[INDX(x+1,y+1)] = -10*cY*std::cos(1.57*cX);


			cX += incStep;
		}

		cX = -0.5f;
		cY -= incStep;
	}


}

void set_bnd(int b, float* x) {
	int i;

	for (i = 1; i <= N; i++) {
		x[INDX(0, i)]   = b == 1 ? -x[INDX(1,i)] : x[INDX(1,i)];
		x[INDX(N+1, i)] = b == 1 ? -x[INDX(N,i)] : x[INDX(N,i)];
		x[INDX(i, 0)]   = b == 2 ? -x[INDX(i,1)] : x[INDX(i,1)];
		x[INDX(i, N+1)] = b == 2 ? -x[INDX(i,N)] : x[INDX(i,N)];	
	}

	x[INDX(0, 0)]  = 0.5*(x[INDX(1,0)] + x[INDX(0,1)]);
	x[INDX(0,N+1)] = 0.5*(x[INDX(1,N+1)] + x[INDX(0,N)]);
	x[INDX(N+1,0)] = 0.5*(x[INDX(N,0)] + x[INDX(N+1, 1)]);
	x[INDX(N+1,N+1)] = 0.5*(x[INDX(N,N+1)] + x[INDX(N+1,N)]);
}

// Calculate diffusion term for density
void diffuse(
		int b,
		float* x,
		float* x0,
		float diff,
		float dt
		) {

	int i, j, k;
	float a = dt * diff * N * N;

	for (k = 0; k < 20; k++) {
		for (i = 1; i <= N; i++) {
			for (j = 1; j <= N; j++) {
			
				x[INDX(i,j)] = (x0[INDX(i,j)] + a * (x[INDX(i-1, j)]
								+ x[INDX(i+1,j)] + x[INDX(i,j-1)] + x[INDX(i,j+1)])) / (1 + 4*a);
			
			}
		}

		set_bnd(b,x);
	}
}

// Calculate advection term for density
void advect (int b, float * d, float * d0, float * u, float * v, float dt ) {
	int i, j, i0, j0, i1, j1;
	float x, y, s0, t0, s1, t1, dt0;
	dt0 = dt*N;
	for ( i=1 ; i<=N ; i++ ) {
		for ( j=1 ; j<=N ; j++ ) {
			x = i-dt0*u[INDX(i,j)]; y = j -dt0*v[INDX(i,j)];
			if (x<0.5) x=0.5; if (x>N+0.5) x=N+ 0.5; i0=(int)x; i1=i0+ 1;
			if (y<0.5) y=0.5; if (y>N+0.5) y=N+ 0.5; j0=(int)y; j1=j0+1;
			s1 = x-i0; s0 = 1-s1; t1 = y-j0; t0 = 1-t1;
			d[INDX(i,j)] = s0*(t0*d0[INDX(i0,j0)]+t1*d0[INDX(i0,j1)])+
			s1*(t0*d0[INDX(i1,j0)]+t1*d0[INDX(i1,j1)]);
		}
	}
	set_bnd (b, d );
}

void project(float* u, float* v, float* p, float* div) {
	int i, j, k;
	float h;

	h = 1.0 / N;
	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			div[INDX(i,j)] = -0.5*h*(u[INDX(i+1, j)] - u[INDX(i-1, j)] + v[INDX(i,j+1)] - v[INDX(i,j-1)]);

			p[INDX(i,j)] = 0;
		}
	}

	set_bnd(0, div);
	set_bnd(0, p);

	for (k = 0; k < 20; k++) {
		for (i = 1; i <= N; i++) {
			for (j = 1; j <= N; j++) {
				p[INDX(i,j)] = (div[INDX(i,j)] + p[INDX(i-1,j)] + p[INDX(i+1,j)] + p[INDX(i,j-1)]+p[INDX(i,j+1)]) / 4;
			}
		}

		set_bnd(0,p);
	}

	for (i=1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			u[INDX(i,j)] -= 0.5*(p[INDX(i+1,j)] - p[INDX(i-1,j)]) / h;
			v[INDX(i,j)] -= 0.5*(p[INDX(i,j+1)] - p[INDX(i,j-1)]) / h;
		}
	}

	set_bnd(1,u);
	set_bnd(2,v);
}


void density_step(float* x, float* x0, float* u, float* v, float diff, float dt) {

	// Update density field
	std::swap(x0, x);
	diffuse(0, x, x0, diff, dt);
	std::swap(x0, x);
	advect(0, x, x0, u, v, dt);
}

void velocity_step(float* u, float* v, float* u0, float* v0, float visc, float dt) {
	std::swap(u0, u);
	diffuse(1, u, u0, visc, dt);
	std::swap(v0, v);
	diffuse(2, v, v0, visc, dt);
	project(u, v, u0, v0);
	std::swap(u0, u);
	std::swap(v0, v);
	advect(1, u, u0, u0, v0, dt);
	advect(2, v, v0, u0, v0, dt);
	project(u, v, u0, v0);
}

void update(float dt) {
	velocity_step(u, v, u_prev, v_prev, 0.01, dt);
	density_step(density, density_prev, u, v, 0.001, dt);
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
void plotScalarField(sf::Uint8* pixels, float* scalarField) {
	for (int y = 0; y < N; y++) {
		for (int x = 0; x < N; x++) {
			int grayScale = static_cast<int>(255.0f * scalarField[INDX(x+1,y+1)]);
			pixels[y * N * 4 + 4 * x] = grayScale;
			pixels[y * N * 4 + 4 * x + 1] = grayScale;
			pixels[y * N * 4 + 4 * x + 2] = grayScale;
			pixels[y * N * 4 + 4 * x + 3] = 255;
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
	sf::Time timePerUpdate = sf::milliseconds(100.0f);


	// Create texture to draw
	
	sf::Texture texture;
	texture.create(N, N);
	texture.setSmooth(true);

	sf::Uint8* pixels = new sf::Uint8[N * N * 4];
	texture.update(pixels);
	

	// Initialize density and vector field
	initFluid();
	plotScalarField(pixels, density);


	sf::Sprite sprite;
	sprite.setScale(400.0f / N, 400.0f / N);
	sprite.setTexture(texture);

	// Poll events
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}


		update(clock.restart().asSeconds());
		plotScalarField(pixels, density);


		texture.update(pixels);

		window.clear();
		window.draw(sprite);
		window.display();



	}

	free(pixels);

	return 0;
}
