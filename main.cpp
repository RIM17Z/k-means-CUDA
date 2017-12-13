#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include "CameraHelper.h"
#include "InputHelper.h"
#include "KMeans.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace KMeans{

	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;
	const float CAMERA_DISTANCE = 0.5f;
	KMeans* kmeans;
	GLuint VBO;
	Screen screen = { SCREEN_WIDTH, SCREEN_HEIGHT };
	Mouse mouse = { false, false, 0, 0 };
	Camera camera = { 0.0f, 0.0f, CAMERA_DISTANCE };
	char inputchange = 0;

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
		glBufferDataARB(GL_ARRAY_BUFFER, kmeans->getV() * 16, kmeans->getVertices(),
			GL_DYNAMIC_DRAW);

		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 * sizeof(GLfloat)));

		glDrawArrays(GL_POINTS, 0, kmeans->getV());

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

} // namespace KMeans

int main(int argc, char **argv)
{
	KMeans::init(argc, argv);
	glutMainLoop();
	return 0;
}
