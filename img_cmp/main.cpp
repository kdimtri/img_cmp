#include <iostream>
#include <fstream>

#define  STB_IMAGE_IMPLEMENTATION
#include  "../stb/stb_image.h"
#define  STB_IMAGE_RESIZE_IMPLEMENTATION
#include  "../stb/stb_image_resize.h"

#define  IMG_MAX_XY_SIZE  256    //256x256 px
#define  CS_CHANELS    3    //RGB (no alpha)
#define  HISTS_POINTS  30    //hist resolution
#define  ARGS      2    //sholud compare 2
#define  STBI_ONLY_PNG      //    png files
//#define  TO_LOG

int    validate_file(  std::string *filename);
int    load_image(    std::string *filename,
            unsigned char *rdata,
            size_t      *rdata_size);
int    rgb_to_hsl(    int r,int g, int b, float *h, float *s, float *l);

//MARK: main -
int main(int argc, const char *argv[]) {
  std::string        filenames[ARGS]        = {"empty"};
  int            hist[ARGS][6][HISTS_POINTS]  = {0};
  float          result[6]          = {0};

#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
  std::clog<<"argc: "<<argc<<std::endl;
  int argci = argc;
  while (argci--)
    std::clog<<"argv["<<argci<<"]: "<<argv[argci]<<std::endl;
#endif

  if ( --argc == ARGS )
  {
    while(argc && argv[argc--])
      filenames[argc] = argv[argc + 1];
  }
  else
  {
    if(argc)
    {
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
      std::cerr<<"Wrong number of arguments: "<<(argc)<<std::endl;
    }
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
    size_t        img_data_size = IMG_MAX_XY_SIZE*IMG_MAX_XY_SIZE*CS_CHANELS;
    unsigned char    *img_data = new unsigned char [img_data_size];

    if(img_data == NULL)
    {
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
      std::cerr<<"fail to allocate "<<img_data_size
      <<" byte of space for image data"<<std::endl;
      return (2);
    }
    if(load_image(&filenames[fi], img_data, &img_data_size))
    {
      delete [] img_data;
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
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

      //MARK:- hist intervals -
      int hi[6] = {  r/(256/HISTS_POINTS),
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

  int ci = 0;
  int hi = 0;
  while(--fi)
  {
    ci = 0;
    while(ci < 6)
    {
      hi = 0;
      while(hi < HISTS_POINTS)
      {
        result[ci]+= pow(hist[fi][ci][hi]-hist[fi-1][ci][hi],2);
        hi++;
      }
      result[ci] = sqrt(result[ci]);
      ci++;
    }
  }
  for (float res : result)
    std::cout<<'['<<res<<']';
  std::cout<<std::endl;
#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
#endif
  return(0);
}

int    validate_file(std::string *filename)
{
  std::ifstream file (*filename, std::ios::in);

#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
  std::clog<<"checking image file: \""<<*filename<<"\""<<std::endl;
#endif
  if (!file.is_open())
  {
    std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
    std::cerr<<"Unable to open file: \""<<*filename<<"\""<<std::endl;
    return (1);
  }
#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
  std::clog<<"image file: \""<<*filename<<"\" could be read."<<std::endl;
#endif

  file.close();
  return (0);
}

int    load_image(  std::string    *filename,
              unsigned char  *rdata,
              size_t      *rdata_size)
{
    int        x      = 0;
    int        x2      = 0;
    int        y      = 0;
    int        y2      = 0;
    int        xy_max    = 0;
    int        comp_f    = 0;
    int        comp_r    = CS_CHANELS;
    unsigned char  *data_ptr  = NULL;
    unsigned char  *data_ptr2  = NULL;
    size_t      data_size  = 0;

//MARK:- load
#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
  std::clog<<"loading image file: \""<<*filename<<"\""<<std::endl;
#endif
  data_ptr = stbi_load(filename->c_str(), &x, &y, &comp_f, comp_r);
  if (data_ptr == NULL)
  {
    std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
    std::cerr<<"Unable to decode file: \""<<*filename<<"\""<<std::endl;
    std::cerr<<"stbi error : "<< stbi_failure_reason() << std::endl;
    return (1);
  }
  data_size=x*y*comp_r;
  xy_max=std::max(x,y);
#ifdef TO_LOG
  std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
  std::clog<<"image loaded! dimensions are :"<<x<<"x"<<y<<std::endl;
  std::clog<<"its ben decoded by "<<comp_r<<" colors"<<std::endl;
  std::clog<<"image data has : "<<data_size<<" bytes of memory"<<std::endl;
#endif
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
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
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
#ifdef TO_LOG
    std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
    std::clog<<"image need to downscale to :"<< x2 <<"x"<<y2<<std::endl;
#endif
    data_size=x2*y2*comp_r;
    data_ptr2 = new unsigned char [data_size];
    if (data_ptr2 == NULL)
    {
      stbi_image_free(data_ptr);
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
      std::cerr<<"fail to allocate memory to scale image"<<std::endl;
      return (2);
    }
    if (stbir_resize_uint8(data_ptr,x,y,0,data_ptr2,x2,y2,0,comp_r) == 0)
    {
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
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
#ifdef TO_LOG
        std::clog<<__TIME__<<'_'<<__func__<<':'<<std::endl;
        std::clog<<"new image size is :"<< x2 <<"x"<<y2<<std::endl;
        std::clog<<"new image data has : "<<data_size<<" bytes of memory"<<std::endl;
#endif
        stbi_image_free(data_ptr);
        delete [] data_ptr2;
        return (0);
      }
      else
      {
        std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
        std::cerr<<"new image data gets more space than expected"<<std::endl;
        stbi_image_free(data_ptr);
        delete [] data_ptr2;
        return (5);
      }
    }
    else
    {
      std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
      std::cerr<<"unexpected return from stbir_resize_uint8"<<std::endl;
      stbi_image_free(data_ptr);
      delete [] data_ptr2;
      return (6);
    }
  }
  //MARK: -
}

int    rgb_to_hsl(int r,int g, int b, float *h, float *s, float *l)
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
    std::cerr<<__TIME__<<'_'<<__func__<<':'<<std::endl;
    std::cerr<<"corrupted data:\nr:\t"<<r<<"\tg:\t"<<g<<"\tb:\t"<<b<<std::endl;
    return (1);
  }
  float  r_f      = r / 255.0;
  float  g_f      = g / 255.0;
  float  b_f      = b / 255.0;
  float  rgb_min    = std::min(std::min(r,g),b);
  float  rgb_max    = std::max(std::max(r,g),b);
  float  rgb_min_f  = rgb_min / 255.0;
  float  rgb_max_f  = rgb_max / 255.0;
  float  maxSmin    = rgb_max_f - rgb_min_f;
  float  h_m1      = (60 * (maxSmin != 0))/(maxSmin + (maxSmin == 0));
  float  h_m2    = (g_f-b_f)*(r == rgb_max)\
            + (b_f-r_f)*(g == rgb_max) \
            + (r_f-g_f)*(r != rgb_max)*(g != rgb_max);
  float  h_m3    = 360 *(r == rgb_max) + 120 *(g == rgb_max) + 240 *(r != rgb_max)*(g != rgb_max);
  float  h_m4    = (r == rgb_max)*360*(((h_m1 * h_m2) + h_m3) >= 360);
  float  s_m1    = .5 * (rgb_max_f + rgb_min_f);
  float  s_m2    = (s_m1 > .5) *  2.0;
  float  s_m3    = (-1)*(s_m1 > .5) * (2.0 * s_m1) + (s_m1 <= .5) * (2.0 * s_m1);

  *h    = ((h_m1 * h_m2) + h_m3) - h_m4;
  *s    = (maxSmin/(s_m2 + s_m3)) * 100;
  *l    = ((rgb_max_f + rgb_min_f)/2) * 100;
  return (0);
}
