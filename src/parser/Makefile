# -----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <webmaster@flippeh.de> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Lukas Niederbremer.
# -----------------------------------------------------------------------------

CFLAGS=-g -Wall -fPIC
RM=rm

default: level_data_test chunk_test renderer

renderer: libnbt.so main.c chunk.c chunk.h
	gcc -o $@ -L. -lGL -lGLU -lSDL -lnbt -lz chunk.c chunk.h main.c -g

level_data_test: libnbt.so level_data.c level_data.h
	gcc -o $@ -L. -lnbt -lz -DLEVEL_DATA_TEST level_data.c -g

chunk_test: libnbt.so chunk.c chunk.h
	gcc -o $@ -L. -lnbt -lz -DCHUNK_TEST chunk.c -g

#main: libnbt.so level_data.o
#	gcc -o main -L. -lnbt -lz level_data.o -g

libnbt.so: nbt.o endianness.o
	gcc -fPIC -lz -shared -o libnbt.so nbt.o endianness.o

clean:
	$(RM) -rf *.{o,a,so}
	$(RM) -rf level_data_test
	$(RM) -rf chunk_test
	$(RM) -rf renderer

nbt.o: nbt.h nbt.c
endianness.o: endianness.h endianness.c
level_data.o: level_data.c level_data.h
chunk.o: chunk.c chunk.h
