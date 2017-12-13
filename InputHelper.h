#ifndef INPUTHELPER_H
#define INPUTHELPER_H
#include "KMeansTypes.h"

namespace KMeans {
	void mouseInput(int button, int state, int x, int y, Mouse *mouse);
	void mouseMove(int x, int y, Mouse *mouse, Camera *camera);
	void keyboardInput(int key, char *change);
} // namespace KMeans
#endif