#ifndef KMEANS_H_
#define KMEANS_H_
#include <GL/freeglut.h>

namespace km{
	typedef struct Pos {
		GLfloat x;
		GLfloat y;
		GLfloat z;
	};

	typedef struct DataPoint {
		Pos pos;
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte n;
	};

	class KMeans{
	private:
		Pos *hsums, *dsums;
		int *hccnt, *dccnt;

		static float rand_normal(float mean, float stddev);
		static void set(DataPoint *hv, DataPoint *ov, int *v, int *oc, int *c);
		void allocateVertices();
		void allocateCentroids();
		void getForgyCentroids();
		void init();
		void deleteVertices();
		void deleteCentroids();
		bool assignPoints(DataPoint *hostPoints, DataPoint *hostCentroids);
		void moveCentroids(DataPoint *hc, int *hccnt, Pos *hsums);
		bool update(DataPoint *hv, DataPoint *hc, int *hccnt, Pos *hsums);
		void toRGB(GLfloat h, GLfloat s, GLfloat v, GLubyte*r, GLubyte*g, GLubyte*b);
		GLfloat hue2rgb(GLfloat p, GLfloat q, GLfloat t);

	public:
		KMeans();
		int v, c, c2;
		bool converged;
		DataPoint *hv, *hc, *dv, *dc, *ov, *oc;
		void update();
		~KMeans();
	};

} // namespace km

#endif /* KMEANS_H_ */
