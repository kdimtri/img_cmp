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



int main(int argc, const char * argv[]) {
	std::string 			filenames[ARGS]	= {};
	int						result[6]		= {0};
	int 					x = 0;
	int 					x2 = 0;
	int						y = 0;
	int						y2 = 0;
	int						xy_max = 0;
	int 					comp_ff = 0;
	int 					comp_r = CS_CHANELS;
	unsigned char 			*data_ptr = NULL;
	unsigned char 			*data_ptr2 = NULL;

	std::clog<<__TIME__<<std::endl;

	if ( --argc == ARGS )
		while(argc && argv[argc--])
			filenames[argc] = argv[argc + 1];
	else
	{
		std::cerr<<"Wrong number of arguments: "<<(argc - 1)<<std::endl;
		int i(0);
		while (i < ARGS)
		{
			std::cout<<"Enter the path to png file "<< (i+1) <<std::endl;
			std::cin>>filenames[i++];
		}
	}
	for (std::string filename:filenames)
	{
		std::ifstream file (filename, std::ios::in);
		if (file.is_open())
			file.close();
		else
		{
			std::cerr<<"Unable to open file: \""<<filename<<"\""<<std::endl;
			return (-1);
		}
		data_ptr = stbi_load(filename.c_str(), &x, &y, &comp_ff, comp_r);
		if (data_ptr != NULL)
		{
			std::cout<<"Image size is :"<<x<<"x"<<y<<std::endl;
			std::cout<<"its ben decoded by "<<comp_r<<" colors"<<std::endl;
			xy_max=std::max(x,y);
			if (xy_max>IMG_MAX_XY_SIZE)
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
				std::cout<<"image need to scale up to :"<< x2 <<"x"<<y2<<std::endl;
				data_ptr2 = new unsigned char [x2*y2*comp_r];
				if (data_ptr2 != NULL)
				{
					if (stbir_resize_uint8(data_ptr,x,y,0,data_ptr2,x2,y2,0,comp_r))
					{
						std::cout<<"new image size is :"<< x2 <<"x"<<y2<<std::endl;
						stbi_image_free(data_ptr);
						
					}
					else
					{
						std::cerr<<"fail to resize image file: \""<<filename<<"\""<<std::endl;
						return (-2);
					}
				}
				else
				{
					std::cerr<<"fail to allocate memory to scale image"<<std::endl;
					return (-2);
				}
			}
		}
		else
		{
			std::cerr<<"Unable to decode file: \""<<filename<<"\""<<std::endl;
			std::cerr<<"stbi error : "<< stbi_failure_reason() << std::endl;
		}
	}
	std::cout<<"Result:";
	for (int r : result)
		std::cout<<'['<<r<<']';
	std::cout<<std::endl;
	std::clog<<__TIME__<<std::endl;
	return(0);
}
