#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "chunk.h"
#include "colors.h"
#include "utils.h"

int angle_x = 0;
int angle_y = 0;
int angle_z = 0;
int distance = 60;
int limit_min_y = 48;

void draw();

#define MIN_X -13
#define MIN_Z -16
#define MAX_X 15
#define MAX_Z 9

#define LEN_X (MAX_X - MIN_X)
#define LEN_Z (MAX_Z - MIN_Z)

struct vertex_t {
	GLfloat x;	// 32 bits
	GLfloat y;	// 32 bits
	GLfloat z;	// 32 bits
	GLubyte type;	// 8 bits
	GLubyte light;	// 8 bits
	GLubyte padding[2];// 16 bits
}; /* total : 128 bits */


chunk_t *ch[LEN_X][LEN_Z];
#define CHUNK_GET(x, z) \
	(ch[x-MIN_X][z-MIN_Z])

int need_redraw = 1;
int hide_walls = 0;

int count_redraws = 0;

int btn_down = 0;
int old_x = 0, old_y = 0;
#define BLK_PER_CHUNK (16*16*128)

#define VPACK_GET(x, z) \
	(vertices_pack[x-MIN_X][z-MIN_Z])
struct vertex_t vertices_pack[LEN_X][LEN_Z][8*BLK_PER_CHUNK];

#define IDX_GET(x, z) \
	(indices[x-MIN_X][z-MIN_Z])
GLuint indices[LEN_X][LEN_Z][6*4*BLK_PER_CHUNK]; /* 786k longs */

#define IDX_IDX_GET(x, z) \
	(idx_idx[x-MIN_X][z-MIN_Z])
long idx_idx[LEN_X][LEN_Z];

GLuint program_shd;
GLuint vbo_ids[2*LEN_X*LEN_Z];

void prepare_shader(char *vprog, char *pprog, GLuint *vertex, GLuint *pixel, GLuint *program)
{
	char *vsrc = NULL, *psrc = NULL;
	int vsrclen, psrclen;
	int i;
	int cnt;
	FILE *f;
	int compile_ok, link_ok;

	/* read and prepare vertex shader */
	*vertex = glCreateShader(GL_VERTEX_SHADER);
	i = 1;
	cnt = 0;
	f = fopen(vprog, "r");
	do {
		printf("read %i times\n", i-1);
		vsrc = realloc(vsrc, 1024*i);
		cnt += fread(vsrc+1024*(i-1), sizeof(char), 1024, f);
		i++;
	} while (!feof(f));
	fclose(f);
	vsrc[cnt] = '\0';
	vsrclen = strlen(vprog);
	printf("Vertex shader %s (%i) : %s\n", vprog, cnt, vsrc);
	glShaderSource(*vertex, 1, &vsrc, NULL);

	/* read and prepare pixel shader */
	*pixel = glCreateShader(GL_FRAGMENT_SHADER);
	i = 1;
	cnt = 0;
	f = fopen(pprog, "r");
	do {
		psrc = realloc(psrc, 1024*i);
		cnt += fread(psrc+1024*(i-1), sizeof(char), 1024, f);
		i++;
	} while (!feof(f));
	fclose(f);
	psrc[cnt] = '\0';
	psrclen = strlen(pprog);
	printf("Pixel shader : %s\n", psrc);
	glShaderSource(*pixel, 1, &psrc, NULL);

	/* compile vertex shader */
	glCompileShader(*vertex);
	glGetShaderiv(*vertex, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		printf("Could not compile vertex shader\n");
		char log[512];
		int s;
		glGetShaderInfoLog(*vertex, 512, &s, log);
		printf("%s\n", log);
		exit(0);
	}
	/* compile pixel shader */
	glCompileShader(*pixel);
	glGetShaderiv(*pixel, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		printf("Could not compile pixel shader\n");
		char log[512];
		int s;
		glGetShaderInfoLog(*pixel, 512, &s, log);
		printf("%s\n", log);
		exit(0);
	}

	*program = glCreateProgram();
	glAttachShader(*program, *vertex);
	glAttachShader(*program, *pixel);
	glLinkProgram(*program);

	glGetProgramiv(*program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		printf("Could not link program\n");
		char log[512];
		int s;
		glGetProgramInfoLog(*program, 512, &s, log);
		printf("%s\n", log);
		exit(0);
	}
	//glBindFragDataLocation(*program, 0, "gl_FragColor");
}

