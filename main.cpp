#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>

#include "filters.hpp"

/*
 * jak korzystać programu: w linuxie jest prosto, nazwę pliku do obróbki
 * należy podać jako argument programu np: ./main nazwa_pliku.jpg
 * na windowsie pewnie trzeba jakoś nawigować do folderu ze skompilowanym
 * programem i włączyć analogicznie. jeśli to się nie powiedzie to
 * można wpisać nazwę pliku w kodzie przy wywołaniu funkcji al_load_bitmap(char*);
 * Do działania wymagana jest biblioteka Allegro5.
 * Wywołanie funkcji ogranicza się do filters::nazwa_funkcji(ALLEGRO_BITMAP*);
 * Allegro obsługuje tylko niektóre rozszerzenia plików: BMP, PCX, TGA, JPEG, PNG
 */

int main(int argc, char const *argv[])
{
	// wzór do losowania zmiennej p
	//1.0 / ((filters::noise_1d(rand()) + 1) / 2.0 + 1
	// inicjalizacja biblioteki
	al_init();
	al_init_image_addon();
	al_init_primitives_addon();

	std::chrono::system_clock::time_point	start_timer;
	std::chrono::system_clock::time_point	end_timer;

	ALLEGRO_BITMAP*	source		=	al_load_bitmap(argv[1]);
	start_timer	=	std::chrono::high_resolution_clock::now();
	ALLEGRO_BITMAP*	output		=	filters::gaussian_blur_optimized(source, 2);
	end_timer	=	std::chrono::high_resolution_clock::now();
	std::cout	<<	std::chrono::duration_cast<std::chrono::seconds>(end_timer - start_timer).count()	<<	std::endl;

	start_timer	=	std::chrono::high_resolution_clock::now();
	ALLEGRO_BITMAP*	gauss_old	=	filters::gaussian_blur(source, 2);
	end_timer	=	std::chrono::high_resolution_clock::now();
	std::cout	<<	std::chrono::duration_cast<std::chrono::seconds>(end_timer - start_timer).count()	<<	std::endl;
	
	al_save_bitmap("gauss.jpg", output);
	al_save_bitmap("gauss_old.jpg", gauss_old);

	return 0;
}
