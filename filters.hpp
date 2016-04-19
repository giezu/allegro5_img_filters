#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <algorithm>
#include <string>
#include <random>
#include <iterator>

/**
 *	funkcje zazwyczaj przyjmują 1 argument (bitmapę do obróbki), ewentualnie
 *	opcjonalny argument, np w przypadku rozmycia jest to ilość iteracji.
 *	zwracają przetworzoną bitmapę (oryginał pozostaje niezmieniony)
*/

namespace filters
{
	namespace perlin
	{
		inline float	interpolated_noise_1d(float x);
		inline float	interpolated_noise_2d(float x, float y);
		
		/*
		 */
		float
		perlin_noise_1d(float x);

		/*
		 */
		float
		perlin_noise_2d(float x, float y, float p);
		
		/*
		 *			clouds width,	clouds height,	amplitude
		 *	ARGS:	unsigned int,	unsigned int,	float
		 *	RET:	ALLEGRO_BITMAP*
		 *	Generates clouds using perlin noise.
		 *	Returns generated clouds image with given resolution.
		 */
		ALLEGRO_BITMAP*
		clouds(unsigned int width, unsigned int height, float p);

		ALLEGRO_BITMAP*
		heightmap(ALLEGRO_BITMAP* source);
	}

	/*
	 *			from	,	to		,	time
	 *	ARGS:	double	,	double	,	double
	 *	RET:	double
	 *	Linear interpolation. Interpolates between two values with given time.
	 *	Returns interpolated value.
	 */
	double
	lerp(double a, double b, double x);

	/*
	 *			from	,	to		,	time
	 *	ARGS:	double	,	double	,	double
	 *	RET:	double
	 *	Cosine interpolation. Interpolates between two values with given time.
	 *	Returns interpolated value.
	 */
	double
	cosrp(double a, double b, double x);

	/*
	 *			before	,	from	,	to		,	after	,	time
	 *	ARGS:	double	,	double	,	double	,	double	,	double
	 *	RET:	double
	 *	Cubic interpolation. Interpolates between two values with given time.
	 *	Returns interpolated value.
	 */	
	double
	cubrp(double v0, double v1, double v2, double v3, double x);
	
	float	noise_1d(int x);
	float	noise_2d(int x, int y);
	float	smooth_noise1d(int x);
	float	smooth_noise2d(int x, int y);

	/*
	 *			source bitmap
	 *	ARGS: 	ALLEGRO_BITMAP*
	 *	RET:	ALLEGRO_BITMAP*
	 *	Turns source img to grayscale.
	 *	Returns grayscale image.
	 */
	ALLEGRO_BITMAP*
	grayscale(ALLEGRO_BITMAP* source);

	/*
	 *			source bitmap
	 *	ARGS: 	ALLEGRO_BITMAP*
	 *	RET:	ALLEGRO_BITMAP*
	 *	Turns source img to black/white.
	 *	Returns b/w image.
	 */
 	ALLEGRO_BITMAP*
 	black_white(ALLEGRO_BITMAP* source);
 	
 	/*
	 *			source bitmap	,	# of iterations
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Gaussian blur using convolution matrix, weights build with Gaussian curve.
 	 *	Returns blurred image.
	 */
	ALLEGRO_BITMAP*
 	gaussian_blur(ALLEGRO_BITMAP* source, unsigned int n = 1);
	
	/*
	 *			source bitmap	,	# of iterations
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Gaussian blur using convolution matrix, weights build with Gaussian curve.
 	 *	Returns blurred image.
	 */
	ALLEGRO_BITMAP*
 	gaussian_blur_optimized(ALLEGRO_BITMAP* source, unsigned int n = 1);

 	/*
	 *			source bitmap	,	# of iterations	,	# of samples
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	[int]			,	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Gaussian blur using convolution matrix, weights build with Gaussian curve.
 	 *	Optimized by sampling image. Returns blurred image.
	 */
	ALLEGRO_BITMAP*
	gaussian_blur_sampling(ALLEGRO_BITMAP* source, unsigned int n = 1, unsigned int samples = 2);

