SHELL="/bin/sh"
CC="clang++"
STDCPP="-std=c++11"
STDLIB="-stdlib=libc++"
SOURCE="img_cmp/main.cpp"
TARGET="imgcmp"
STBI="stb/stb_image.h" "stb/stb_image_resize.h"
RUN1="Assignment/frame1.png" "Assignment/frame2.png" 
RUN2="Assignment/frame1.png" "Assignment/frame3.png"

all:
	$(CC) $(STDCPP) $(STDLIB) $(SOURCE) -o $(TARGET)
clean:
	rm -f $(TARGET)
run:
	make clean
	make
	./$(TARGET) $(RUN1) && ./$(TARGET) $(RUN2)
	
