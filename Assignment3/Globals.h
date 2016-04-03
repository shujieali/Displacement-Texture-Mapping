#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "glut.h"
#include <math.h>
#include <algorithm>
#include "imageloader.h"
#include "vec3f.h"


// --------------------------------------- Global Variables Defination ----------------------------------
#define TERR_D 20
#define TERR_W 20

float cam_xrot = 15, cam_yrot = 45, cam_zrot = 0, waterLever = 0, McamX = 0, McamY = 0, McamZ = 0;
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 100.0 };
GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };
float _angleX = 60.0f, angleY = 0, angleZ = 0; Image *image2;
Image *image1;
Image *image3;
Terrain* _terrain;


// Mouse
bool mouseleftdown = false;   // True if mouse LEFT button is down.

// Saved by mouse.
int mousex = 0, mousey = 0;           // Mouse x,y coords, in GLUT format (pixels from upper-left corner).
// Only guaranteed to be valid if a mouse button is down.
// Saved by mouse, motion.




// ----------------- 
int ImageLoad(char *filename, Image *image) {

	FILE *file;

	unsigned long size;
	unsigned long i; // standard counter.
	unsigned short int planes;
	unsigned short int bpp; //bits per pixel


	if ((file = fopen(filename, "rb")) == NULL){
		printf("File Not Found : %s\n", filename);
		return 0;
	}
	// seek through the bmp header, up to the width/height:
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