void benchmark(int value)
{
	printf("%f fps\n", count_redraws/10.f);
	count_redraws = 0;
	/* Do timer processing */ /* maybe glutPostRedisplay(), if necessary */
	/* call back again after elapsedUSecs have passed */
	glutTimerFunc (10000, &benchmark, 0);
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

void keyboard(unsigned char key, int x, int y)
{
	int limit_old = limit_min_y;
	int hide_walls_old = hide_walls;
	int recompute = 0;

	switch(key) {
		case '+':
			limit_min_y = MAX(limit_min_y-8, 0);
			break;
		case '-':
			limit_min_y = MIN(limit_min_y+8, 64);
			break;
		case 'w': /* hide walls */
			if (hide_walls) {
				hide_walls = 0;
				glEnable(GL_CULL_FACE);
			} else {
				hide_walls = 1;
				glDisable(GL_CULL_FACE);
			}
			break;
	}
	/* check if any change requires recomputing the arrays */
	if (limit_min_y != limit_old)
		recompute = 1;
	if (hide_walls_old != hide_walls)
		recompute = 1;

	if (recompute) {
		int x, z;
		for (x = MIN_X ; x < MAX_X ; x++) {
			for (z = MIN_Z ; z < MAX_Z ; z++) {
				idx_idx[x-MIN_X][z-MIN_Z] = 0;
				build_vertex_arrays(CHUNK_GET(x,z), IDX_GET(x, z),
						VPACK_GET(x, z), &IDX_IDX_GET(x, z));
#ifdef _USE_VBO
				/* bind and load indices */
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2]); /* index vbo */
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, (6*4*16*16*128*sizeof(GLuint)),
						IDX_GET(x, z), GL_STATIC_DRAW);
				/* bind and load vertices&shader args */
				glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2+1]);	   /* vertex vbo */
				glBufferData(GL_ARRAY_BUFFER, (8*BLK_PER_CHUNK*sizeof(struct vertex_t)),
						VPACK_GET(x, z), GL_STATIC_DRAW);
#endif
			}
		}
		glutPostRedisplay();
	}
}

void print_vertices(GLfloat *vertices)
{
	int i;
	for (i=0 ; i < 16 ; i++) {
		//printf("vert: {%f, %f, %f}\n", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
	}
}
void print_indices(GLuint *indices)
{
	int i;
	for (i=0 ; i < 16 ; i++) {
		//printf("indices: {%i, %i, %i, %i}\n", indices[i*4], indices[i*4+1], indices[i*4+2],indices[i*4+3]);
	}
}

int main(int argc, char *argv[])
{
	static struct option long_options[] = {
		{"x-min", 1, 0, 'x'},
		{"x-max", 1, 0, 'X'},
		{"z-min", 1, 0, 'z'},
		{"z-max", 1, 0, 'Z'},
		{"world", 1, 0, 'w'},
		{"nether", 1, 0, 'n'},
		{NULL, 0, NULL, 0}
	};
	int option_index = 0;
	int c;

	while ((c = getopt_long(argc, argv, "x:X:z:Z:w:n", long_options, &option_index)) != -1) {
		switch (c) {
		case 'x':
			printf("x option set\n");
			break;
		case 'X':
			printf("X option set\n");
			break;
		case 'z':
			printf("z option set\n");
			break;
		case 'Z':
			printf("Z option set\n");
			break;
		case 'w':
			printf("w option set\n");
			break;
		case 'n':
			printf("n option set\n");
			break;
		}
	}

	int x, z;
	GLuint vprog, pprog;
	for (x = MIN_X; x < MAX_X ; x++) {
		for (z = MIN_Z; z < MAX_Z ; z++) {
			char chunk_name[256];
			char *xdir, *zdir, *xfile, *zfile;
			xfile = b36enc(x);
			zfile = b36enc(z);
			xdir = b36enc(MOD(x, 64));
			zdir = b36enc(MOD(z, 64));
			//printf("%s / %s / %s / %s\n", xdir, zdir, xfile, zfile);
			snprintf(chunk_name, sizeof(chunk_name), "../save/world/%s/%s/c.%s.%s.dat", xdir, zdir, xfile, zfile);
			//printf("loading : %s\n", chunk_name);
			CHUNK_GET(x, z) = chunk_parse(chunk_name);
			CHUNK_GET(x, z)->pos.x = x;
			CHUNK_GET(x, z)->pos.z = z;
			free(xdir);
			free(zdir);
			free(xfile);
			free(zfile);
		}
	}
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutCreateWindow("GLUT Application");

	glutDisplayFunc(&draw);
	glutMouseFunc(&mouse);
	glutMotionFunc(&motion);
	glutKeyboardFunc(&keyboard);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(70,(double)640/480,1,1000);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	//glEnable(GL_LIGHTING) ;                 // Active la gestion des lumières
	glEnable(GL_LIGHT0) ;                      // allume la lampe 0
	glEnable(GL_CULL_FACE);
	//glEnable(GL_NORMALIZE);
	glShadeModel(GL_FLAT);
	/* enable VBO */
#ifdef _USE_VBO
	glGenBuffers(2*LEN_X*LEN_Z, vbo_ids);
#endif

	printf("GL_VERSION : %s\n", glGetString(GL_VERSION));
	printf("GL_SHADING_VERSION : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	/* vertex arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	for (x = MIN_X ; x < MAX_X ; x++) {
		for (z = MIN_Z ; z < MAX_Z ; z++) {
			build_vertex_arrays(CHUNK_GET(x, z), IDX_GET(x, z),
					VPACK_GET(x, z), &IDX_IDX_GET(x, z));
#ifdef _USE_VBO
			/* bind and load indices */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2]); /* index vbo */
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (6*4*16*16*128*sizeof(GLuint)),
					IDX_GET(x, z), GL_STATIC_DRAW);
			/* bind and load vertices&shader args */
			glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2+1]);	   /* vertex vbo */
			glBufferData(GL_ARRAY_BUFFER, (8*BLK_PER_CHUNK*sizeof(struct vertex_t)),
					VPACK_GET(x, z), GL_STATIC_DRAW);
