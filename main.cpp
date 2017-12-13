//#define GL_GLEXT_PROTOTYPES
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <GL/freeglut.h>
#include "KMeans.h"

template <typename T> std::string to_string(T value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

namespace km{

	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;
	const float CAMERA_DISTANCE = 0.5f;

	namespace helpers{

		typedef struct Screen{
			int width, height;
		} Screen;

		typedef struct Mouse{
			bool leftDown, rightDown;
			float x, y;
		} Mouse;

		typedef struct Camera{
			float angleX, angleY, distance;
		} Camera;

		void toOrtho(Screen *screen)
		{
			// set viewport to be the entire window
			glViewport(0, 0, (GLsizei)screen->width, (GLsizei)screen->height);
			// set orthographic viewing frustum
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			// FOV, AspectRatio, NearClip, FarClip
			glOrtho(0, screen->width, 0, screen->height, -1, 1);
			// switch to modelview matrix in order to set scene
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}

		void toPerspective(Screen *screen)
		{
			// set viewport to be the entire window
			glViewport(0, 0, (GLsizei)screen->width, (GLsizei)screen->height);
			// set perspective viewing frustum
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			// FOV, AspectRatio, NearClip, FarClip
			gluPerspective(40.0f, (float)(screen->width) / screen->height, .4f, 1000.0f);
			// switch to modelview matrix in order to set scene
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}

		void keyboardInput(int key, char *change){
			switch (key)
			{
			case 27: // ESCAPE
				*change = 'E';
				break;
			case 13: // ENTER
				*change = 'R';
				break;
			default:
				break;
			}
		}

		void mouseInput(int button, int state, int x, int y, Mouse *mouse){
			mouse->x = x;
			mouse->y = y;
			if (button == GLUT_LEFT_BUTTON)
			{
				if (state == GLUT_DOWN)
				{
					mouse->leftDown = true;
				}
				else if (state == GLUT_UP)
					mouse->leftDown = false;
			}
		}

		void mouseMove(int x, int y, Mouse *mouse, Camera *camera){
			if (mouse->leftDown)
			{
				camera->angleY += (x - mouse->x);
				camera->angleX += (y - mouse->y);
				mouse->x = x;
				mouse->y = y;
			}
		}

	} // namespace helpers

	helpers::Screen screen = { SCREEN_WIDTH, SCREEN_HEIGHT };
	helpers::Mouse mouse = { false, false, 0.0f, 0.0f };
	helpers::Camera camera = { 0.0f, 0.0f, CAMERA_DISTANCE };
	char inputchange = 0;

	KMeans* kmeans;

	void submenuCB(int);
	void menuCB(int);
	void displayCB();
	void reshapeCB(int w, int h);
	void idleCB();
	void keyboardCB(unsigned char key, int x, int y);
	void specialKeyboardCB(int key, int x, int y);
	void mouseCB(int button, int stat, int x, int y);
	void mouseMotionCB(int x, int y);
	void exitCB();

	void handleInput();
	void drawAxes();

	void init(int argc, char **argv)
	{

		kmeans = new KMeans();

		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
		glutInitWindowSize(screen.width, screen.height);
		glutInitWindowPosition(100, 100);
		int handle = glutCreateWindow("K-means 3D");

		// register GLUT callback functions
		glutDisplayFunc(displayCB);
		atexit(exitCB);
		glutIdleFunc(idleCB);
		glutReshapeFunc(reshapeCB);
		glutKeyboardFunc(keyboardCB);
		glutSpecialFunc(specialKeyboardCB);
		glutMouseFunc(mouseCB);
		glutMotionFunc(mouseMotionCB);
		glClearColor(0, 0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	void displayCB()
	{
		handleInput();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glPushMatrix();
		glTranslatef(0, 0, -camera.distance);
		gluLookAt(3, 2, 3, 0, 0.5, -0.5, 0, 1, 0);
		glRotatef(camera.angleX, 1, 0, 0);   // pitch
		glRotatef(camera.angleY, 0, 1, 0);   // heading
		glPointSize(1);

		drawAxes();

		kmeans->update();
		kmeans->draw();

		glPopMatrix();
		glutSwapBuffers();
	}

	void handleInput(){
		switch (inputchange){
		case 'E':
			glutLeaveMainLoop();
			break;
		case 'R':
			delete kmeans;
			kmeans = new KMeans();
			break;
		default:
			break;
		}
		inputchange = 0;
	}

	void drawAxes(){
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(2, 0, 0);
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 2, 0);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 2);
		glEnd();
	}

	void reshapeCB(int w, int h)
	{
		screen.width = w;
		screen.height = h;
		helpers::toPerspective(&screen);
	}

	void idleCB()
	{
		glutPostRedisplay();
	}

	void keyboardCB(unsigned char key, int x, int y)
	{
		helpers::keyboardInput(key, &inputchange);
	}

	void specialKeyboardCB(int key, int x, int y)
	{
		helpers::keyboardInput(key, &inputchange);
	}

	void mouseCB(int button, int state, int x, int y)
	{
		helpers::mouseInput(button, state, x, y, &mouse);
	}

	void mouseMotionCB(int x, int y)
	{
		helpers::mouseMove(x, y, &mouse, &camera);
	}

	void exitCB()
	{
		delete kmeans;
	}
} // namespace km

int main(int argc, char **argv)
{
	km::init(argc, argv);
	glutMainLoop();
	return 0;
}
