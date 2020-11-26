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


static	int		validate_file(	std::string		*filename);
static	int		load_image(		std::string		*filename,
								unsigned char	*rdata,
								size_t			*rdata_size);


int main(int argc, const char *argv[]) {
	std::string 			filenames[ARGS]	= {};
	int						result[6]		= {0};
	unsigned char 			*img_data	= NULL;
	size_t					img_data_size = 0;

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
		img_data_size = IMG_MAX_XY_SIZE*IMG_MAX_XY_SIZE*CS_CHANELS;
		img_data = new unsigned char [img_data_size];
		if(img_data == NULL)
		{
			std::cerr<<"fail to allocate "<<img_data_size
			<<" byte of space for image data"<<std::endl;
			return (2);
		}
		if(load_image(&filenames[fi++], img_data, &img_data_size))
		{
			delete [] img_data;
			std::cerr<<"Fail to load image: \""
				<<filenames[--fi]<<"\""<<std::endl;
			return (3);
		}
		delete [] img_data;
	}
	std::cout<<"Result:";
	for (int r : result)
		std::cout<<'['<<r<<']';
	std::cout<<std::endl;
	return(0);
}

static	int		validate_file(std::string *filename)
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

static	int		load_image(	std::string		*filename,
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
	data_ptr = stbi_load(filename->c_str(), &x, &y, &comp_f, comp_r);
	if (data_ptr == NULL)
	{
		std::cerr<<"Unable to decode file: \""<<*filename<<"\""<<std::endl;
		std::cerr<<"stbi error : "<< stbi_failure_reason() << std::endl;
		return (1);
	}
	std::cout<<"image dimensions are :"<<x<<"x"<<y<<std::endl;
	std::cout<<"its ben decoded by "<<comp_r<<" colors"<<std::endl;
	data_size=x*y*comp_r;
	std::cout<<"image data has : "<<data_size<<" bytes of memory"<<std::endl;
	xy_max=std::max(x,y);
	if (xy_max<=IMG_MAX_XY_SIZE)
	{
		if (data_size <= *rdata_size)
		{
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
