#ifndef KMEANSTYPES_H_
#define KMEANSTYPES_H_
#include <GL/freeglut.h>

#define MAX_CLUSTERS_CNT 512

namespace KMeans {

	typedef struct Camera{
		float angleX, angleY, distance;
	} Camera;

	typedef struct Mouse{
		bool leftDown, rightDown;
		int x, y;
	} Mouse;

	typedef struct Screen{
		int width, height;
	} Screen;

	typedef struct Pos {
		GLfloat x;
		GLfloat y;
		GLfloat z;
	} Pos;

	typedef struct DataPoint {
		Pos pos;
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte cluster_id;
	} DataPoint;
} // namespace KMeans

#endif /* KMEANS_H_ */
