#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"


int angle_x = 0;
int angle_y = 0;
int angle_z = 0;
int distance = 60;

void draw();

#define MAX_X 9
#define MAX_Z 9

GLfloat blk_colors[255*3] =
{ 0.0, 0.0, 0.0
, 0.5, 0.5, 0.5		// stone
, 0.5, 1.0, 0.0		// grass
, 0.55, 0.27, 0.08	// dirt
, 0.38, 0.38, 0.38	// cobblestone
, 0.0, 0.0, 0.0		// 5
, 0.0, 0.0, 0.0		// 6
, 0.22, 0.22, 0.22	// bedrock
, 0.0, 0.0, 0.0		// 8
, 0.0, 0.0, 0.39	// water
, 0.0, 0.0, 0.0		// 10
, 1.0, 0.25, 0.0	// lava
, 0.76, 0.70, 0.5	// sand
, 0.43, 0.39, 0.39	// gravel
, 0.80, 0.63, 0.21	// gold ore
, 0.64, 0.64, 0.64	// iron ore
, 0.13, 0.13, 0.13	// coal ore
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 17
, 0.0, 0.0, 0.0		// 47
, 0.0, 0.25, 0.0	// mossy cobblestone
, 0.05, 0.0, 0.05	// obsidian
, 0.0, 0.0, 0.0		// 50
, 0.0, 0.0, 0.0		// 50
, 0.0, 0.0, 0.0		// 50
, 0.0, 0.0, 0.0		// 50
, 0.0, 0.0, 0.0		// 50
, 0.0, 0.0, 0.0		// 55
, 0.0, 0.0, 0.78	// diamond
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 57
, 0.0, 0.0, 0.0		// 72
, 0.5, 0.0, 0.0		// redstone ore
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.0, 0.0		// 74
, 0.0, 0.25, 0.0	// cacti
};

struct vertex_t {
	GLfloat x;	// 32 bits
	GLfloat y;	// 32 bits
	GLfloat z;	// 32 bits
	GLubyte type;	// 8 bits
	GLubyte light;	// 8 bits
	GLubyte padding[2];// 16 bits
}; /* total : 128 bits */

int limit_min_y = 0;
chunk_t *ch[MAX_X][MAX_Z];
void build_vertex_arrays(chunk_t *ch, GLuint *indices, struct vertex_t *vertices, long *idx_idx);
int need_redraw = 1;

int count_redraws = 0;

int btn_down = 0;
int old_x = 0, old_y = 0;
#define BLK_PER_CHUNK (16*16*128)

struct vertex_t vertices_pack[MAX_X][MAX_Z][8*BLK_PER_CHUNK];

GLuint indices[MAX_X][MAX_Z][6*4*BLK_PER_CHUNK]; /* 786k longs */
long idx_idx[MAX_X][MAX_Z];
GLuint program_shd;

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

GLuint vbo_ids[2*MAX_X*MAX_Z];
int main(int argc, char *argv[])
{

	int x, z;
	GLuint vprog, pprog;
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
	//glEnable(GL_LIGHTING) ;                 // Active la gestion des lumières
	glEnable(GL_LIGHT0) ;                      // allume la lampe 0
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_NORMALIZE);
	glShadeModel(GL_FLAT);
	/* enable VBO */
#ifdef _USE_VBO
	glGenBuffers(2*MAX_X*MAX_Z, vbo_ids);
#endif

	printf("GL_VERSION : %s\n", glGetString(GL_VERSION));
	printf("GL_SHADING_VERSION : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	/* vertex arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
        //glEnableClientState(GL_COLOR_ARRAY);
        //glEnableClientState(GL_NORMAL_ARRAY);
	for (x = 0 ; x < MAX_X ; x++) {
		for (z = 0 ; z < MAX_Z ; z++) {
			build_vertex_arrays(ch[x][z], indices[x][z], vertices_pack[x][z], &idx_idx[x][z]);
#ifdef _USE_VBO
			/* bind and load indices */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[z*MAX_X*2+x*2]); /* index vbo */
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (6*4*16*16*128*sizeof(GLuint)), indices[x][z], GL_STATIC_DRAW);
			/* bind and load vertices&shader args */
			glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[z*MAX_X*2+x*2+1]);	   /* vertex vbo */
			glBufferData(GL_ARRAY_BUFFER, (8*BLK_PER_CHUNK*sizeof(struct vertex_t)), vertices_pack[x][z], GL_STATIC_DRAW);
