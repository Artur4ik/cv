//����������� ����������� ���������
#include <highgui.h>
#include <cv.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include "scene.h"
#include "glut.h"          

#define KMEANS_CLUSTERS 2

void kmeans(
            int  dim,		                  // dimension of data 
            float *X,                        // pointer to data
            int   n,                          // number of elements
            int   k,                          // number of clusters
            float *cluster_centroid,         // initial cluster centroids
            int   *cluster_assignment_final   // output
           );
void DrawFPS(IplImage * pic, int _fps, int clusters);
void DrawTeapots();
void DrawWalls();
void BGRToRGB(IplImage * pic);

const int SCREEN_WIDTH = 1440;  // ���������� ����� OpenGL&CV �� �����������
const int SCREEN_HEIGHT = 900;  // ���������� ����� OpenGL&CV �� ���������

using namespace std; //����������� ������������ ���

float *depth, *forkmeans;
double angle = 0 /*���� �������� ������ XOZ*/, step = 0 /*��� ������*/, camLookAt[3] = {0,0,0}/*����������� ������� ������*/;
int last = 0, fpstmp = 0/*����������� ���������� ��� ����������� FPS*/, fps = 0/*���������� ������ � �������*/;
Camera MyCam; // ������� �����, ���������� �� ���������� �������
char *output, *color;
long int t1 = 0;
int *out;

GLfloat arr[4] = {50.0,30.0,50.0,1.0};
GLfloat arr2[3] = {50.0,-1.0,50.0};
GLfloat colorX[3] = {1,0,1};
GLfloat colorY[3] = {0,0,1};
GLfloat colorZ[3] = {1,0,0};
GLfloat colorA[3] = {1,1,0};
GLfloat colorBlack[3] = {0.5,0.3,0.2};
GLfloat amb[4] = {0,1,0,0};
GLfloat colorTeapot[3] = {0,1,0};


CvSize Size;
IplImage *img/*������� �����������, �������������� OpenCV*/, *gray = 0, *dst = 0;


void keybord(unsigned char key, int x, int y)
{
	if (key == 'j' || key == 238)
	{
	

	}
	
	if (key == 'w' || key == 246) // �������� ������
	{
		MyCam.MoveForward(1);
	}

	if (key == 's' || key == 251) // �������� �����
	{
		MyCam.MoveBack(1);
	}

	if (key == 'a' || key == 244) // ������� ������ �������
	{
		MyCam.Rotate(-1);
	}

	if (key == 'd' || key == 226) // ������� ������ ������
	{
		MyCam.Rotate(1);
	}

	glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
	//std::cout << "Mouse X: " << x << ", Y: " << y << std::endl;
}

void idle(void)
{
	// ������� �������� ������� 
	angle+=0.3;
	if(angle>360) angle = 0;

	glutPostRedisplay();
}

void display(void)
{

	/* ����������� ���������� ������ � ������� (FPS)*/
	long int t2 = GetTickCount(); 
	if(t2-t1>1000)
	{
		fps = fpstmp;
		fpstmp = 0;
		t1=t2;
	} else fpstmp++;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_SPECULAR, amb);
	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_POSITION, arr);
	gluPerspective(60,SCREEN_WIDTH/SCREEN_HEIGHT,10,200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(180,0,1,0);
	glRotated(MyCam.GetAngleXOZ(),0,1,0);
	glTranslated(-MyCam.GetX(), -MyCam.GetY(), -MyCam.GetZ());
	img->origin = IPL_ORIGIN_BL;

	DrawTeapots();
	DrawWalls();

	float clusters[] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};

    glReadPixels(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
	glReadPixels(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, img->imageData);

	dst->origin = CV_ORIGIN_BL;
	
	cvCvtColor(img, gray, CV_RGB2GRAY);
    cvCanny(gray, dst, 4, 100, 3);

	for(int i = 0, c = 0, x = 0, y = 0; i<SCREEN_WIDTH*SCREEN_HEIGHT*4; i+=4)
	{
		forkmeans[i] = depth[c];
        if((int)dst->imageData[c] == 0) 
		{
			forkmeans[i+1] = 0; 
		}
		else 
		{
			forkmeans[i+1] = 1;
		}
		if(x > SCREEN_WIDTH)
		{
			y++;
			x = 0;
		}
		forkmeans[i+2] = (float)(x*((float)1.0/(float)SCREEN_WIDTH));
		forkmeans[i+3] = (float)(y*((float)1.0/(float)SCREEN_HEIGHT));
		c++; x++;
	}
	kmeans(4, forkmeans, SCREEN_WIDTH*SCREEN_HEIGHT, KMEANS_CLUSTERS, clusters, out);

	for(int i = 0, a = 0; i<SCREEN_WIDTH*SCREEN_HEIGHT*3; i+=3)
	{
		if(out[a] == 0)
		{
			img->imageData[i] = 0;
			img->imageData[i+1] = 0;
			img->imageData[i+2] = 0;
		}
		else
		{
			img->imageData[i] = 255;
			img->imageData[i+1] = 255;
			img->imageData[i+2] = 255;
		}
			
		a++;
	}
	DrawFPS(img, fps, KMEANS_CLUSTERS);
	glFlush();
	glutSwapBuffers();
	//cvShowImage("cvCanny", dst);
	cvShowImage("����������� ��������", img);	
}

