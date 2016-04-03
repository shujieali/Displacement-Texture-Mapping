#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "glut.h"
#include <math.h>
#include <algorithm>
#include "imageloader.h"
#include "vec3f.h"

#define TERR_D 20
#define TERR_W 20

bool mouseleftdown = false;   
int mousex = 0, mousey = 0;   

float cam_xrot = 15, cam_yrot = 45, cam_zrot = 0, waterLever = 0, McamX = 0, McamY = 0 , McamZ = 0;
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 100.0 };
GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };
float _angleX = 60.0f, angleY = 0, angleZ = 0;



// Function Prototypes ////////////////////////////////////
void drawTerrain();
void drawAxes();

void cleanup();
void camera();
Image *image2;
Image *image1;
Image *image3;
void drawLand();
void drawWorld();

class Terrain {
private:
	Vec3f** normals;
	bool computedNormals;
	float** hs;
	int w; 
	int l; 
public:
	Terrain(int w2, int l2);
	~Terrain();
	int width(){return w;}
	int length() {return l;}
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}
	float getHeight(int x, int z) {return hs[z][x];}
	void computeNormals();
	Vec3f getNormal(int x, int z) {
		if (!computedNormals)
			computeNormals();
	return normals[z][x];
	}
};


Terrain::Terrain(int w2, int l2) {
	l = l2;
	w = w2;

	hs = new float*[l];
	for (int i = 0; i < l; i++)
		hs[i] = new float[w];

	normals = new Vec3f*[l];
	for (int i = 0; i < l; i++)
		normals[i] = new Vec3f[w];
	computedNormals = false;
}

Terrain::~Terrain() {
	for (int i = 0; i < l; i++)
		delete[] hs[i];
	delete[] hs;

	for (int i = 0; i < l; i++) 
		delete[] normals[i];
	delete[] normals;
}

void Terrain ::computeNormals() {
	if (computedNormals)
		return;

	Vec3f** normals2 = new Vec3f*[l];
	for (int i = 0; i < l; i++) 
		normals2[i] = new Vec3f[w];

	for (int z = 0; z < l; z++) {
		for (int x = 0; x < w; x++) {
			// calculating the normal by simple cross multiplicaiton of existing triangles, online source was used for help in this part.
			Vec3f sum(0.0f, 0.0f, 0.0f), out, in, left, right;
			if (z > 0)
				out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
			if (z < l - 1) 
				in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
			if (x > 0) 
				left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
			if (x < w - 1) 
				right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);

			if (x > 0 && z > 0)
				sum += out.cross(left).normalize();
			if (x > 0 && z < l - 1) 
				sum += left.cross(in).normalize();
			if (x < w - 1 && z < l - 1) 
				sum += in.cross(right).normalize();
			if (x < w - 1 && z > 0) 
				sum += right.cross(out).normalize();
			normals2[z][x] = sum;
		}
	}
	for (int z = 0; z < l; z++) {
		for (int x = 0; x < w; x++) {
			Vec3f sum = normals2[z][x];

			if (x > 0) {
				sum += normals2[z][x - 1] * 0.5f;
			}
			if (x < w - 1) {
				sum += normals2[z][x + 1] * 0.5f;
			}
			if (z > 0) {
				sum += normals2[z - 1][x] * 0.5f;
			}
			if (z < l - 1) {
				sum += normals2[z + 1][x] * 0.5f;
			}

			if (sum.magnitude() == 0) {
				sum = Vec3f(0.0f, 1.0f, 0.0f);
			}
			normals[z][x] = sum;
		}
	}
	for (int i = 0; i < l; i++)
		delete[] normals2[i];
	delete[] normals2;
	computedNormals = true;
}


Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	delete image;
	t->computeNormals();
	return t;
}

Terrain* _terrain;


