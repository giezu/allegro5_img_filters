main: main.cpp filters.hpp
	g++ -o main main.cpp filters.cpp -lallegro -lallegro_image -lallegro_primitives -std=c++11 --pedantic -Wall -Werror