int main(int argc, char **argv)
{
	Size.height = SCREEN_HEIGHT; Size.width = SCREEN_WIDTH;
	img = cvCreateImage(Size, IPL_DEPTH_8U, 3);
	gray = cvCreateImage(Size, IPL_DEPTH_8U, 1);
    dst = cvCreateImage(Size, IPL_DEPTH_8U, 1);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE |  GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutCreateWindow("OpenGL �������� ���");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keybord);
	glutMotionFunc(mouseMotion);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	out = new int[SCREEN_WIDTH*SCREEN_HEIGHT];
	depth = new float[SCREEN_WIDTH*SCREEN_HEIGHT];
	color = new char[SCREEN_WIDTH*SCREEN_HEIGHT*3];
	forkmeans = new float[SCREEN_WIDTH*SCREEN_HEIGHT*4];
	glutMainLoop();
	delete[]out;
	delete[]depth;
	delete[]color;
	delete[]forkmeans;
	return 0;
}

void DrawFPS(IplImage * pic, int _fps, int clusters)
{
	CvPoint pt = cvPoint( SCREEN_WIDTH-100, SCREEN_HEIGHT-30 );
    CvFont font;
    cvInitFont( &font, CV_FONT_HERSHEY_SIMPLEX,0.7, 0.7, 0, 0, CV_AA);
	char buf[20];
	sprintf(buf,"FPS: %d",_fps);
	cvPutText(pic, buf, pt, &font, CV_RGB(100, 100, 255));
	pt = cvPoint( 10, SCREEN_HEIGHT-30 );
	sprintf(buf,"CLUSTERS: %d", clusters);
	cvPutText(pic, buf, pt, &font, CV_RGB(100, 100, 255));
}

void BGRToRGB(IplImage * pic)
{
	for(int i = 0; i<(SCREEN_WIDTH*SCREEN_HEIGHT*3); i+=3)
	{
		char tmp;
		tmp = pic->imageData[i];
		pic->imageData[i] = img->imageData[i+2];
		pic->imageData[i+2] = tmp;
	}
}

void DrawTeapots()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorTeapot);
  
	glPushMatrix();
	glTranslated(95,9.5,65);
	glRotated(angle,0,1,0);
	glutSolidTeapot(3);
	glPopMatrix();

	glPushMatrix();
	glTranslated(95,9.5,80);
	glRotated(angle,0,1,0);
	glutSolidDodecahedron();
	glPopMatrix();

	glPushMatrix();
	glTranslated(95, 9.5, 50);
	glRotated(-angle,0,1,0);
	glutWireTeapot(3);
	glPopMatrix();

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorA);
	glPushMatrix();
	glTranslated(95, 2, 50);
	glutSolidCube(10);
	glPopMatrix();

	glPushMatrix();
	glTranslated(95, 2, 65);
	glutSolidCube(10);
	glPopMatrix();

	glPushMatrix();
	glTranslated(95, 2, 80);
	glutSolidCube(10);
	glPopMatrix();
}

void DrawWalls()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorY);
  glBegin(GL_QUADS);
		glVertex3d(100,-1,100);
		glVertex3d(100,-1,0);
		glVertex3d(0,-1,0);
		glVertex3d(0,-1,100);
  glEnd();

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorX);

    glBegin(GL_QUADS);
		glVertex3d(0,-1,0);
		glVertex3d(100,-1,0);
		glVertex3d(100,30,0);
		glVertex3d(0,30,0);
  glEnd();

      glBegin(GL_QUADS);
		glVertex3d(0,-1,100);
		glVertex3d(0,-1,0);
		glVertex3d(0,30,0);
		glVertex3d(0,30,100);
  glEnd();

        glBegin(GL_QUADS);
		glVertex3d(0,-1,100);
		glVertex3d(100,-1,100);
		glVertex3d(100,30,100);
		glVertex3d(0,30,100);
  glEnd();

          glBegin(GL_QUADS);
		glVertex3d(100,-1,100);
		glVertex3d(100,-1,0);
		glVertex3d(100,30,0);
		glVertex3d(100,30,100);
  glEnd();
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colorZ);
            glBegin(GL_QUADS);
		glVertex3d(0,30,0);
		glVertex3d(0,30,100);
		glVertex3d(100,30,100);
		glVertex3d(100,30,0);
  glEnd();
}