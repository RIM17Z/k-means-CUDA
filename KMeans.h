#ifndef KMEANS_H_
#define KMEANS_H_
#include <vector>
#include <cstring>
#include <GL/freeglut.h>

namespace km{

	typedef struct Pos{
		GLfloat *x, *y, *z;
	} Pos;

	typedef struct Color{
		GLfloat *r, *g, *b;
	} Color;

	typedef struct Points{
		Pos pos;
		Color color;
		int *n;
	} Points;

	class KMeans{
	private:
		bool assignPoints(Points hv, Points hc);
		void moveCentroids(Points hc, Pos hsums);
		bool update(Points hv, Points hc, Pos hsums);
		static float rand_normal(float mean, float stddev);
		static void set1(Pos p, int *originalClass, int *v, int *oc, int *c);
		Points hv, hc, dv, dc, ov, oc;
		Pos hsums, dsums;
		int v, c, c2;
		bool converged;
		GLfloat hue2rgb(GLfloat p, GLfloat q, GLfloat t);
		void toRGB(GLfloat h, GLfloat s, GLfloat v, GLfloat*r, GLfloat*g, GLfloat*b);
		void getForgyCentroids();
		void allocateVertices();
		void allocateCentroids();
		void deleteVertices();
		void deleteCentroids();
		void init();

	public:
		KMeans();
		int getC() { return c; }
		void update();
		void draw();
		~KMeans();
	};

} // namespace km

#endif /* KMEANS_H_ */
