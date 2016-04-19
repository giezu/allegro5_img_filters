#include "filters.hpp"

#include <iostream>
#include <fstream>
#include <ctime>

inline double
filters::lerp(double a, double b, double x)
{
	return a * (1 - x) + b * x;
}

inline double
filters::cosrp(double a, double b, double x)
{
	double 	ft	=	x	*	3.1415927;
	double	f	=	(1	-	cos(ft))	*	0.5;
	return	a	*	(1	-	f)	+	b	*	f;
}

inline double
filters::cubrp(double v0, double v1, double v2, double v3, double x)
{
	double	P	=	(v3	-	v2)	-	(v0	-	v1);
	double	Q	=	(v0	-	v1)	-	P;
	double	R	=	v2	-	v0;
	double	S	=	v1;

	return	(P	*	pow(x, 3)	+	Q	*	pow(x, 2)	+	R	*	x	+	S);
}

inline float
filters::noise_1d(int x)
{
	x	=	(x	<<	13)	^	x;
	return	(1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline float
filters::noise_2d(int x, int y)
{
    int	n	=	x	+	y	*	57;
    n	=	(n	<<	13)	^ 	n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline float
filters::smooth_noise1d(int x)
{
	return	noise_1d(x) / 2	+	noise_1d(x - 1) / 4	+	noise_1d(x + 1) / 4;
}

inline float
filters::smooth_noise2d(int x, int y)
{
    float	corners = (noise_2d(x - 1, y - 1) + noise_2d(x + 1, y - 1) + noise_2d(x - 1, y + 1) + noise_2d(x + 1, y + 1) ) / 16;
    float	sides   = (noise_2d(x - 1, y) + noise_2d(x + 1, y) + noise_2d(x, y - 1) + noise_2d(x, y + 1)) / 8;
    float	center  = noise_2d(x, y) / 4;

    return corners + sides + center;
}

inline float
filters::perlin::interpolated_noise_1d(float x)
{
	int		integer_X		=	int(x);
	float	fractional_X	=	x	-	integer_X;
	
	float	v1 = smooth_noise1d(integer_X);
	float	v2 = smooth_noise1d(integer_X + 1);

	return cosrp(v1, v2, fractional_X);
}

float
filters::perlin::perlin_noise_1d(float x)
{
	float	total = 0;
	float	p	=	1.0 / 4.0;
	int		n	= 	7;

	for (int i = 0; i < n; ++i)
	{
		int		frequency = pow(2, i);		
		float	amplitude = pow(p, i);
		total 	= 	total	+	interpolated_noise_1d(x * frequency) * amplitude;
	}

	return total;
}

inline float
filters::perlin::interpolated_noise_2d(float x, float y)
{
	int		integer_X    = int(x);
	float	fractional_X = x - integer_X;

	int		integer_Y    = int(y);
	float	fractional_Y = y - integer_Y;

	float	v1 = smooth_noise2d(integer_X,     integer_Y);
	float	v2 = smooth_noise2d(integer_X + 1, integer_Y);
	float	v3 = smooth_noise2d(integer_X,     integer_Y + 1);
	float	v4 = smooth_noise2d(integer_X + 1, integer_Y + 1);

	float	i1 = cosrp(v1 , v2 , fractional_X);
	float	i2 = cosrp(v3 , v4 , fractional_X);

	return cosrp(i1 , i2 , fractional_Y);
}

float
filters::perlin::perlin_noise_2d(float x, float y, float p)
{
	srand(time(NULL));
	float	total = 0;
	//float	p	=	1.0 / 1.2;
	int		n	= 	16;

	for (int i = 0; i < n; ++i)
	{
		int		frequency 	= 	pow(2, i);
		float	amplitude	=	pow(p, i);
		total	=	total	+	interpolated_noise_2d(x * frequency, y * frequency)	*	amplitude;
	}

	return total;
}

ALLEGRO_BITMAP*
filters::perlin::clouds(unsigned int width, unsigned int height, float p)
{
	ALLEGRO_BITMAP*	output	=	al_create_bitmap(width, height);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	srand(time(NULL));
	unsigned int	seed	=	rand() % 10000000;

	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			int	val	=	(perlin_noise_2d((float) (x + seed) / width , (float) (y + seed) / height, p) * 127)	+ 127;
			val	=	std::min(std::max(val, 0), 255);
			al_put_pixel(x, y, al_map_rgb(val, val, val));
		}
	}

	al_unlock_bitmap(output);
	return output;
}

ALLEGRO_BITMAP*
filters::perlin::heightmap(ALLEGRO_BITMAP* source)
{
	unsigned int 	img_w	=	al_get_bitmap_width(source);
	unsigned int	img_h	=	al_get_bitmap_height(source);

	ALLEGRO_BITMAP*	output	=	al_create_bitmap(img_w, img_h);

	ALLEGRO_COLOR	colors[3]	=	{	al_map_rgb(0, 0, 255),
										al_map_rgb(0, 255, 0),
										al_map_rgb(255, 0, 0)};

	unsigned char	pxl[3];
	unsigned char	color[3];
	unsigned char	hill[3]	=	{0, 255, 0};
	unsigned char	mntn[3]	=	{255, 0, 0};

	ALLEGRO_COLOR	out_pxl;

	al_set_target_bitmap(output);
	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&pxl[0],
							&pxl[1],
							&pxl[2]);
			
			unsigned int no_of_color	=	floor(pxl[0] / 256.0 * 3);
			if (no_of_color > 0)
			{
				al_unmap_rgb(	colors[no_of_color],
								&color[0],
								&color[1],
								&color[0]);
				for (int i = 0; i < 3; ++i)
				
					pxl[i]	=	lerp(hill[i], mntn[i], pxl[i] / 256.0);
				
				out_pxl	=	al_map_rgb(	pxl[0],
										pxl[1],
										pxl[2]);
			}

			else
			{
				out_pxl	=	al_map_rgb(	0,
										0,
										255 * (pxl[0] / (256.0 / 3)));
			}

			al_put_pixel(x, y, out_pxl);
		}
	}

	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

