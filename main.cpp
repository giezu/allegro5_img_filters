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

	ALLEGRO_BITMAP*	source	=	al_load_bitmap(argv[1]);
	ALLEGRO_BITMAP*	output	=	filters::glitch(source, 50);
	al_save_bitmap("glitch.jpg", output);

	return 0;
}
