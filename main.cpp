#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include "KMeans.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace km{

	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;
	const float CAMERA_DISTANCE = 0.5f;

	typedef struct Screen{
		int width, height;
	} Screen;

	typedef struct Mouse{
		bool leftDown, rightDown;
		int x, y;
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

	Screen screen = { SCREEN_WIDTH, SCREEN_HEIGHT };
	Mouse mouse = { false, false, 0, 0 };
	Camera camera = { 0.0f, 0.0f, CAMERA_DISTANCE };
	char inputchange = 0;

	KMeans* kmeans;
	GLuint VBO;

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

		glBindBufferARB(GL_ARRAY_BUFFER, VBO);
		glBufferDataARB(GL_ARRAY_BUFFER, kmeans->V * 16, kmeans->vertices,
			  GL_DYNAMIC_DRAW);

		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 *sizeof(GLfloat)));
		
		glDrawArrays(GL_POINTS, 0, kmeans->V);

		glFlush();
		glPopMatrix();
		glutSwapBuffers();
	}


	void reshapeCB(int w, int h)
	{
		screen.width = w;
		screen.height = h;
		toPerspective(&screen);
	}

	void idleCB()
	{
		glutPostRedisplay();
	}

	void keyboardCB(unsigned char key, int x, int y)
	{
		keyboardInput(key, &inputchange);
	}

	void specialKeyboardCB(int key, int x, int y)
	{
		keyboardInput(key, &inputchange);
	}

	void mouseCB(int button, int state, int x, int y)
	{
		mouseInput(button, state, x, y, &mouse);
	}

	void mouseMotionCB(int x, int y)
	{
		mouseMove(x, y, &mouse, &camera);
	}

	void exitCB()
	{
		delete kmeans;
		glDeleteBuffers(1, &VBO);
	}

	void init(int argc, char **argv)
	{
		glewExperimental = GL_TRUE;
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
		glutInitWindowSize(screen.width, screen.height);
		glutInitWindowPosition(100, 100);
		int handle = glutCreateWindow("K-means 3D");
		GLenum err = glewInit();


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

		glGenBuffers(1, &VBO);
		kmeans = new KMeans();
	}

} // namespace km

int main(int argc, char **argv)
{
	km::init(argc, argv);
	glutMainLoop();
	return 0;
}
