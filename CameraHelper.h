#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

namespace KMeans {
	typedef struct Screen{
		int width, height;
	} Screen;

	void toOrtho(Screen *screen);
	void toPerspective(Screen *screen);
} // namespace KMeans
#endif
