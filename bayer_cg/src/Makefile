MAIN=Bayer
CC=g++
CFLAGS= -g -O2 -Wall `Magick++-config --cppflags` #-DSAVEFRAMES #-DOLD  #-DBAYER
INCLUDES = -I$(HOME)/stc/max/glew/include -I.
LFLAGS = -L$(HOME)/stc/max/glew/lib -L/usr/X11R6/lib `Magick++-config --ldflags --libs` -lCgGL -lCg -lGL -lGLU -lGLEW -lglut -lpthread -lXmu -lXi
SRCS = $(MAIN).cpp Image.cpp BayerRenderer.cpp RenderTexture.cpp FPSCounter.cpp GLUTFPSCounter.cpp trackball.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(MAIN)

$(MAIN):  $(OBJS) $(OBJS_MAIN)
	$(CC) $(OBJS) $(OBJS_MAIN) -o $(MAIN) $(LFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)

outmovie:
	/bin/sh movie.sh