#endif
		}
	}
	//print_vertices(vertices);
	//print_indices(indices);

	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	prepare_shader("shader.vert", "shader.frag", &vprog, &pprog, &program_shd);
	glUseProgram(program_shd);

#ifdef BENCHMARK
	glutTimerFunc (10000, &benchmark, 0);
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

int write_cube_vertex_array(int x, int y, int z,
		unsigned char type, unsigned char light,
		neighbours_t *nghb,
		GLuint *indices, struct vertex_t *vertices_pack,
		long *idx_idx, long *vert_idx)
{
	if (!nghb->zplus) {
		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}
	if (!nghb->zminus) {
		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}
	if (!nghb->yplus) {
		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}
	if (!nghb->yminus) {
		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}
	if (!nghb->xplus) {
		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x+1;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}
	if (!nghb->xminus) {
		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z+1;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;

		vertices_pack[*vert_idx].x = x;
		vertices_pack[*vert_idx].y = y+1;
		vertices_pack[*vert_idx].z = z;
		vertices_pack[*vert_idx].type = (GLubyte)type;
		vertices_pack[*vert_idx].light = (GLubyte)light;
		indices[*vert_idx] = *vert_idx;
		(*vert_idx)++;
	}

	*idx_idx = (*vert_idx);	/* */
	return 0;
}

#define GET_CHUNK_BLOCK_TYPE(ch_x, ch_z, x, y, z) \
	(CHUNK_GET(ch_x, ch_z)->blocks[(x*128*16)+(z*128)+y])

void fill_nghbs(chunk_t *c, int x, int y, int z, neighbours_t *nghb)
{
	int i = (x*128*16)+(z*128)+y;

	/* compute vertical neighbours */
	if (y == limit_min_y)
		nghb->yminus = hide_walls;
	else
		nghb->yminus = c->blocks[i-1];
	if (y == 127)
		nghb->yplus = 0;
	else
		nghb->yplus = c->blocks[i+1];

	/* compute Z neighbours */
	if (z == 0) {
		if (c->pos.z > MIN_Z)
			nghb->zminus = GET_CHUNK_BLOCK_TYPE(c->pos.x, c->pos.z-1, x, y, 15);
		else
			nghb->zminus = hide_walls;
	} else {
		nghb->zminus = c->blocks[i-128];
	}

	if (z == 15) {
		if (c->pos.z < MAX_Z-1)
			nghb->zplus = GET_CHUNK_BLOCK_TYPE(c->pos.x, c->pos.z+1, x, y, 0);
		else
			nghb->zplus = hide_walls;
	} else {
		nghb->zplus = c->blocks[i+128];
	}

	/* compute X neighbours */
	if (x == 0) {
		if (c->pos.x > MIN_X)
			nghb->xminus = GET_CHUNK_BLOCK_TYPE(c->pos.x-1, c->pos.z, 15, y, z);
		else
			nghb->xminus = hide_walls;
	} else {
		nghb->xminus = c->blocks[i-(128*16)];
	}
	if (x == 15)
		if (c->pos.x < MAX_X-1)
			nghb->xplus = GET_CHUNK_BLOCK_TYPE(c->pos.x+1, c->pos.z, 0, y, z);
		else
			nghb->xplus = hide_walls;
	else
		nghb->xplus = c->blocks[i+(128*16)];
}