 	/*
	 *			source bitmap	,	# of iterations
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Gaussian blur using convolution matrix, weights are . Returns blurred image.
	 */
	ALLEGRO_BITMAP*
	box_blur(ALLEGRO_BITMAP* source, unsigned int n = 1);

 	/*
	 *			source bitmap	,	# of iterations	,	# of samples
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	[int]			,	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Gaussian blur using convolution matrix, weights are .
 	 *	Optimized by sampling image. Returns blurred image.
	 */
	ALLEGRO_BITMAP*
	box_blur_sampling(ALLEGRO_BITMAP* source, unsigned int n = 1, unsigned int samples = 2);
	
	/*
	 *			background image,	foreground image,	alpha value
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	ALLEGRO_BITMAP*	,	float
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Blends two images with given alpha. Returns blended image.
	 */
	ALLEGRO_BITMAP*
	alpha_blending(ALLEGRO_BITMAP* background, ALLEGRO_BITMAP* foreground, float alpha);

	/*
	 *			background image,	foreground image,	grayscale mask
 	 *	ARGS:	ALLEGRO_BITMAP*	, 	ALLEGRO_BITMAP*	,	ALLEGRO_BITMAP*
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Blends two images basing on grayscale mask. Returns blended image.
	 */
	ALLEGRO_BITMAP*
	alpha_blending(ALLEGRO_BITMAP* background, ALLEGRO_BITMAP* foreground, ALLEGRO_BITMAP* mask);

	/*
	 *			source image
 	 *	ARGS:	ALLEGRO_BITMAP*
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Sharpens source image. 
 	 *	Returns sharpened image
	 */
	ALLEGRO_BITMAP*
	sharpen(ALLEGRO_BITMAP* source);
 	
	/*
	 *			source image	,	light value
 	 *	ARGS:	ALLEGRO_BITMAP*	,	[int]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Lighten/darken source image.
 	 *	Returns image with changed lighting.
	 */
 	ALLEGRO_BITMAP*
 	lighten(ALLEGRO_BITMAP* source, int n = 1);
	
 	/*
 	 *			source image	,	contrast value
 	 *	ARGS:	ALLEGRO_BITMAP*	,	[float]
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Changes image contrast with given value.
 	 *	Returns image with changed contrast.
 	 */
	ALLEGRO_BITMAP*
	contrast(ALLEGRO_BITMAP* source, float n = 1.0);
 	
 	/*
 	 *			source image
 	 *	ARGS:	ALLEGRO_BITMAP*
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Changes source image channels.
 	 *	Returns image with changed channels.
 	 */
 	ALLEGRO_BITMAP*
 	tint(ALLEGRO_BITMAP* source);

 	/*
 	 *			source image
 	 *	ARGS:	ALLEGRO_BITMAP*
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Edge detection using convolution matrix.
 	 *	Returns edge detected image.
 	 */
 	ALLEGRO_BITMAP*
 	detect_edges(ALLEGRO_BITMAP* source);
	ALLEGRO_BITMAP*	gradient(	unsigned int width,
								unsigned int height,
								ALLEGRO_COLOR from,
								ALLEGRO_COLOR to);
	
	/*
 	 *			filename of binary file	,	width of output image
 	 *	ARGS:	std::string				,	unsigned int
 	 *	RET:	ALLEGRO_BITMAP*
 	 *	Opens binary file, then takes 3 x 8 bits per pixel.
 	 *	Returns image made from binary file
	 */
	ALLEGRO_BITMAP*
	file_to_img(std::string filename, unsigned int width);
	
	/*
	 *			source image	,	power of glitch
	 *	ARGS:	ALLEGRO_BITMAP*	,	unsigned int
	 *	RET:	ALLEGRO_BITMAP*
	 *	Glitches image using various effects.
	 *	Returns glitched image.
	 */
	ALLEGRO_BITMAP*
	glitch(ALLEGRO_BITMAP* source, unsigned int power);
}

namespace fractals
{
	void	draw_circle(int x, int y, float radius, ALLEGRO_BITMAP* background);
	void	draw_line(int x, int y, float length, ALLEGRO_BITMAP* background);
}
