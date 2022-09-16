all: game
	@echo "DONE"

#Compiladores
CXX=g++
CC=gcc 


game: Entidades.o drawFunctions.o game.cpp 
	$(CXX) Entidades.o drawFunctions.o -O3 game.cpp -lm -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_mixer -lGL -lGLU -o game

lib: drawFunctions.cpp
	$(CXX) drawFunctions.cpp -lm -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_mixer -lGL -lGLU -c

clean:
	$(RM) *~ *.o game -f

testeE: testes/testeEntidades.cpp Entidades.cpp
	$(CXX) testes/testeEntidades.cpp Entidades.cpp

entidades: Entidades.cpp
	$(CXX) Entidades.cpp -c