ALLEGRO_BITMAP*
filters::glitch(ALLEGRO_BITMAP* source, unsigned int power)
{
	unsigned int	img_w	=	al_get_bitmap_width(source);
	unsigned int	img_h	=	al_get_bitmap_height(source);

	unsigned int	rand_source_x	=	0;
	unsigned int	rand_source_y	=	0;

	unsigned int	rand_output_x	=	0;
	unsigned int	rand_output_y	=	0;

	unsigned int	rand_matrix_w	=	0;
	unsigned int	rand_matrix_h	=	0;
/*
	unsigned int	line_beg		=	0;
	unsigned int	line_end		=	0;
*/
	unsigned char	pixel[3];

	unsigned int	mode	=	0;

	ALLEGRO_BITMAP*	output	=	al_create_bitmap(img_w, img_h);

	srand(time(NULL));

    std::vector<int> 	v	=	{0, 1, 2};
	std::random_device 	rd;
	std::mt19937 		g(rd());

	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);
	al_set_target_bitmap(output);

	for (unsigned int i = 0; i < img_h; ++i)
	{
		for (unsigned int j = 0; j < img_w; ++j)
		{
			al_unmap_rgb(	al_get_pixel(	source,
											j,
											i),
							&pixel[0],
							&pixel[1],
							&pixel[2]);

			al_set_target_bitmap(output);
			al_put_pixel(j, i, al_map_rgb(	pixel[0],
											pixel[1],
											pixel[2]));
		}
	}

	for (unsigned int n = 0; n < power; ++n)
	{
		rand_source_x	=	g()	%	img_w;
		rand_source_y	=	g()	%	img_h;

		rand_output_x	=	g()	%	img_w;
		rand_output_y	=	g()	%	img_h;

		rand_matrix_w	=	g()	%	img_w / 4;
		rand_matrix_h	=	g()	%	img_h / 8;

		mode	=	g()	%	100;
		if (mode<=	33)
		{
			for (unsigned int i = 0; i < rand_matrix_h / 2; ++i)
			{
				for (unsigned int j = 0; j < rand_matrix_w / 2; ++j)
				{
					al_put_pixel(	(rand_output_x	+	g() % rand_matrix_w)	%	img_w,
									(rand_output_y	+	g()	% rand_matrix_h)	%	img_h,
									al_map_rgb(	g() %	256,
												g()	%	256,
												g()	%	256));
				}
			}
		}

		else if (mode >	33)
		{
	    	std::shuffle(v.begin(), v.end(), g);
		
			for (unsigned int i = 0; i < rand_matrix_h; ++i)
			{
				for (unsigned int j = 0; j < rand_matrix_w; ++j)
				{
					al_unmap_rgb(	al_get_pixel(	source,
													(rand_source_x	+	j)	%	img_w,
													(rand_source_y	+	i)	%	img_h),
									&pixel[0],
									&pixel[1],
									&pixel[2]);

					
					al_put_pixel(	(rand_output_x	+	j)	%	img_w,
									(rand_output_y	+	i)	%	img_h,
									al_map_rgb(	pixel[v[0]],
												pixel[v[1]],
												pixel[v[2]]));				
				}
			}
		}
	}

	for (unsigned int i = 0; i < img_h; ++i)
	{
		if (i & 1)
		{
			al_draw_line(0, i, img_w, i, al_map_rgb(0, 0, 0), 1.0);
			/*
			line_beg	=	g()	%	img_w;
			line_end	=	g()	%	(img_w 	/ 2);
			al_draw_line(line_beg, i, (line_beg + line_end) % img_w, i, al_map_rgb(0, 0, 0), g() % 2);
			*/
		}
	}

	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

