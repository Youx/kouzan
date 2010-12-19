#ifndef __LEVEL_DATA_H__
#define __LEVEL_DATA_H__

typedef struct {
	long tod;	/* time of day */
	long last_played;
	/* player inventory */
	struct {
		int x;
		int y;
		int z;
	} spawn;
	long size;	/* size on disk */
	long seed;	/* random seed */
} level_data_t;

void level_data_print(level_data_t *lvl);
level_data_t *level_data_parse(char *f);

#endif
