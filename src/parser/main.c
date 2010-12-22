#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <pthread.h>

#include "chunk.h"


int angle_x = 0;
int angle_y = 0;
int angle_z = 0;
int distance = 60;

void draw();

#define MAX_X 8
#define MAX_Z 8


int limit_min_y = 32;
chunk_t *ch[MAX_X][MAX_Z];
void build_vertex_arrays(chunk_t *ch, GLfloat *vertices, GLubyte *colors, GLuint *indices, GLfloat *normals);
int need_redraw = 1;

int count_redraws = 0;

int btn_down = 0;
int old_x = 0, old_y = 0;
GLfloat vertices[MAX_X][MAX_Z][8*3*16*16*128]; /* 786k floats */
GLfloat normals[MAX_X][MAX_Z][8*3*16*16*128]; /* 786k floats */
GLubyte colors[MAX_X][MAX_Z][8*3*16*16*128]; /* 786k floats */
GLuint indices[MAX_X][MAX_Z][6*4*16*16*128]; /* 786k longs */
long idx_idx;

void benchmark(int value)
{
	printf("%i fps\n", count_redraws);
	count_redraws = 0;
	/* Do timer processing */ /* maybe glutPostRedisplay(), if necessary */
	/* call back again after elapsedUSecs have passed */
	glutTimerFunc (1000, &benchmark, 0);
}

void mouse(int btn, int state, int x, int y)
{
	old_x = x;
	old_y = y;
	switch(btn) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			btn_down = 1;
		} else {
			btn_down = 0;
		}
		break;
	case 3:
		distance -=1;
		glutPostRedisplay();
		break;
	case 4:
		distance +=1;
		glutPostRedisplay();
		break;
	}
}

void motion(int x, int y)
{
	
	if (btn_down) {
		angle_x = (angle_x + (x - old_x))%360;
		angle_y = (angle_y + (y - old_y))%360;
		glutPostRedisplay();
	}
	old_x = x;
	old_y = y;
	//printf("motion : %i,%i\n", x, y);
}

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
	}
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutCreateWindow("GLUT Application");
	
	glutDisplayFunc(&draw);
	glutMouseFunc(&mouse);
	glutMotionFunc(&motion);

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
	for (x = 0 ; x < MAX_X ; x++) {
		for (z = 0 ; z < MAX_Z ; z++) {
			build_vertex_arrays(ch[x][z],vertices[x][z],
					colors[x][z], indices[x][z],
					normals[x][z]);
		}
	}
	//print_vertices(vertices);
	//print_indices(indices);

	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

#ifdef BENCHMARK
	glutTimerFunc (1000, &benchmark, 0);
#endif
	glutMainLoop();

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
	case 81:
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

void build_vertex_arrays(chunk_t *ch, GLfloat *vertices, GLubyte *colors, GLuint *indices, GLfloat *normals)
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
				nghb.yminus = (y-1 > limit_min_y) ? ch->blocks[i-1] : 0;
				nghb.zplus = (z+1 < 15) ? ch->blocks[i+128] : 0;
				nghb.zminus = (z-1 > 0) ? ch->blocks[i-128] : 0;
				nghb.xplus = (x+1 < 15) ? ch->blocks[i+(128*16)] : 0;
				nghb.xminus = (x-1 > 0) ? ch->blocks[i-(128*16)] : 0;
				//printf("computed id : %i VS expected : %i\n", y+(z*128)+(x*128*16), i);
				if (type == 0 || y < limit_min_y) {
					i++;
					//printf("%i,%i,%i(missed)\n", x, y, z);
					continue;
				}
				//printf("type: %i\n", type);
				write_cube_vertex_array(x+ch->pos.x*15, y, z+ch->pos.z*15, type, &nghb,
							vertices, colors, indices, normals,
							&vert_idx, &col_idx, &idx_idx, &nml_idx);
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
	//gluLookAt(-5, -5, -5, 0, 0, 0,0,1,0);
	glRotated(angle_x, 1, 0, 0);
	glRotated(angle_y, 0, 1, 0);
	//glRotated(angle_z, 0, 0, 1);
	//printf("=====================\n");
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), vertices[x][z]);
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, colors[x][z]);
			glDrawElements(GL_QUADS, idx_idx, GL_UNSIGNED_INT, indices[x][z]);
		}
	}
	//printf("%li elements to draw\n", idx_idx);

	glFlush();
	glutSwapBuffers();
	count_redraws++;
#ifdef BENCHMARK
	glutPostRedisplay();
#endif
}