// działa
ALLEGRO_BITMAP*
filters::grayscale(ALLEGRO_BITMAP* source)
{
	unsigned int		img_w	=	al_get_bitmap_width(source);
	unsigned int		img_h	=	al_get_bitmap_height(source);
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	unsigned char		r	=	0;
	unsigned char		g	=	0;
	unsigned char		b	=	0;
	unsigned char		avg	=	0;

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_set_target_bitmap(source);			
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&r,
							&g,
							&b
						);
			avg	=	(int)((r + g + b) / 3.0);
			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(avg, avg, avg));
		}
	}

	return output;
}

// działa
ALLEGRO_BITMAP*
filters::black_white(ALLEGRO_BITMAP* source)
{
	unsigned int		img_w	=	al_get_bitmap_width(source);
	unsigned int		img_h	=	al_get_bitmap_height(source);
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	unsigned char		r	=	0;
	unsigned char		g	=	0;
	unsigned char		b	=	0;
	unsigned char		avg	=	0;
	unsigned char		bw	=	0;

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_set_target_bitmap(source);			
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&r,
							&g,
							&b
						);
			avg	=	(int)((r + g + b) / 3.0);
			bw	=	avg > 127 ? 255 : 0;
			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(bw, bw, bw));
		}
	}

	return output;
}

// działa
ALLEGRO_BITMAP*
filters::gaussian_blur(ALLEGRO_BITMAP* source, unsigned int n)
{
	if (!n) return source;
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	7;
	const	unsigned int	matrix_h	=	7;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	int					kernel	=	0;

	int				filter_matrix[matrix_h][matrix_w]	=
	{ 
		{0,		0,		0,		5,		0,		0,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		18,		64,		100,	64,		18,		0},
		{5,		32,		100,	100,	100,	32,		5},
		{0,		18,		64,		100,	64,		18,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		0,		0,		5,		0,		0,		0}
	};

	for (unsigned int y = 0; y < matrix_h; ++y)
	
		for (unsigned int x = 0; x < matrix_w; ++x)
		
			kernel	+=	filter_matrix[y][x];

	float				factor	=	1.0/kernel;
	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int y = 0; y < img_h; ++y)
		{
			for (unsigned int x = 0; x < img_w; ++x)
			{
				al_set_target_bitmap(source);
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - (matrix_w / 2)) % img_w, (img_h + y + i - (matrix_h / 2)) % img_h);
					}
				}
				
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						al_unmap_rgb(	pixels[i][j],
										&tmp_channels[0][i][j],
										&tmp_channels[1][i][j],
										&tmp_channels[2][i][j]);
					}
				}

				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
						g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
						b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
					}
				}

				results[0]	=	std::min(std::max(int(factor * r), 0), 255);
				results[1]	=	std::min(std::max(int(factor * g), 0), 255);
				results[2]	=	std::min(std::max(int(factor * b), 0), 255);

				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
			}
		}
		source	=	output;
	}
	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

