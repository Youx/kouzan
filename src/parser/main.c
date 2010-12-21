#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <pthread.h>

#include "chunk.h"


int angle_x = 0;
int angle_y = 0;
int angle_z = 0;
int distance = 60;

void draw();

#define MAX_X 9
#define MAX_Z 9

#define LIMIT_MIN_Y 0

chunk_t *ch[MAX_X][MAX_Z];
GLuint chunk_dl[MAX_X][MAX_Z];
void build_chunk_display_list(chunk_t *ch);
int need_redraw = 1;

void *sdl_th(void *data)
{
	SDL_Event event;
	int button = 0;
	for (;;)
	{
		SDL_WaitEvent(&event);

		switch(event.type)
		{
			case SDL_MOUSEBUTTONDOWN:
				//printf("button %i\n", event.button.button);
				switch (event.button.button) {
					case 1:
						button = 1;
						break;
					case 4: /* scroll up */
						distance -=1;
						need_redraw = 1;
						break;
					case 5: /* scroll down */
						distance +=1;
						need_redraw = 1;
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				button = 0;
				break;
			case SDL_MOUSEMOTION:
				if (button) {
					angle_x = (angle_x + event.motion.xrel)%360;
					angle_y = (angle_y + event.motion.yrel)%360;
					need_redraw = 1;
				}
				break;
			case SDL_QUIT:
				exit(0);
				break;
		}
	//	draw();

	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int x, z;

	//ch = chunk_parse("../../save/world/0/6/c.0.6.dat");
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			char chunk_name[256];
			snprintf(chunk_name, sizeof(chunk_name), "../../save/world/%i/%i/c.%i.%i.dat", x, z, x, z);
			printf("loading : %s\n", chunk_name);
			ch[x][z] = chunk_parse(chunk_name);
			ch[x][z]->pos.x = x;
			ch[x][z]->pos.z = z;
		}
	}

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	SDL_WM_SetCaption("SDL GL Application", NULL);
	SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(70,(double)640/480,1,1000);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING) ;                 // Active la gestion des lumières
	glEnable(GL_LIGHT0) ;                      // allume la lampe 0 

	GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	/* put the whole chunk in a render list */
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			chunk_dl[x][z] = glGenLists ((x+1)*(z+1)-1);
			glNewList(chunk_dl[x][z], GL_COMPILE);
			build_chunk_display_list(ch[x][z]);
			glEndList();
		}
	}
	pthread_t th;
	pthread_create(&th, NULL, &sdl_th, NULL);

#ifdef BENCHMARK
	int start_bench, end_bench;
	int frames = 0;
	start_bench = SDL_GetTicks();
	while (1) {
		end_bench = SDL_GetTicks();
		if (end_bench - start_bench > 1000) {
			printf("%f fps\n", frames * 1000.f / (double)(end_bench - start_bench));
			start_bench = end_bench;
			frames = 0;
		}
		draw();
		frames++;
	}
#else
	while (1) {
		if (need_redraw) {
			draw();
			need_redraw = 0;
		}
	}
#endif

	return 0;
}

typedef struct {
	char xplus;
	char xminus;
	char yplus;
	char yminus;
	char zplus;
	char zminus;
} neighbours_t;

