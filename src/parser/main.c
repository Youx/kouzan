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
void build_vertex_arrays(chunk_t *ch);
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

GLfloat vertices[8*3*16*16*128]; /* 786k floats */
GLfloat normals[8*3*16*16*128]; /* 786k floats */
GLubyte colors[8*3*16*16*128]; /* 786k floats */
GLuint indices[6*4*16*16*128]; /* 786k longs */
long idx_idx;

void print_vertices(GLfloat *vertices)
{
	int i;
	for (i=0 ; i < 16 ; i++) {
		printf("vert: {%f, %f, %f}\n", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
	}
}
void print_indices(GLuint *indices)
{
	int i;
	for (i=0 ; i < 16 ; i++) {
		printf("indices: {%i, %i, %i, %i}\n", indices[i*4], indices[i*4+1], indices[i*4+2],indices[i*4+3]);
	}
}

int main(int argc, char *argv[])
{

	ch[0][0] = chunk_parse("../../save/world/0/0/c.0.0.dat");
	/*
	int x, z;
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			char chunk_name[256];
			snprintf(chunk_name, sizeof(chunk_name), "../../save/world/%i/%i/c.%i.%i.dat", x, z, x, z);
			printf("loading : %s\n", chunk_name);
			ch[x][z] = chunk_parse(chunk_name);
			ch[x][z]->pos.x = x;
			ch[x][z]->pos.z = z;
		}
	}*/

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
	glEnable(GL_CULL_FACE);

	/* vertex arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        //glEnableClientState(GL_NORMAL_ARRAY);

	build_vertex_arrays(ch[0][0]);
	print_vertices(vertices);
	print_indices(indices);

        glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), vertices);
        glColorPointer(3, GL_UNSIGNED_BYTE, 0, colors);
        //glNormalPointer(GL_FLOAT, 3*sizeof(GLfloat), normals);
        //glNormalPointer(GL_FLOAT, 0, normals);

	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

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

int write_cube_vertex_array(int x, int y, int z, char type, neighbours_t *nghb,
		GLfloat *vertices, GLubyte *colors, GLuint *indices, GLfloat *normals,
		long *vert_idx, long *col_idx, long *idx_idx, long *nml_idx)
{
	//int v = *vert_idx;
	int a, d, c;
	GLubyte r, g, b;
	switch(type) {
	case 1: /* stone */
		r = 128; g = 128; b = 128;
		break;
	case 7: /* bedrock */
		r = 56; g = 56; b = 56;
		break;
	case 3: /* dirt */
		r = 139; g = 69; b = 19;
		break;
	case 2: /* grass */
		r = 128; g = 255; b = 0;
		break;
	case 12: /* sand */
		r = 194; g = 178; b = 128;
		break;
	case 13: /* gravel */
		r = 110; g = 100; b = 100;
		break;
	case 9: /* water */
		r = 0; g = 0; b = 100;
		break;
	case 15: /* iron ore */
		r = 164; g = 164; b = 164;
		break;
	case 14: /* gold ore */
		r = 203; g = 161; b = 53;
		break;
	case 16: /* coal ore */
		r = 32; g = 32; b = 32;
		break;
	case 73: /* redstone ore */
		r = 128; g = 0; b = 0;
		break;
	case 49: /* obsidian */
		r = 0; g = 0; b = 0;
		break;
	case 11: /* lava */
		r = 255; g = 64; b = 0;
		break;
	case 48: /* mossy cobblestone */
		r = 0; g = 64; b = 0;
		break;
	case 4: /* cobblestone */
		r = 96; g = 96; b = 96;
		break;
	case 56: /* diamond */
		r = 0; g = 0; b = 200;
		break;
	default:
		printf("Unknown color for : %i\n", type);
		r = 200; g = 200; b = 200;
		break;
	}

	float norm_x;
	float norm_y;
	float norm_z;

	for (a = 0 ; a <= 1 ; a++) {
		norm_x = (a == 0 ? -1.f : 1.f) * 0.577350269;
		for (d = 0 ; d <= 1 ; d++) {
			norm_y = (b == 0 ? -1.f : 1.f) * 0.577350269;
			for (c = 0 ; c <= 1 ; c++) {
				norm_z = (c == 0 ? -1.f : 1.f) * 0.577350269;

				vertices[(*vert_idx) + (a*12)+(d*6)+(c*3)] = x+a;
				vertices[(*vert_idx) + (a*12)+(d*6)+(c*3)+1] = y+d;
				vertices[(*vert_idx) + (a*12)+(d*6)+(c*3)+2] = z+c;
				colors[(*col_idx) + (a*12)+(d*6)+(c*3)] = r;
				colors[(*col_idx) + (a*12)+(d*6)+(c*3)+1] = g;
				colors[(*col_idx) + (a*12)+(d*6)+(c*3)+2] = b;
				normals[(*nml_idx) + (a*12)+(d*6)+(c*3)] = norm_x;
				normals[(*nml_idx) + (a*12)+(d*6)+(c*3)+1] = norm_y;
				normals[(*nml_idx) + (a*12)+(d*6)+(c*3)+2] = norm_z;
				
			}	
		}
	}
	long i = *idx_idx;
	if (!nghb->zplus) {
		//glNormal3f(0.0f, 0.0f, 1.0f);
		indices[i++] = ((*vert_idx)/3)+1;
		indices[i++] = ((*vert_idx)/3)+5;
		indices[i++] = ((*vert_idx)/3)+7;
		indices[i++] = ((*vert_idx)/3)+3;
	}
	if (!nghb->zminus) {
		//glNormal3f(0.0f, 0.0f, -1.0f);
		indices[i++] = ((*vert_idx)/3)+0;
		indices[i++] = ((*vert_idx)/3)+2;
		indices[i++] = ((*vert_idx)/3)+6;
		indices[i++] = ((*vert_idx)/3)+4;
	}
	if (!nghb->yplus) {
		//glNormal3f(0.0f, 1.0f, 0.0f);
		indices[i++] = ((*vert_idx)/3)+2;
		indices[i++] = ((*vert_idx)/3)+3;
		indices[i++] = ((*vert_idx)/3)+7;
		indices[i++] = ((*vert_idx)/3)+6;
	}
	if (!nghb->yminus) {
		//glNormal3f(0.0f, -1.0f, 0.0f);
		indices[i++] = ((*vert_idx)/3)+0;
		indices[i++] = ((*vert_idx)/3)+4;
		indices[i++] = ((*vert_idx)/3)+5;
		indices[i++] = ((*vert_idx)/3)+1;
	}
	if (!nghb->xplus) {
		//glNormal3f(1.0f, 0.0f, 0.0f);
		indices[i++] = ((*vert_idx)/3)+4;
		indices[i++] = ((*vert_idx)/3)+6;
		indices[i++] = ((*vert_idx)/3)+7;
		indices[i++] = ((*vert_idx)/3)+5;
	}
	if (!nghb->xminus) {
		//glNormal3f(x, 0.0f, 0.0f);
		indices[i++] = ((*vert_idx)/3)+0;
		indices[i++] = ((*vert_idx)/3)+1;
		indices[i++] = ((*vert_idx)/3)+3;
		indices[i++] = ((*vert_idx)/3)+2;
	}



	*idx_idx = i;	/* */
	*vert_idx += 24;	/* 8 vertices per cube * 3 components (x,y,z) */
	*col_idx += 24;	/* one color per vertex * 3 components (r,g,b) */
	*nml_idx += 24;
	return 0;
}