ALLEGRO_BITMAP*
filters::gaussian_blur_optimized(ALLEGRO_BITMAP* source, unsigned int n)
{
	// TODO
	// dodać odejmowanie i dodawanie pixeli na krawędzi
	if (!n) return source;
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	7;
	const	unsigned int	matrix_h	=	1;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	int					kernel	=	0;
/*
	int				filter_matrix[matrix_h][matrix_w]	=
	{ 
		{0,		0,		0,		5,		0,		0,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		18,		64,		100,	64,		18,		0},
		{5,		32,		100,	100,	100,	32,		5},
		{0,		18,		64,		100,	64,		18,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		0,		0,		5,		0,		0,		0}
	};
*/
	int	filter_matrix[matrix_h][matrix_w]	=
	{
		{5,		32,		100,	100,	100,	32,		5}		
	};

	for (unsigned int y = 0; y < matrix_h; ++y)
	
		for (unsigned int x = 0; x < matrix_w; ++x)
		
			kernel	+=	filter_matrix[y][x];

	float				factor	=	1.0/kernel;
	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	// first pass

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int y = 0; y < img_h; ++y)
		{
			for (unsigned int x = 0; x < img_w; ++x)
			{
				al_set_target_bitmap(source);
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					pixels[0][j]	=	al_get_pixel(source, (img_w + x + j - (matrix_w / 2)) % img_w, (img_h + y - (matrix_h / 2)) % img_h);
				}
				
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					al_unmap_rgb(	pixels[0][j],
									&tmp_channels[0][0][j],
									&tmp_channels[1][0][j],
									&tmp_channels[2][0][j]);
				}

				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
						g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
						b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
					}
				}

				results[0]	=	std::min(std::max(int(factor * r), 0), 255);
				results[1]	=	std::min(std::max(int(factor * g), 0), 255);
				results[2]	=	std::min(std::max(int(factor * b), 0), 255);

				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
			}
		}
		source	=	output;
	}

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			for (unsigned int y = 0; y < img_h; ++y)
			{
				al_set_target_bitmap(source);
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						pixels[i][j]	=	al_get_pixel(source, (img_w + x + i - (matrix_h / 2)) % img_w, (img_h + y + j - (matrix_w / 2)) % img_h);
					}
				}
				
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						al_unmap_rgb(	pixels[i][j],
										&tmp_channels[0][i][j],
										&tmp_channels[1][i][j],
										&tmp_channels[2][i][j]);
					}
				}

				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
						g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
						b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
					}
				}

				results[0]	=	std::min(std::max(int(factor * r), 0), 255);
				results[1]	=	std::min(std::max(int(factor * g), 0), 255);
				results[2]	=	std::min(std::max(int(factor * b), 0), 255);

				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
			}
		}
		source	=	output;
	}

	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

// działa
ALLEGRO_BITMAP*
filters::gaussian_blur_sampling(ALLEGRO_BITMAP* source, unsigned int n, unsigned int samples)
{
	if (!n) return source;
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	7;
	const	unsigned int	matrix_h	=	7;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	int					kernel	=	0;
	float				factor	=	0;
	int					filter_matrix[matrix_h][matrix_w]	=
	{ 
		{0,		0,		0,		5,		0,		0,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		18,		64,		100,	64,		18,		0},
		{5,		32,		100,	100,	100,	32,		5},
		{0,		18,		64,		100,	64,		18,		0},
		{0,		5,		18,		32,		18,		5,		0},
		{0,		0,		0,		5,		0,		0,		0}
	};

	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int y = 0; y < img_h; ++y)
		{
			for (unsigned int x = 0; x < img_w; ++x)
			{
				al_set_target_bitmap(source);
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - (matrix_w / 2)) % img_w, (img_h + y + i - (matrix_h / 2)) % img_h);
					}
				}
				
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						al_unmap_rgb(	pixels[i][j],
										&tmp_channels[0][i][j],
										&tmp_channels[1][i][j],
										&tmp_channels[2][i][j]);
					}
				}


				for (unsigned int i = 0; i < samples; ++i)
				{
					int	sample	=	rand()	%	(matrix_h * matrix_w);
					int	x		=	sample % matrix_w;
					int	y		=	(int)((float)sample / matrix_h);
					r	+=	tmp_channels[0][y][x]	*	filter_matrix[y][x];
					g	+=	tmp_channels[1][y][x]	*	filter_matrix[y][x];
					b	+=	tmp_channels[2][y][x]	*	filter_matrix[y][x];
					kernel	+=	filter_matrix[y][x];
				}

				factor	=	1.0	/	kernel;
				results[0]	=	std::min(std::max(int(factor * r), 0), 255);
				results[1]	=	std::min(std::max(int(factor * g), 0), 255);
				results[2]	=	std::min(std::max(int(factor * b), 0), 255);
				kernel	=	0;
				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
			}
		}
		source	=	output;
	}
	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

