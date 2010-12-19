#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "nbt.h"
#include "level_data.h"

void level_data_print(level_data_t *lvl)
{
	long real_t = (((lvl->tod + 6000) % 24000) * 3.6);
	long lp = lvl->last_played/1000;
	printf("Time of Day 	: %02ld:%02ld:%02ld\n", real_t/3600, (real_t/60%60), real_t%60);
	printf("Last played 	: %s\n", ctime(&lp));
	printf("Spawn		: %i;%i;%i\n", lvl->spawn.x,
			lvl->spawn.y, lvl->spawn.z);
	printf("Size of world	: %likB\n", lvl->size/1024);
	printf("Seed		: %li\n", lvl->seed);
}

level_data_t *level_data_parse(char *f)
{
	level_data_t *lvl = calloc(1, sizeof(level_data_t));
	nbt_file *nbt;
	nbt_tag *data;
	
	nbt_tag *tod;
	nbt_tag *last_played;
	nbt_tag *spawn_x;
	nbt_tag *spawn_y;
	nbt_tag *spawn_z;
	nbt_tag *size;
	nbt_tag *seed;

	if (nbt_init(&nbt) != NBT_OK) {
		fprintf(stderr, "Error initializing nbt_file\n");
		return NULL;
	}
	if (nbt_parse(nbt, f) != NBT_OK) {
		fprintf(stderr, "Error parsing level_data\n");
		return NULL;
	}
	/* TODO: security check on validity of format */

	data = nbt_find_tag_by_name("Data", nbt->root);
	/* parse time of day */
	tod = nbt_find_tag_by_name("Time", data);
	lvl->tod = *(long*)tod->value;
	/* parse last played */
	last_played = nbt_find_tag_by_name("LastPlayed", data);
	lvl->last_played = *(long*)last_played->value;
	/* parse spawnX */
	spawn_x = nbt_find_tag_by_name("SpawnX", data);
	lvl->spawn.x = *(int*)spawn_x->value;
	/* parse spawnY */
	spawn_y = nbt_find_tag_by_name("SpawnY", data);
	lvl->spawn.y = *(int*)spawn_y->value;
	/* parse spawnZ */
	spawn_z = nbt_find_tag_by_name("SpawnZ", data);
	lvl->spawn.z = *(int*)spawn_z->value;
	/* parse size on disk */
	size = nbt_find_tag_by_name("SizeOnDisk", data);
	lvl->size = *(long*)size->value;
	/* parse seed */
	seed = nbt_find_tag_by_name("RandomSeed", data);
	lvl->seed = *(long*)seed->value;

	return lvl;
}

#ifdef LEVEL_DATA_TEST

int main()
{
	level_data_t *lvl;
	lvl = level_data_parse("../../save/world/level.dat");
	level_data_print(lvl);
}

#endif
