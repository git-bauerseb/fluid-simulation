# Fluid simulation based on Navier-Stokes

A **very** simple fluid simulation tool written in C++ using SFML. The fluid simulation works by solving the Navier-Stokes equation for incompressible fluids using finite differences. Below is a screenshot of the current program you can find in **main.cpp**:

![fluid.png](Fluid simulation image)


## Run

Install SFML. Then do

```
g++ main.cpp  -lsfml-window -lsfml-system -lsfml-graphics && ./a.out
```
