#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include <iostream>
#include "CameraHelper.h"
#include "InputHelper.h"
#include "TextHelper.h"
#include "KMeans.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

typedef LARGE_INTEGER app_timer_t;

static inline void timer(app_timer_t *t_ptr)
{
#ifdef __CUDACC__
	checkCudaErrors(cudaDeviceSynchronize());
#endif
	QueryPerformanceCounter(t_ptr);
}

double elapsed_time(app_timer_t start, app_timer_t stop)
{
	LARGE_INTEGER clk_freq;
	QueryPerformanceFrequency(&clk_freq);
	return (stop.QuadPart - start.QuadPart) /
		(double)clk_freq.QuadPart * 1e3;
}

#else

#include <time.h> /* requires linking with rt library
(command line option -lrt) */

typedef struct timespec app_timer_t;

static inline void timer(app_timer_t *t_ptr)
{
#ifdef __CUDACC__
	checkCudaErrors(cudaDeviceSynchronize());
#endif
	clock_gettime(CLOCK_MONOTONIC, t_ptr);
}

double elapsed_time(app_timer_t start, app_timer_t stop)
{
	return 1e+3 * (stop.tv_sec - start.tv_sec) +
		1e-6 * (stop.tv_nsec - start.tv_nsec);
}

#endif
namespace KMeans{

	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;
	const int TEXT_WIDTH = 8;
	const int TEXT_HEIGHT = 13;
	const float CAMERA_DISTANCE = 0.5f;
	KMeans* kmeans;
	GLuint VBO;
	Screen screen = { SCREEN_WIDTH, SCREEN_HEIGHT };
	Mouse mouse = { false, false, 0, 0 };
	Camera camera = { 0.0f, 0.0f, CAMERA_DISTANCE };
	char inputchange = 0;
	float drawTime, updateTime = 0;

	app_timer_t t1, t2, t3;

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


	void showFPS(int textWidth, int textHeight, Screen screen)
	{
		static int frameCount = 0;
		static std::string fps = "0.0 FPS";
		static int previousTime = 0;

		// backup current model-view matrix
		glPushMatrix();                     // save current modelview matrix
		glLoadIdentity();                   // reset modelview matrix
		// set to 2D orthogonal projection
		glMatrixMode(GL_PROJECTION);        // switch to projection matrix
		glPushMatrix();                     // save current projection matrix
		glLoadIdentity();                   // reset projection matrix
		gluOrtho2D(0, screen.width, 0, screen.height); // set to orthogonal projection

		frameCount++;
		//  Get the number of milliseconds since glutInit called
		//  (or first call to glutGet(GLUT ELAPSED TIME)).
		int currentTime = glutGet(GLUT_ELAPSED_TIME);
		int timeInterval = currentTime - previousTime;
		if (timeInterval > 1000)
		{
			fps = to_string((int)(frameCount / (timeInterval / 1000.0f))) + " FPS";
			previousTime = currentTime;
			frameCount = 0;
		}
		drawText(fps.c_str(), screen.width - fps.size()*textWidth,
			screen.height - textHeight);

		// restore projection matrix
		glPopMatrix();                      // restore to previous projection matrix
		// restore modelview matrix
		glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
		glPopMatrix();                      // restore to previous modelview matrix
	}

	void showInfo()
	{
		std::string text;
		// backup current model-view matrix
		glPushMatrix();                 // save current modelview matrix
		glLoadIdentity();               // reset modelview matrix
		// set to 2D orthogonal projection
		glMatrixMode(GL_PROJECTION);    // switch to projection matrix
		glPushMatrix();                 // save current projection matrix
		glLoadIdentity();               // reset projection matrix
		gluOrtho2D(0, screen.width, 0, screen.height); // set to orthogonal projection

		int line = screen.height - 2 * TEXT_HEIGHT;
		text = "Updating Time: " + to_string(updateTime) + " ms";
		drawText(text.c_str(), 1, line);

		line -= TEXT_HEIGHT;
		text = "Drawing Time: " + to_string(drawTime) + " ms";
		drawText(text.c_str(), 1, line);

		line -= TEXT_HEIGHT;
		text = "Clusters count: " + to_string(kmeans->getC());
		drawText(text.c_str(), 1, line);

		line -= TEXT_HEIGHT;
		text = "Points count: " + to_string(kmeans->getV());
		drawText(text.c_str(), 1, line);

		line -= TEXT_HEIGHT;
		text = kmeans->isConverged() ? "<<<Converged>>>" : "running...";
		drawText(text.c_str(), 1, line);

		line = TEXT_HEIGHT;
		text = "Press ENTER to rerun clustering.";
		drawText(text.c_str(), 1, line);

		// restore projection matrix
		glPopMatrix();                   // restore to previous projection matrix
		// restore modelview matrix
		glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
		glPopMatrix();                   // restore to previous modelview matrix
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

		timer(&t1); //--------------------------------------------
		kmeans->update();
		timer(&t2); //--------------------------------------------

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
		timer(&t3); //--------------------------------------------

		updateTime = elapsed_time(t1, t2);
		drawTime = elapsed_time(t2, t3);
		showInfo();
		showFPS(TEXT_WIDTH, TEXT_HEIGHT, screen);

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