// działa
ALLEGRO_BITMAP*
filters::box_blur(ALLEGRO_BITMAP* source, unsigned int n)
{
	if (!n) return source;
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	3;
	const	unsigned int	matrix_h	=	3;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	int					kernel	=	0;
	int					filter_matrix[matrix_h][matrix_w]	=
	{ 
		{1,	1,	1},
		{1,	1,	1},
		{1,	1,	1}
	};

	for (unsigned int i = 0; i < matrix_h; ++i)
	{
		for (unsigned int j = 0; j < matrix_w; ++j)
		{
			kernel	+=	filter_matrix[i][j];
		}
	}

	float				factor	=	1.0/kernel;

	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int y = 0; y < img_h; ++y)
		{
			for (unsigned int x = 0; x < img_w; ++x)
			{
				al_set_target_bitmap(source);
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - (matrix_w / 2)) % img_w, (img_h + y + i - (matrix_h / 2)) % img_h);
					}
				}
				
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						al_unmap_rgb(	pixels[i][j],
										&tmp_channels[0][i][j],
										&tmp_channels[1][i][j],
										&tmp_channels[2][i][j]);
					}
				}

				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
						g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
						b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
					}
				}

				results[0]	=	std::min(std::max(int(factor * r), 0), 255);
				results[1]	=	std::min(std::max(int(factor * g), 0), 255);
				results[2]	=	std::min(std::max(int(factor * b), 0), 255);

				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
			}
		}
		source	=	output;
	}
	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

// działa
ALLEGRO_BITMAP*
filters::box_blur_sampling(ALLEGRO_BITMAP* source, unsigned int n, unsigned int samples)
{
	if (!n) return source;
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	3;
	const	unsigned int	matrix_h	=	3;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	int					kernel	=	0;
	float				factor	=	0.0;
	int					filter_matrix[matrix_h][matrix_w]	=
	{ 
		{1,	1,	1},
		{1,	1,	1},
		{1,	1,	1}
	};

	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));

	al_lock_bitmap(source, al_get_bitmap_format(source), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int i = 0; i < n; ++i)
	{
		for (unsigned int y = 0; y < img_h; ++y)
		{
			for (unsigned int x = 0; x < img_w; ++x)
			{
				al_set_target_bitmap(source);

				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - (matrix_w / 2)) % img_w, (img_h + y + i - (matrix_h / 2)) % img_h);
					}
				}
				
				for (unsigned int i = 0; i < matrix_h; ++i)
				{
					for (unsigned int j = 0; j < matrix_w; ++j)
					{
						al_unmap_rgb(	pixels[i][j],
										&tmp_channels[0][i][j],
										&tmp_channels[1][i][j],
										&tmp_channels[2][i][j]);
					}
				}

				for (unsigned int i = 0; i < samples; ++i)
				{
					int	sample	=	rand()	%	(matrix_h * matrix_w);
					int	x		=	sample % matrix_w;
					int	y		=	(int)((float)sample / matrix_h);
					r	+=	tmp_channels[0][y][x]	*	filter_matrix[y][x];
					g	+=	tmp_channels[1][y][x]	*	filter_matrix[y][x];
					b	+=	tmp_channels[2][y][x]	*	filter_matrix[y][x];
					kernel	+=	filter_matrix[y][x];
				}
				factor	=	1.0 / kernel;
				results[0]	=	std::min(std::max((int) (r	*	factor), 0), 255);
				results[1]	=	std::min(std::max((int) (g	*	factor), 0), 255);
				results[2]	=	std::min(std::max((int) (b	*	factor), 0), 255);
				al_set_target_bitmap(output);
				al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
				r			= 	g			=	b			=	0;
				results[0]	=	results[1]	=	results[2]	=	0;
				kernel	=	0;
			}
		}
		source	=	output;
	}
	al_unlock_bitmap(source);
	al_unlock_bitmap(output);
	return output;
}