void build_vertex_arrays(chunk_t *ch, GLuint *indices, struct vertex_t *vertices, long *idx_idx)
{
	long i;
	int x, y, z;
	neighbours_t nghb = {0};
	i = 0;

	long vert_idx = 0;
	for (x = 0 ; x < 16 ; x++) {
		for (z = 0 ; z < 16 ; z++) {
			for (y = 0 ; y < 128 ; y++) {
				char type = ch->blocks[i];
				//printf("computed id : %i VS expected : %i\n", y+(z*128)+(x*128*16), i);
				fill_nghbs(ch, x, y, z, &nghb);
				if (type == 0 || y < limit_min_y) {
					i++;
					//printf("%i,%i,%i(missed)\n", x, y, z);
					continue;
				}
				//printf("type: %i\n", type);
				unsigned char light = ((ch->sky_light[i/2] << (i%2==0 ? 4 : 0)) & 0xF0);
				write_cube_vertex_array(x+ch->pos.x*16, y, z+ch->pos.z*16, type, light,
						&nghb,
						indices, vertices,
						idx_idx, &vert_idx);
				i++;
			}
		}
	}
}

void draw()
{

	int x, z;
	GLint type_arg, colors_arg, light_arg;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	type_arg = glGetAttribLocation(program_shd, "type");
	light_arg = glGetAttribLocation(program_shd, "blk_light");
	colors_arg = glGetUniformLocation(program_shd, "colors");

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	gluLookAt(-1,64+distance,-1,MAX_X*8,(128-limit_min_y)/2,MAX_Z*8,0,1,0);
	glTranslatef(MAX_X*8, (128-limit_min_y)/2, MAX_Z*8);
	glRotated(angle_x, 1, 0, 0);
	glRotated(angle_y, 0, 1, 0);
	glTranslatef(-MAX_X*8, -(128-limit_min_y)/2, -MAX_Z*8);
	//printf("=====================\n");
	glEnable(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(type_arg);
	glEnableVertexAttribArray(light_arg);
	glEnableClientState(GL_VERTEX_ARRAY);
	for (x = MIN_X; x < MAX_X ; x++) {
		for (z = MIN_Z; z < MAX_Z ; z++) {
			/* link uniform data to shader */
			glUniform3fv(colors_arg, 128, blk_colors);
#ifdef _USE_VBO
			/* bind VBOs */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2]); /* index vbo */
			glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[(z-MIN_Z)*LEN_X*2+(x-MIN_X)*2+1]);	   /* vertex vbo */
			/* point to data */
			glVertexPointer(3, GL_FLOAT, sizeof(struct vertex_t), 0);
			glVertexAttribPointer(type_arg, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex_t), sizeof(GLfloat) * 3);
			glVertexAttribPointer(light_arg, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vertex_t), sizeof(GLubyte)+(sizeof(GLfloat)*3));
			glDrawElements(GL_QUADS, idx_idx[x-MIN_X][z-MIN_Z], GL_UNSIGNED_INT, (void *)0);
			//glDrawArrays(GL_QUADS, 0, idx_idx[x][z]);
#else
			glVertexPointer(3, GL_FLOAT, sizeof(struct vertex_t), &(vertices_pack[x-MIN_X][z-MIN_Z][0].x));
			glVertexAttribPointer(type_arg, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex_t), &(vertices_pack[x-MIN_X][z-MIN_Z][0].type));
			glVertexAttribPointer(light_arg, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vertex_t), &(vertices_pack[x-MIN_X][z-MIN_Z][0].light));
			//glDrawArrays(GL_QUADS, 0, idx_idx[x][z]);
			glDrawElements(GL_QUADS, idx_idx[x-MIN_X][z-MIN_Z], GL_UNSIGNED_INT, indices[x-MIN_X][z-MIN_Z]);
#endif

#ifdef _USE_VBO
			/* cleanup */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); /* index vbo */
			glBindBuffer(GL_ARRAY_BUFFER, 0); /* index vbo */
#endif
		}
	}
	glDisableVertexAttribArray(type_arg);
	glDisableVertexAttribArray(light_arg);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_VERTEX_ARRAY);
	//printf("%li elements to draw\n", idx_idx);

	glFlush();
	glutSwapBuffers();
	count_redraws++;
#ifdef BENCHMARK
	glutPostRedisplay();
#endif
}