void build_vertex_arrays(chunk_t *ch)
{
	long i;
	int x, y, z;
	neighbours_t nghb = {0};
	i = 0;

	long col_idx = 0, vert_idx = 0, nml_idx = 0;
	idx_idx = 0;

	for (x = 0 ; x < 16 ; x++) {
		for (z = 0 ; z < 16 ; z++) {
			for (y = 0 ; y < 128 ; y++) {
				char type = ch->blocks[i];
				nghb.yplus = (y+1 < 127) ? ch->blocks[i+1] : 0;
				nghb.yminus = (y-1 > 0) ? ch->blocks[i-1] : 0;
				nghb.zplus = (z+1 < 15) ? ch->blocks[i+128] : 0;
				nghb.zminus = (z-1 > 0) ? ch->blocks[i-128] : 0;
				nghb.xplus = (x+1 < 15) ? ch->blocks[i+(128*16)] : 0;
				nghb.xminus = (x-1 > 0) ? ch->blocks[i-(128*16)] : 0;
				//printf("computed id : %i VS expected : %i\n", y+(z*128)+(x*128*16), i);
				if (type == 0 || y < LIMIT_MIN_Y) {
					i++;
					//printf("%i,%i,%i(missed)\n", x, y, z);
					continue;
				}
				//printf("type: %i\n", type);
				write_cube_vertex_array(x+ch->pos.x*16, y, z+ch->pos.z*16, type, &nghb,
							vertices, colors, indices, normals,
							&vert_idx, &col_idx, &idx_idx, &nml_idx);
				i++;
			}
		}
	}
}

void draw()
{

	//int x, z;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	//gluLookAt(-1,64+distance,-1,8,64,8,0,1,0);
	gluLookAt(0,32+distance,0,8,32,8,0,1,0);
	//gluLookAt(-5, -5, -5, 0, 0, 0,0,1,0);
	glRotated(angle_x, 1, 0, 0);
	glRotated(angle_y, 0, 1, 0);
	//glRotated(angle_z, 0, 0, 1);
	//printf("=====================\n");
	//for (x = 0; x < MAX_X ; x++) {
	//	for (z = 0; z < MAX_Z ; z++) {
	//		glCallList(chunk_dl[x][z]);
	//	}
	//}
	//printf("%li elements to draw\n", idx_idx);
	glDrawElements(GL_QUADS, idx_idx, GL_UNSIGNED_INT, indices);

	glFlush();
	SDL_GL_SwapBuffers();
}
