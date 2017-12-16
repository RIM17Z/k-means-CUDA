#include "InputHelper.h"
#include <GL/freeglut.h>

namespace KMeans{
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

	void keyboardInput(int key, char *change){
		switch (key)
		{
		case 27: // ESCAPE
			*change = 'E';
			break;
		case ' ':
			*change = 1;
			break;
		case 13: // ENTER
			*change = 'R';
			break;
		default:
			break;
		}
	}
}