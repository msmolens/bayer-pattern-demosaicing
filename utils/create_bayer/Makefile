
SRC = rgb_to_bayer.c
OBJ = $(SRC:.c=.o)
OUT = rgb_to_bayer

CC = gcc
CFLAGS = -Wall -O2 `Magick-config --cflags --cppflags`
LDFLAGS = `Magick-config --ldflags --libs`

RM = rm -f

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

clean:
	-$(RM) $(OUT) $(OBJ) core