ALLEGRO_BITMAP*
filters::alpha_blending(ALLEGRO_BITMAP* background, ALLEGRO_BITMAP* foreground, ALLEGRO_BITMAP* mask)
{
	int	bg_w	=	al_get_bitmap_width(background);
	int	bg_h	=	al_get_bitmap_height(background);
	if (bg_w	!=	al_get_bitmap_width(foreground)	||
		bg_h	!=	al_get_bitmap_height(foreground))
		return	nullptr;

	ALLEGRO_BITMAP*	output	=	al_create_bitmap(bg_w, bg_h);

	unsigned char	bg_r	=	0;
	unsigned char	bg_g	=	0;
	unsigned char	bg_b	=	0;
	unsigned char	fg_r	=	0;
	unsigned char	fg_g	=	0;
	unsigned char	fg_b	=	0;
	unsigned char	alpha	=	0;
	unsigned char	results[3];
	float	alpha_val	=	0;

	ALLEGRO_COLOR	bg_pixel;
	ALLEGRO_COLOR	fg_pixel;
	ALLEGRO_COLOR	mask_pixel;

	al_lock_bitmap(background, al_get_bitmap_format(background), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(foreground, al_get_bitmap_format(foreground), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(mask, al_get_bitmap_format(mask), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int y = 0; y < (unsigned int) bg_h; ++y)
	{
		for (unsigned int x = 0; x < (unsigned int) bg_w; ++x)
		{
			bg_pixel	=	al_get_pixel(background, x, y);
			fg_pixel	=	al_get_pixel(foreground, x, y);
			mask_pixel	=	al_get_pixel(mask, x, y);
			al_unmap_rgb(	bg_pixel,
							&bg_r,
							&bg_g,
							&bg_b);
			al_unmap_rgb(	fg_pixel,
							&fg_r,
							&fg_g,
							&fg_b);
			al_unmap_rgb(	mask_pixel,
							&alpha,
							&alpha,
							&alpha
						);
			alpha_val	=	(float) alpha / 255;
			results[0]	=	std::min(std::max(int((bg_r * (1.0 - alpha_val))	+	(fg_r * alpha_val)), 0), 255);
			results[1]	=	std::min(std::max(int((bg_g * (1.0 - alpha_val))	+	(fg_g * alpha_val)), 0), 255);
			results[2]	=	std::min(std::max(int((bg_b * (1.0 - alpha_val))	+	(fg_b * alpha_val)), 0), 255);

			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(	results[0],
											results[1],
											results[2]));
		}
	}

	al_unlock_bitmap(output);
	al_unlock_bitmap(foreground);
	al_unlock_bitmap(background);
	al_unlock_bitmap(mask);
	return output;
}

// działa
ALLEGRO_BITMAP*
filters::alpha_blending(ALLEGRO_BITMAP* background, ALLEGRO_BITMAP* foreground, float alpha)
{
	int	bg_w	=	al_get_bitmap_width(background);
	int	bg_h	=	al_get_bitmap_height(background);
	if (bg_w	!=	al_get_bitmap_width(foreground)	||
		bg_h	!=	al_get_bitmap_height(foreground))
		return	nullptr;

	ALLEGRO_BITMAP*	output	=	al_create_bitmap(bg_w, bg_h);
	if (alpha	==	1.0)
	{
		output	=	foreground;
		return	output;
	}
	if (alpha	==	0.0)
	{
		output	=	background;
		return	output;
	}	

	unsigned char	bg_r	=	0;
	unsigned char	bg_g	=	0;
	unsigned char	bg_b	=	0;
	unsigned char	fg_r	=	0;
	unsigned char	fg_g	=	0;
	unsigned char	fg_b	=	0;
	unsigned char	results[3];

	ALLEGRO_COLOR	bg_pixel;
	ALLEGRO_COLOR	fg_pixel;

	al_lock_bitmap(background, al_get_bitmap_format(background), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(foreground, al_get_bitmap_format(foreground), ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);

	for (unsigned int y = 0; y < (unsigned int) bg_h; ++y)
	{
		for (unsigned int x = 0; x < (unsigned int) bg_w; ++x)
		{
			bg_pixel	=	al_get_pixel(background, x, y);
			fg_pixel	=	al_get_pixel(foreground, x, y);
			al_unmap_rgb(	bg_pixel,
							&bg_r,
							&bg_g,
							&bg_b);
			al_unmap_rgb(	fg_pixel,
							&fg_r,
							&fg_g,
							&fg_b);

			results[0]	=	std::min(std::max(int((bg_r * (1.0 - alpha))	+	(fg_r * alpha)), 0), 255);
			results[1]	=	std::min(std::max(int((bg_g * (1.0 - alpha))	+	(fg_g * alpha)), 0), 255);
			results[2]	=	std::min(std::max(int((bg_b * (1.0 - alpha))	+	(fg_b * alpha)), 0), 255);

			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(	results[0],
											results[1],
											results[2]));
		}
	}

	al_unlock_bitmap(output);
	al_unlock_bitmap(foreground);
	al_unlock_bitmap(background);
	return output;
}

//działa
ALLEGRO_BITMAP*
filters::detect_edges(ALLEGRO_BITMAP* source)
{
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	5;
	const	unsigned int	matrix_h	=	5;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	float				factor	=	1.0/1.0;
	int				filter_matrix[matrix_h][matrix_w]	=
	{ 
		{0,	0,	-1,	0,	0},
		{0, 0,	-1,	0,	0},
		{0, 0,	2,	0,	0},
		{0, 0,	0,	0,	0},
		{0, 0,	0,	0,	0}
	};

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - 1) % img_w, (img_h + y + i - 1) % img_h);
				}
			}
			
			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					al_unmap_rgb(	pixels[i][j],
									&tmp_channels[0][i][j],
									&tmp_channels[1][i][j],
									&tmp_channels[2][i][j]);
				}
			}

			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
					g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
					b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
				}
			}

			results[0]	=	std::min(std::max(int(factor * r), 0), 255);
			results[1]	=	std::min(std::max(int(factor * g), 0), 255);
			results[2]	=	std::min(std::max(int(factor * b), 0), 255);

			al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
			r			= 	g			=	b			=	0;
			results[0]	=	results[1]	=	results[2]	=	0;
		}
	}
	
	return output;
}

