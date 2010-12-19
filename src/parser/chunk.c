#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "chunk.h"
#include "nbt.h"

#define MIN(a, b) \
	(a < b ? a : b)

chunk_t *chunk_parse(char *f)
{
	chunk_t *ch = calloc(1, sizeof(chunk_t));
	nbt_file *nbt;
	nbt_tag *level;
	nbt_byte_array *ar;

	nbt_tag *blocks;

	if (nbt_init(&nbt) != NBT_OK) {
		fprintf(stderr, "Error initializing nbt_file\n");
		return NULL;
	}
	if (nbt_parse(nbt, f) != NBT_OK) {
		fprintf(stderr, "Error parsing chunk\n");
		return NULL;
	}
	level = nbt_find_tag_by_name("Level", nbt->root);
	blocks = nbt_find_tag_by_name("Blocks", level);
	ar = blocks->value;
	memcpy(ch->blocks, ar->content, MIN(ar->length, 32768));

}

void chunk_print(chunk_t *ch)
{
}

#ifdef CHUNK_TEST

int main()
{
	chunk_t *ch;

	ch = chunk_parse("../../save/world/0/6/c.0.6.dat");
	chunk_print(ch);
}

#endif
