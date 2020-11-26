#include <iostream>
#include <fstream>

#define 	STB_IMAGE_IMPLEMENTATION
#include 	"../stb/stb_image.h"
#define 	STB_IMAGE_RESIZE_IMPLEMENTATION
#include 	"../stb/stb_image_resize.h"


#define 	IMG_MAX_XY_SIZE 256		//256x256
#define 	CS_CHANELS	3		//RGB
#define 	HISTS_POINTS 30			//30 intervals for hist resolution
#define 	ARGS 2				//sholud compare 2 files
								//#define 	STBIR_MAX_CHANNELS 3
#define 	STBI_ONLY_PNG
#define 	STBIR_ALPHA_CHANNEL_NONE -1

int		validate_file(	std::string		*filename);
int		load_image(		std::string		*filename,
								unsigned char	*rdata,
								size_t			*rdata_size);
int		rgb_to_hsl(		int r,int g, int b, float *h, float *s, float *l);

//MARK: main -
int main(int argc, const char *argv[]) {
	std::string 			filenames[ARGS]				= {"empty"};
	int 					hist[ARGS][6][HISTS_POINTS]	= {0};
	int						result[6]					= {0};



	if ( --argc == ARGS )
	{
		while(argc && argv[argc--])
			filenames[argc] = argv[argc + 1];
	}
	else
	{
		if(argc)
			std::cerr<<"Wrong number of arguments: "<<(argc)<<std::endl;
		int fi = 0;
		while (fi < ARGS)
		{
			std::cout<<"Enter the path to png file "<< (fi+1) <<std::endl;
			std::cin>>filenames[fi];
			if(validate_file(&filenames[fi++]))
				return (1);
		}
	}
	int fi = 0;
	while (fi < ARGS)
	{
	size_t 			img_data_size = IMG_MAX_XY_SIZE*IMG_MAX_XY_SIZE*CS_CHANELS;
	unsigned char 	*img_data = new unsigned char [img_data_size];
		if(img_data == NULL)
		{
			std::cerr<<"fail to allocate "<<img_data_size
			<<" byte of space for image data"<<std::endl;
			return (2);
		}
		if(load_image(&filenames[fi], img_data, &img_data_size))
		{
			delete [] img_data;
			std::cerr<<"Fail to load image: \""
				<<filenames[--fi]<<"\""<<std::endl;
			return (3);
		}
		//MARK:- iterating through pixel data
		size_t pix_count = img_data_size/CS_CHANELS;
		unsigned char *img_data_start = img_data;
		while (pix_count--)
		{
			int r = (int ) *img_data++;
			int g = (int ) *img_data++;
			int b = (int ) *img_data++;
			float h = .0f;
			float s = .0f;
			float l = .0f;
			if(rgb_to_hsl(r,g,b,&h,&s,&l))
				return (4);

			int hi[6] = {	r/(256/HISTS_POINTS),
							g/(256/HISTS_POINTS),
							b/(256/HISTS_POINTS),
							(int)(h/(360/HISTS_POINTS)),
							(int)(s/(100/HISTS_POINTS)),
							(int)(l/(100/HISTS_POINTS))};
			hist[fi][0][hi[0]]++;
			hist[fi][1][hi[1]]++;
			hist[fi][2][hi[2]]++;
			hist[fi][3][hi[3]]++;
			hist[fi][4][hi[4]]++;
			hist[fi][5][hi[5]]++;
		}
		//MARK: -
		delete [] img_data_start;
		fi++;
	}

	fi = 0;
	int ci = 0;
	while (fi < ARGS)
	{
		std::cout<<fi<<std::endl;
		ci = 0;
		while (ci < 6)
		{
		std::cout<<"["<<ci<<"]: ";
			for (int histint:hist[fi][ci])
				std::cout<<"\t\t"<<histint<<"\t\t";
			std::cout<<std::endl;
			ci++;
		}
		fi++;
	}
	std::cout<<"Result:";
	for (int r : result)
		std::cout<<'['<<r<<']';
	std::cout<<std::endl;
	return(0);
}

int		validate_file(std::string *filename)
{
	std::ifstream file (*filename, std::ios::in);

	if (!file.is_open())
	{
		std::cerr<<"Unable to open file: \""<<*filename<<"\""<<std::endl;
		return (1);
	}
	file.close();
	return (0);
}