#endif
		}
	}
	//print_vertices(vertices);
	//print_indices(indices);

	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	prepare_shader("vertex.shd", "pixel.shd", &vprog, &pprog, &program_shd);
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
	//int v = *vert_idx;
	int a, b, c;

	for (a = 0 ; a <= 1 ; a++) {
		for (b = 0 ; b <= 1 ; b++) {
			for (c = 0 ; c <= 1 ; c++) {
				vertices_pack[(*vert_idx) + (a*4)+(b*2)+c].x = (x+a);
				vertices_pack[(*vert_idx) + (a*4)+(b*2)+c].y = (y+b);
				vertices_pack[(*vert_idx) + (a*4)+(b*2)+c].z = (z+c);
				vertices_pack[(*vert_idx) + (a*4)+(b*2)+c].type = (GLubyte)type;
				vertices_pack[(*vert_idx) + (a*4)+(b*2)+c].light = (GLubyte)light;
			}
		}
	}
	long i = *idx_idx;
	if (!nghb->zplus) {
		//glNormal3f(0.0f, 0.0f, 1.0f);
		indices[i++] = (*vert_idx)+1;
		indices[i++] = (*vert_idx)+5;
		indices[i++] = (*vert_idx)+7;
		indices[i++] = (*vert_idx)+3;
	}
	if (!nghb->zminus) {
		//glNormal3f(0.0f, 0.0f, -1.0f);
		indices[i++] = (*vert_idx)+0;
		indices[i++] = (*vert_idx)+2;
		indices[i++] = (*vert_idx)+6;
		indices[i++] = (*vert_idx)+4;
	}
	if (!nghb->yplus) {
		//glNormal3f(0.0f, 1.0f, 0.0f);
		indices[i++] = (*vert_idx)+2;
		indices[i++] = (*vert_idx)+3;
		indices[i++] = (*vert_idx)+7;
		indices[i++] = (*vert_idx)+6;
	}
	if (!nghb->yminus) {
		//glNormal3f(0.0f, -1.0f, 0.0f);
		indices[i++] = (*vert_idx)+0;
		indices[i++] = (*vert_idx)+4;
		indices[i++] = (*vert_idx)+5;
		indices[i++] = (*vert_idx)+1;
	}
	if (!nghb->xplus) {
		//glNormal3f(1.0f, 0.0f, 0.0f);
		indices[i++] = (*vert_idx)+4;
		indices[i++] = (*vert_idx)+6;
		indices[i++] = (*vert_idx)+7;
		indices[i++] = (*vert_idx)+5;
	}
	if (!nghb->xminus) {
		//glNormal3f(x, 0.0f, 0.0f);
		indices[i++] = (*vert_idx)+0;
		indices[i++] = (*vert_idx)+1;
		indices[i++] = (*vert_idx)+3;
		indices[i++] = (*vert_idx)+2;
	}



	*idx_idx = i;	/* */
	*vert_idx += 8;
	return 0;
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
				nghb.yplus = (y+1 < 127) ? ch->blocks[i+1] : 0;
				nghb.yminus = (y-1 > limit_min_y) ? ch->blocks[i-1] : 0;
				nghb.zplus = (z+1 < 15) ? ch->blocks[i+128] : 0;
				nghb.zminus = (z-1 > 0) ? ch->blocks[i-128] : 0;
				nghb.xplus = (x+1 < 15) ? ch->blocks[i+(128*16)] : 0;
				nghb.xminus = (x-1 > 0) ? ch->blocks[i-(128*16)] : 0;
				//printf("computed id : %i VS expected : %i\n", y+(z*128)+(x*128*16), i);
				if (type == 0) {
					i++;
					//printf("%i,%i,%i(missed)\n", x, y, z);
					continue;
				}
				//printf("type: %i\n", type);
				unsigned char light = ((ch->sky_light[i/2] << (i%2 ? 4 : 0)));
				if (light != 0 && light != 240)
				printf("Light for cube is : %i\n", light);
				write_cube_vertex_array(x+ch->pos.x*15, y, z+ch->pos.z*15, type, light,
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
	gluLookAt(-1,64+distance,-1,8,64,8,0,1,0);
	glRotated(angle_x, 1, 0, 0);
	glRotated(angle_y, 0, 1, 0);
	//printf("=====================\n");
	glEnable(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(type_arg);
	glEnableVertexAttribArray(light_arg);
	glEnableClientState(GL_VERTEX_ARRAY);
	for (x = 0; x < MAX_X ; x++) {
		for (z = 0; z < MAX_Z ; z++) {
			/* link uniform data to shader */
			glUniform3fv(colors_arg, 128, blk_colors);
#ifdef _USE_VBO
			/* bind VBOs */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[z*MAX_X*2+x*2]); /* index vbo */
			glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[z*MAX_X*2+x*2+1]);	   /* vertex vbo */
			/* point to data */
			glVertexPointer(3, GL_FLOAT, sizeof(struct vertex_t), 0);
			glVertexAttribPointer(type_arg, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex_t), sizeof(GLfloat) * 3);
			glVertexAttribPointer(light_arg, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vertex_t), sizeof(GLubyte)+(sizeof(GLfloat)*3));
			glDrawElements(GL_QUADS, idx_idx[x][z], GL_UNSIGNED_INT, (void *)0);
#else
			glVertexPointer(3, GL_FLOAT, sizeof(struct vertex_t), &(vertices_pack[x][z][0].x));
			glVertexAttribPointer(type_arg, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex_t), &(vertices_pack[x][z][0].type));
			glVertexAttribPointer(light_arg, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct vertex_t), &(vertices_pack[x][z][0].light));
			glDrawElements(GL_QUADS, idx_idx[x][z], GL_UNSIGNED_INT, indices[x][z]);
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
