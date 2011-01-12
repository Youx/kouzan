#ifndef __CHUNK_H__
#define __CHUNK_H__

#define BLOCKID(blocks, x, y, z) \
	(blocks[y+(z*128)+(x*128*16)])

#define HALFID(array, x, y, z) \
	(array[ ((y+(z*128)+(x*128*16))/2) ] >> (((y+(z*128)+(x*128*16))%2) * 4))

#define BPCHUNK 32768

typedef struct {
	char blocks[BPCHUNK];
	char data[16384];
	char sky_light[16384];
	char block_light[16384];
	char heightmap[16384];
	struct {
		int x;
		int z;
	} pos;
	char populated : 1;
} chunk_t;

chunk_t *chunk_parse(char *f);
void chunk_print(chunk_t *ch);

#endif