int		load_image(	std::string		*filename,
							unsigned char	*rdata,
							size_t			*rdata_size)
{
		int				x			= 0;
		int				x2			= 0;
		int				y			= 0;
		int				y2			= 0;
		int				xy_max		= 0;
		int				comp_f		= 0;
		int				comp_r		= CS_CHANELS;
		unsigned char	*data_ptr	= NULL;
		unsigned char	*data_ptr2	= NULL;
		size_t			data_size	= 0;

//MARK:- load
	std::cout<<"loading image file: \""<<*filename<<"\""<<std::endl;
	data_ptr = stbi_load(filename->c_str(), &x, &y, &comp_f, comp_r);
	if (data_ptr == NULL)
	{
		std::cerr<<"Unable to decode file: \""<<*filename<<"\""<<std::endl;
		std::cerr<<"stbi error : "<< stbi_failure_reason() << std::endl;
		return (1);
	}
	std::cout<<"image loaded! dimensions are :"<<x<<"x"<<y<<std::endl;
	std::cout<<"its ben decoded by "<<comp_r<<" colors"<<std::endl;
	data_size=x*y*comp_r;
	std::cout<<"image data has : "<<data_size<<" bytes of memory"<<std::endl;
	xy_max=std::max(x,y);
	if (xy_max<=IMG_MAX_XY_SIZE)
	{
		if (data_size <= *rdata_size)
		{
			*rdata_size = data_size;
			std::memcpy(rdata, data_ptr, data_size);
			stbi_image_free(data_ptr);
			return (0);
		}
		else
		{
			std::cerr<<"image data gets more space than expected"<<std::endl;
			stbi_image_free(data_ptr);
			return (4);
		}
	}
	else //MARK:- resize
	{
		if(xy_max==x)
		{
			x2=IMG_MAX_XY_SIZE;
			y2=y*x2/x;
		}
		else
		{
			y2=IMG_MAX_XY_SIZE;
			x2=x*y2/y;
		}
		std::cout<<"image need to downscale to :"<< x2 <<"x"<<y2<<std::endl;
		data_size=x2*y2*comp_r;
		data_ptr2 = new unsigned char [data_size];
		if (data_ptr2 == NULL)
		{
			stbi_image_free(data_ptr);
			std::cerr<<"fail to allocate memory to scale image"<<std::endl;
			return (2);
		}
		if (stbir_resize_uint8(data_ptr,x,y,0,data_ptr2,x2,y2,0,comp_r) == 0)
		{
			std::cerr<<"fail to resize image file: \""<<*filename<<"\""<<std::endl;
			stbi_image_free(data_ptr);
			delete [] data_ptr2;
			return (3);
		}
		else if (stbir_resize_uint8(data_ptr,x,y,0,data_ptr2,x2,y2,0,comp_r) == 1)
		{
			if (data_size <= *rdata_size)
			{
				*rdata_size = data_size;
				std::memcpy(rdata, data_ptr2, data_size);
				std::cout<<"new image size is :"<< x2 <<"x"<<y2<<std::endl;
				std::cout<<"new image data has : "<<data_size<<" bytes of memory"<<std::endl;
				stbi_image_free(data_ptr);
				delete [] data_ptr2;
				return (0);
			}
			else
			{
				std::cerr<<"new image data gets more space than expected"<<std::endl;
				stbi_image_free(data_ptr);
				delete [] data_ptr2;
				return (5);
			}
		}
		else
		{
			std::cerr<<"unexpected return from stbir_resize_uint8"<<std::endl;
			stbi_image_free(data_ptr);
			delete [] data_ptr2;
			return (6);
		}

	}
	//MARK: -
}

int		rgb_to_hsl(int r,int g, int b, float *h, float *s, float *l)
{
	if((r == 0) & (g == 0) & (b == 0))
	{
		*h = .0;
		*s = .0;
		*l = .0;
		return (0);
	}
	if ((r < 0)|(r > 255)|(g < 0)|(g > 255)|(b < 0)|(b > 255))
	{
		std::cerr<<"corrupted data:\nr:\t"<<r<<"\tg:\t"<<g<<"\tb:\t"<<b<<std::endl;
		return (1);
	}
	float	r_f			= r / 255.0;
	float	g_f			= g / 255.0;
	float	b_f			= b / 255.0;
	float	rgb_min 	= std::min(std::min(r,g),b);
	float 	rgb_max 	= std::max(std::max(r,g),b);
	float	rgb_min_f	= rgb_min / 255.0;
	float	rgb_max_f	= rgb_max / 255.0;
	float	maxSmin		= rgb_max_f - rgb_min_f;
	float 	h_m1	 	= (60 * (maxSmin != 0))/(maxSmin + (maxSmin == 0));
	float 	h_m2		= (g_f-b_f)*(r == rgb_max) + (b_f-r_f)*(g == rgb_max) + (r_f-g_f)*(r != rgb_max)*(g != rgb_max);
	float 	h_m3		= 360 *(r == rgb_max) + 120 *(g == rgb_max) + 240 *(r != rgb_max)*(g != rgb_max);
	float	h_m4		= (r == rgb_max)*360*(((h_m1 * h_m2) + h_m3) >= 360);
	float	s_m1		= .5 * (rgb_max_f + rgb_min_f);
	float	s_m2		= (s_m1 > .5) *  2.0;
	float	s_m3		= (-1)*(s_m1 > .5) * (2.0 * s_m1) + (s_m1 <= .5) * (2.0 * s_m1);

	*h 		= ((h_m1 * h_m2) + h_m3) - h_m4;
	*s 		= (maxSmin/(s_m2 + s_m3)) * 100;
	*l 		= ((rgb_max_f + rgb_min_f)/2) * 100;
	return (0);
}