//działa
ALLEGRO_BITMAP*
filters::sharpen(ALLEGRO_BITMAP* source)
{
	unsigned int		img_w		=	al_get_bitmap_width(source);
	unsigned int		img_h		=	al_get_bitmap_height(source);
	const	unsigned int	matrix_w	=	3;
	const	unsigned int	matrix_h	=	3;
	int		r	=	0;
	int		g	=	0;
	int		b	=	0;
	ALLEGRO_COLOR		pixels[matrix_h][matrix_w];
	unsigned char		results[3];
	unsigned char		tmp_channels[3][matrix_h][matrix_w];
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	float				factor	=	1.0/1.0;
	int				filter_matrix[matrix_h][matrix_w]	=
	{ 
		{0, -1, 0},
		{-1, 5, -1},
		{0, -1, 0}
	};

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					pixels[i][j]	=	al_get_pixel(source, (img_w + x + j - 1) % img_w, (img_h + y + i - 1) % img_h);
				}
			}
			
			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					al_unmap_rgb(	pixels[i][j],
									&tmp_channels[0][i][j],
									&tmp_channels[1][i][j],
									&tmp_channels[2][i][j]);
				}
			}

			for (unsigned int i = 0; i < matrix_h; ++i)
			{
				for (unsigned int j = 0; j < matrix_w; ++j)
				{
					r	+=	round(tmp_channels[0][i][j]	*	filter_matrix[i][j]);
					g	+=	round(tmp_channels[1][i][j]	*	filter_matrix[i][j]);
					b	+=	round(tmp_channels[2][i][j]	*	filter_matrix[i][j]);
				}
			}

			results[0]	=	std::min(std::max(int(factor * r), 0), 255);
			results[1]	=	std::min(std::max(int(factor * g), 0), 255);
			results[2]	=	std::min(std::max(int(factor * b), 0), 255);

			al_put_pixel(x, y, al_map_rgb(results[0], results[1], results[2]));
			r			= 	g			=	b			=	0;
			results[0]	=	results[1]	=	results[2]	=	0;
		}
	}
	
	return output;
}

ALLEGRO_BITMAP*
filters::tint(ALLEGRO_BITMAP* source)
{
	unsigned int		img_w	=	al_get_bitmap_width(source);
	unsigned int		img_h	=	al_get_bitmap_height(source);
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	unsigned char		r	=	0;
	unsigned char		g	=	0;
	unsigned char		b	=	0;

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_set_target_bitmap(source);			
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&r,
							&g,
							&b
						);

			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(b, r, g));
		}
	}

	return output;
}

// działa
ALLEGRO_BITMAP*
filters::lighten(ALLEGRO_BITMAP* source, int n)
{
	if (!n)	return source;
	unsigned int					img_w	=	al_get_bitmap_width(source);
	unsigned int					img_h	=	al_get_bitmap_height(source);
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	unsigned char		r	=	0;
	unsigned char		g	=	0;
	unsigned char		b	=	0;

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_set_target_bitmap(source);			
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&r,
							&g,
							&b
						);
			if (n > 0)
			{
				r	=	(r + n) <= 255 ? (r + n) : 255;
				g	=	(g + n) <= 255 ? (g + n) : 255;
				b	=	(b + n) <= 255 ? (b + n) : 255;				
			}

			else
			{
				r	=	(r + n) >= 0 ? (r + n) : 0;
				g	=	(g + n) >= 0 ? (g + n) : 0;
				b	=	(b + n) >= 0 ? (b + n) : 0;				
			}


			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(r, g, b));
		}
	}

	return output;
}

