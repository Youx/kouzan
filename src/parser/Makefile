# -----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <webmaster@flippeh.de> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Lukas Niederbremer.
# -----------------------------------------------------------------------------

CFLAGS=-ggdb -Wall -fPIC
RM=rm
SOURCES=main.c chunk.c utils.c

default: level_data_test chunk_test renderer renderer_benchmark renderer_vbo renderer_vbo_benchmark

renderer: libnbt.so chunk.h $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -L. -lGL -lGLU -lglut -lnbt -lz $(SOURCES)

renderer_vbo: libnbt.so chunk.h $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -L. -lGL -lGLU -lglut -lnbt -lz -D_USE_VBO $(SOURCES)

renderer_vbo_benchmark: libnbt.so chunk.h $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -L. -lGL -lGLU -lglut -lnbt -lz -DBENCHMARK -D_USE_VBO $(SOURCES)

renderer_benchmark: libnbt.so chunk.h $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -L. -lGL -lGLU -lglut -lnbt -lz -DBENCHMARK $(SOURCES)

level_data_test: libnbt.so level_data.c level_data.h
	$(CC) $(CFLAGS) -o $@ -L. -lnbt -lz -DLEVEL_DATA_TEST level_data.c

chunk_test: libnbt.so chunk.c chunk.h
	$(CC) $(CFLAGS) -o $@ -L. -lnbt -lz -DCHUNK_TEST chunk.c

#main: libnbt.so level_data.o
#	$(CC) -o main -L. -lnbt -lz level_data.o -g

libnbt.so: nbt.o endianness.o
	$(CC) $(CFLAGS) -fPIC -lz -shared -o libnbt.so nbt.o endianness.o

clean:
	$(RM) -rf *.{o,a,so}
	$(RM) -rf level_data_test
	$(RM) -rf chunk_test
	$(RM) -rf renderer renderer_vbo renderer_benchmark renderer_vbo_benchmark

nbt.o: nbt.h nbt.c
endianness.o: endianness.h endianness.c
level_data.o: level_data.c level_data.h
chunk.o: chunk.c chunk.h
