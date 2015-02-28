MAIN=bayer_viewer
MAIN_CPU=bayer_viewer_cpu
CC=g++
CFLAGS= -O2 -Wall `Magick-config --cflags --cppflags`
INCLUDES =  -I. -I../../glew/include
LFLAGS= -L../../glew/lib `Magick-config --ldflags --libs` -lglut -lGLU -lGL -lGLEW -lXi -lXmu
DOXYGEN=doxygen
SRCS = FPSCounter.cpp GLUTFPSCounter.cpp
SRCS_MAIN = test_bayer_renderer.cpp BayerRenderer.cpp RenderTexture.cpp
SRCS_MAIN_CPU = test_bayer_renderer_cpu.cpp BayerRendererCPU.cpp bayer.cpp
OBJS = $(SRCS:.cpp=.o)
OBJS_MAIN = $(SRCS_MAIN:.cpp=.o)
OBJS_MAIN_CPU = $(SRCS_MAIN_CPU:.cpp=.o)

all: $(MAIN) $(MAIN_CPU)

$(MAIN):  $(OBJS) $(OBJS_MAIN)
	$(CC) $(OBJS) $(OBJS_MAIN) -o $(MAIN) $(LFLAGS)

$(MAIN_CPU): $(OBJS) $(OBJS_MAIN_CPU)
	$(CC) $(OBJS) $(OBJS_MAIN_CPU) -o $(MAIN_CPU) $(LFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

doc:
	$(DOXYGEN)

clean:
	rm -f *.o *~ $(MAIN) $(MAIN_CPU)
