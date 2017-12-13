#ifndef INPUTHELPER_H
#define INPUTHELPER_H

namespace KMeans {
	typedef struct Camera{
		float angleX, angleY, distance;
	} Camera;

	typedef struct Mouse{
		bool leftDown, rightDown;
		int x, y;
	} Mouse;

	void mouseInput(int button, int state, int x, int y, Mouse *mouse);
	void mouseMove(int x, int y, Mouse *mouse, Camera *camera);
	void keyboardInput(int key, char *change);
} // namespace KMeans
#endif