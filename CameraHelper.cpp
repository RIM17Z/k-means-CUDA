#include "CameraHelper.h"
#include <GL/freeglut.h>

namespace KMeans{
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
}