int ImageLoad(char *filename, Image *image) { // source taken

	FILE *file;
	unsigned long size;
	unsigned long i; // standard counter.
	unsigned short int planes;
	unsigned short int bpp; //bits per pixel
	if ((file = fopen(filename, "rb")) == NULL){
		printf("File Not Found : %s\n", filename);
		return 0;
	}
	fseek(file, 18, SEEK_CUR);

	if ((i = fread(&image->width, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n", filename, image->sizeX);

	if ((i = fread(&image->height, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, image->sizeY);
	size = image->width * image->height * 3;

	if ((fread(&planes, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}

	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);
		return 0;
	}
	fseek(file, 24, SEEK_CUR);

	image->pixels = (char *)malloc(size);
	if (image->pixels == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}

	if ((i = fread(image->pixels, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}

	for (i = 0; i<size; i += 3) { // bgr -- > rgb
		char temp = image->pixels[i];
		image->pixels[i] = image->pixels[i + 2];
		image->pixels[i + 2] = temp;
	}
	return 1;
}


Image * loadTexture(char* filename){

	Image *image1;
	image1 = (Image *)malloc(sizeof(Image));
	if (!ImageLoad(filename, image1)) {
		exit(1);
	}
	return image1;
}


void initLight(void)
{
	GLfloat ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat position[] = { 0.0, 3.0, 2.0, 0.0 };
	GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat local_view[] = { 0.0 };

	glClearColor(0.1 ,0.5 ,1, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void initilizeTexture(Image *image1){
	
	if (image1 == NULL){
		printf("Image was not returned from loadTexture\n");
		exit(0);
	}
	//Texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->width, image1->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //TEXTURE
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

// Init ///////////////////////////////////////////////////
void init(void)
{
	_terrain = loadTerrain("heightmap1.bmp", 20);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	initLight();
	image1 = loadTexture("reflection__.bmp");
	image2 = loadTexture("brick_bump.bmp");
	image3 = loadTexture("sand_dn.bmp");

}
///////////////////////////////////////////////////////////
void display(void)
{
	//mydisplay();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	camera();
	glRotatef(-angleZ, 0.0f, 0.0f, 1.0f);
	glRotatef(-angleY, 1.0f, 0.0f, 0.0f);
	glRotatef(-_angleX, 0.0f, 1.0f, 0.0f);
	glTranslatef(McamX, 0, McamY);
	initilizeTexture(image1);
	drawTerrain();
	//Write your code here
	initilizeTexture(image2);
	drawLand();
	initilizeTexture(image3);
	drawWorld();
	glutSwapBuffers();
}


void drawWorld(){
	int sides = 1000, slices = 40;
	float radius = 130;
	int height = 100;
	int x = 0 , y = 0, z = 0;
	const float theta = 2. * (3.14) / (float)sides;
	float x2 = radius, z2 = 0;
	float dis = 50;
	glTranslatef(dis, -10,dis);
	glColor3f(1.0, 1.0, 1.0); // this line is not needed when lighting in enabled
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= sides; i++) {
		const float tx = (float)i / sides;
		const float nf = 1. / sqrt(x2*x2 + z2*z2),
		xn = x2*nf, zn = z2*nf;
		glTexCoord2f(tx, 0);
		glVertex3f(x + x2, y, z + z2);
		glTexCoord2f(tx, 1);
		glVertex3f(x + x2, y + height, z + z2);

		const float x3 = x2;
		x2 = cos(theta) * x2 - sin(theta) * z2;
		z2 = sin(theta) * x3 + cos(theta) * z2;
	}
	glEnd();
}

////////////////////////////////////////////////////////////
//Add more draw functions here

void drawLand(){
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	// Rotations for land
	glTranslatef(0.0f, 0.5f, 0.0);
	glRotatef(0.0, 1.0f, 0.0f, 0.0f);
	// coloring
	GLfloat ambientColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	GLfloat lightColor0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightPos0[] = { -0.5f, 0.8f, 0.1f, 0.0f };
	// adding lights
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	//Scaliing the land
	float scale = 5.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(_terrain->width() - 1) / 2, -1.0f, -(float)(_terrain->length() - 1) / 2);
	glColor3f(1.0, 1.0, 1.0); // this line is not needed when lighting in enabled
	for (int z = 0; z < _terrain->length() - 1; z++) {
		glBegin(GL_TRIANGLE_STRIP);
		// The t texture coordinate for z and z + 1
		float t0 = (float)z / (_terrain->length() - 1); // t0 in [0,1]
		float t1 = (float)(z + 1) / (_terrain->length() - 1); // t1 in [0,1]
		for (int x = 0; x < _terrain->length(); x++) {
			// The s texture coordinate for x
			float s = (float)x / (_terrain->length() - 1); // s in [0, 1]
			Vec3f normal = _terrain->getNormal(x, z); // also work if do not count normals in this
			glNormal3f(normal[0], normal[1], normal[2]);
			glTexCoord2f(s, t0);
			glVertex3f(x, _terrain->getHeight(x, z), z);

			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glTexCoord2f(s, t1);
			glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}
///////////////////////////////////////////////////////////
void drawTerrain(){
	//	initilizeTexture("reflection__.bmp");
	//GLfloat color[] = { 0.2, 0.8, 0.2 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, color);

	glColor3f(1.0, 1.0, 1.0); // this line is not needed when lighting in enabled
	glPushMatrix();
	glTranslatef(-TERR_W / 2, waterLever, -TERR_D / 2);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(TERR_W, 0, 0);
	glTexCoord2f(1,1);
	glVertex3f(TERR_W, 0, TERR_D);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, TERR_D);
	glEnd();
	glPopMatrix();
}


///////////////////////////////////////////////////////////
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
	glMatrixMode(GL_MODELVIEW);
	camera();
}
///////////////////////////////////////////////////////////
void camera(){
	glLoadIdentity();
	glTranslatef(McamX, -1+McamZ, -7.0*(McamY+1));
	glRotatef(cam_xrot, 1, 0, 0);
	glRotatef(cam_yrot, 0, 1, 0);
	glRotatef(cam_zrot, 0, 0, 1);
}

///////////////////////////////////////////////////////////
void keyboard(unsigned char key, int x, int y)
{
	// Camera controls - Rotation along principle axis
	switch (key) {
	case 'q':
		cam_xrot += 1;
		if (cam_xrot >360) cam_xrot -= 360;
		break;
	case 'Q':
		cam_xrot -= 1;
		if (cam_xrot < -360) cam_xrot += 360;
		break;
	case 'e':
		cam_yrot += 1;
		if (cam_yrot >360) cam_yrot -= 360;
		break;
	case 'E':
		cam_yrot -= 1;
		if (cam_yrot < -360) cam_yrot += 360;
		break;
	case 'z':
		cam_zrot += 1;
		if (cam_zrot >360) cam_zrot -= 360;
		break;
	case 'Z':
		cam_zrot -= 1;
		if (cam_zrot < -360) cam_zrot += 360;
		break;
	case 'p':
		_angleX += 1.0f;
		if (_angleX > 360) {
			_angleX -= 360;
		}
		break;
	case 'P':
		_angleX -= 1.0f;
		if (_angleX < 0) {
			_angleX += 360;
		}
		break;
	case 'w': // move up
		McamY-=0.02;
		break;
	case 's': // move down
		McamY+=0.02;
		break;
	case 'a': // move
		break; 
	case 'd':
		break;
	case 'l':
		waterLever+=0.02;
		break;
	case 'L':
		waterLever-=0.02;
		break;
	case 'u':
		McamZ += 0.02;
		break;
	case 'U':
		McamZ -= 0.02;
		break;
	case 27:
		cleanup();
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}



///////////////////////////////////////////////////////////
void cleanup() {
	delete _terrain;
}
void update(int value) {
	_angleX += 1.0f;
	if (_angleX > 360) {
		_angleX -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		mouseleftdown = (state == GLUT_DOWN);
		glutPostRedisplay(); 
	}
	mousex = x;
	mousey = y;
}
void motion(int x, int y)
{
	if (mouseleftdown){
		glutPostRedisplay();
	}
	if (mousex - x < 0)
	{
		cam_yrot += 0.2;
	}
	else if (mousex - x > 0){
		cam_yrot -= 0.2;
	}
	if (mousey - y < 0)
	{
		cam_zrot += 0.2;
	}
	else if (mousey - y > 0){
		cam_zrot -= 0.2;
	}
	mousex = x;
	mousey = y;
}
///////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1024, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(25, update, 0);
	glutMainLoop();
	return 0;
}