int draw_cube(int x, int y, int z, char type, neighbours_t *nghb)
{
	switch(type) {
	case 1: /* stone */
		glColor3ub(128, 128, 128);
		break;
	case 7: /* bedrock */
		glColor3ub(56, 56, 56);
		break;
	case 3: /* dirt */
		glColor3ub(139, 69, 19);
		break;
	case 2: /* grass */
		glColor3ub(128, 255, 0);
		break;
	case 12: /* sand */
		glColor3ub(194, 178, 128);
		break;
	case 13: /* gravel */
		glColor3ub(110, 100, 100);
		break;
	case 9: /* water */
		glColor3ub(0, 0, 100);
		break;
	case 15: /* iron ore */
		glColor3ub(164, 164, 164);
		break;
	case 14: /* gold ore */
		glColor3ub(203, 161, 53);
		break;
	case 16: /* coal ore */
		glColor3ub(32, 32, 32);
		break;
	case 73: /* redstone ore */
		glColor3ub(128, 0, 0);
		break;
	case 49: /* obsidian */
		glColor3ub(0, 0, 0);
		break;
	case 11: /* lava */
		glColor3ub(255, 64, 0);
		break;
	case 48: /* mossy cobblestone */
		glColor3ub(0, 64, 0);
		break;
	case 4: /* cobblestone */
		glColor3ub(96, 96, 96);
		break;
	case 56: /* diamond */
		glColor3ub(0, 0, 200);
		break;
	default:
		printf("Unknown color for : %i\n", type);
		glColor3ub(200, 200, 200);
		break;
	}
	//printf("%i,%i,%i\n", x, y, z);

	glBegin(GL_QUADS);
	if (!nghb->xplus) {
		glNormal3f(0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glVertex3f(x, y, z+1.0f);	// Point 1 (Front)
		glVertex3f(x+1.0f, y, z+1.0f);	// Point 2 (Front)
		glVertex3f(x+1.0f, y+1.0f, z+1.0f);	// Point 3 (Front)
		glVertex3f(x, y+1.0f, z+1.0f);	// Point 4 (Front)
	}

	if (!nghb->xminus) {
		glNormal3f(0.0f, 0.0f, -1.0f);					// Normal Pointing Away From Viewer
		glVertex3f(x, y, z);	// Point 1 (Back)
		glVertex3f(x, y+1.0f, z);	// Point 2 (Back)
		glVertex3f(x+1.0f, y+1.0f, z);	// Point 3 (Back)
		glVertex3f(x+1.0f, y, z);	// Point 4 (Back)
	}

	if (!nghb->yplus) {
		glNormal3f(0.0f, 1.0f, 0.0f);					// Normal Pointing Up
		glVertex3f(x, y+1.0f, z);	// Point 1 (Top)
		glVertex3f(x, y+1.0f, z+1.0f);	// Point 2 (Top)
		glVertex3f(x+1.0f, y+1.0f, z+1.0f);	// Point 3 (Top)
		glVertex3f(x+1.0f, y+1.0f, z);	// Point 4 (Top)
	}

	if (!nghb->yminus) {
		glNormal3f(0.0f, -1.0f, 0.0f);					// Normal Pointing Down
		glVertex3f(x, y, z);	// Point 1 (Bottom)
		glVertex3f(x+1.0f, y, z);	// Point 2 (Bottom)
		glVertex3f(x+1.0f, y, z+1.0f);	// Point 3 (Bottom)
		glVertex3f(x, y, z+1.0f);	// Point 4 (Bottom)
	}

	if (!nghb->zplus) {
		glNormal3f(1.0f, 0.0f, 0.0f);					// Normal Pointing Right
		glVertex3f(x+1.0f, y, z);	// Point 1 (Right)
		glVertex3f(x+1.0f, y+1.0f, z);	// Point 2 (Right)
		glVertex3f(x+1.0f, y+1.0f, z+1.0f);	// Point 3 (Right)
		glVertex3f(x+1.0f, y, z+1.0f);	// Point 4 (Right)
	}

	if (!nghb->zminus) {
		glNormal3f(x, 0.0f, 0.0f);					// Normal Pointing Left
		glVertex3f(x, y, z);	// Point 1 (Left)
		glVertex3f(x, y, z+1.0f);	// Point 2 (Left)
		glVertex3f(x, y+1.0f, z+1.0f);	// Point 3 (Left)
		glVertex3f(x, y+1.0f, z);	// Point 4 (Left)
	}

	glEnd();
}

void build_chunk_display_list(chunk_t *ch)
{
	long i;
	int x, y, z;
	neighbours_t nghb = {0};

	i = 0;
	for (x = 0 ; x < 16 ; x++) {
		for (z = 0 ; z < 16 ; z++) {
			for (y = 0 ; y < 128 ; y++) {
				char type = ch->blocks[i];
				nghb.yplus = (i+1 < BPCHUNK) ? ch->blocks[i+1] : 0;
				nghb.yminus = (i-1 > LIMIT_MIN_Y) ? ch->blocks[i-1] : 0;
				nghb.zplus = (i+16 < BPCHUNK) ? ch->blocks[i+16] : 0;
				nghb.zminus = (i-16 > 0) ? ch->blocks[i-16] : 0;
				nghb.zplus = (i+(128*16) < BPCHUNK) ? ch->blocks[i+(128*16)] : 0;
				nghb.zminus = (i-(128*16) > 0) ? ch->blocks[i-(128*16)] : 0;
				//printf("computed id : %i VS expected : %i\n", y+(z*128)+(x*128*16), i);
				if (type == 0 || y < LIMIT_MIN_Y) {
					i++;
					//printf("%i,%i,%i(missed)\n", x, y, z);
					continue;
				}
				//printf("type: %i\n", type);
				draw_cube(x+ch->pos.x*16, y, z+ch->pos.z*16, type, &nghb);
				i++;
			}
		}
	}
}

void draw()
{

	int x, z;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	//gluLookAt(-1,64+distance,-1,8,64,8,0,1,0);
	gluLookAt(0,32+distance,0,8,32,8,0,1,0);
	glRotated(angle_x, 1, 0, 0);
	glRotated(angle_y, 0, 1, 0);
	//glRotated(angle_z, 0, 0, 1);
	//printf("=====================\n");
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			glCallList(chunk_dl[x][z]);
		}
	}
	glFlush();
	SDL_GL_SwapBuffers();
}