// działa
ALLEGRO_BITMAP*
filters::contrast(ALLEGRO_BITMAP* source, float n)
{
	if (n == 1.0)	return source;
	unsigned int		img_w	=	al_get_bitmap_width(source);
	unsigned int		img_h	=	al_get_bitmap_height(source);
	ALLEGRO_BITMAP*		output	=	al_create_bitmap(img_w, img_h);
	al_set_target_bitmap(output);
	al_clear_to_color(al_map_rgb(0,0,0));
	unsigned char		r	=	0;
	unsigned char		g	=	0;
	unsigned char		b	=	0;

	for (unsigned int y = 0; y < img_h; ++y)
	{
		for (unsigned int x = 0; x < img_w; ++x)
		{
			al_set_target_bitmap(source);			
			al_unmap_rgb(	al_get_pixel(source, x, y),
							&r,
							&g,
							&b
						);
					
			r	=	(r * n) <= 255 ? (int) (r * n) : 255;
			g	=	(g * n) <= 255 ? (int) (g * n) : 255;
			b	=	(b * n) <= 255 ? (int) (b * n) : 255;

			al_set_target_bitmap(output);
			al_put_pixel(x, y, al_map_rgb(r, g, b));
		}
	}

	return output;
}

// działa
ALLEGRO_BITMAP*
filters::gradient(unsigned int width, unsigned int height, ALLEGRO_COLOR from, ALLEGRO_COLOR to)
{
	ALLEGRO_BITMAP*	output	=	al_create_bitmap(width, height);
	unsigned char	from_pxl[3];
	unsigned char	to_pxl[3];

	al_set_target_bitmap(output);

	al_unmap_rgb(	from,
					&from_pxl[0],
					&from_pxl[1],
					&from_pxl[2]
				);

	al_unmap_rgb(	to,
					&to_pxl[0],
					&to_pxl[1],
					&to_pxl[2]
				);

	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{

			al_put_pixel(x, y,
							al_map_rgb(	(int) lerp(from_pxl[0], to_pxl[0], x / (float) width),
										(int) lerp(from_pxl[1], to_pxl[1], x / (float) width),
										(int) lerp(from_pxl[2], to_pxl[2], x / (float) width)
										)
						);
		}
	}

	return output;
}

ALLEGRO_BITMAP*
filters::file_to_img(std::string filename, unsigned int width)
{
	std::ifstream	file(filename, std::ios::binary | std::ios::in);
	if (!file)	std::cout	<<	"error"	<<	std::endl;
	char	r;
	char	g;
	char	b;
	file.seekg(0, std::ios::end);
	unsigned int	size	=	file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned int	height	=	size / width / 3;
	ALLEGRO_BITMAP*	output	=	al_create_bitmap(width, height);
	al_lock_bitmap(output, al_get_bitmap_format(output), ALLEGRO_LOCK_WRITEONLY);
	al_set_target_bitmap(output);
	for (unsigned int i = 0; i	<	size; ++i)
	{
		file.read(&r, sizeof(r));
		file.read(&g, sizeof(g));
		file.read(&b, sizeof(b));
		al_put_pixel(i % width, i / width, al_map_rgb(r, g, b));
	}
	al_unlock_bitmap(output);
	return output;
}

void
fractals::draw_circle(int x, int y, float radius, ALLEGRO_BITMAP* background)
{
	al_draw_circle(x, y, radius, al_map_rgb(255, 255, 255), 1);

	if (radius > 8)
	{
		draw_circle(x + radius / 2, y, radius / 2, background);
    	draw_circle(x - radius / 2, y, radius / 2, background);
    	draw_circle(x, y + radius / 2, radius / 2, background);
    	draw_circle(x, y - radius / 2, radius / 2, background);
	}
}

void
fractals::draw_line(int x, int y, float length, ALLEGRO_BITMAP* background)
{
	if (length >= 1)
	{
		al_set_target_bitmap(background);
		al_draw_line(x, y, x + length, y, al_map_rgb(255, 255, 255), 1);
		y	+=	20;
		draw_line(x, y, length / 3, background);
		draw_line(x + length * 2.0 / 3.0, y, length / 3, background);		
	}
}
