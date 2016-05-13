CC=i686-w64-mingw32-g++ 
CFLAGS=-static-libstdc++ -static-libgcc -static
INC={include-win32-dw2,glm}
LIB=lib-win32-sjlj
LINK=-lsfml-{window,graphics,system}-s -lglew32 -lopengl32 -lwinmm -lgdi32 -lfreetype -ljpeg

orbital: orbital_challenge.cc
	$(CC) $(CFLAGS) -DGLEW_STATIC -DSFML_STATIC -I$(INC) -L$(LIB) orbital_challenge.cc $(LINK)
