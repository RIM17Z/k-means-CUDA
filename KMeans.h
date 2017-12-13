#ifndef KMEANS_H_
#define KMEANS_H_
#include <GL/freeglut.h>
#include "KMeansTypes.h"

namespace KMeans {
	class KMeans{
	private:
		Pos *sums, *d_sums;
		int *clusters_cnt, *d_clusters_cnt;
		int V, C, original_C;
		bool converged;
		DataPoint *vertices, *centroids, *d_vertices, *d_centroids, *original_vertices, *original_centroids;

		static float rand_normal(float mean, float stddev);
		void generate_set();
		void allocateVertices();
		void allocateCentroids();
		void getForgyCentroids();
		void init();
		void deleteVertices();
		void deleteCentroids();
		bool assignPoints();
		void moveCentroids();
		void toRGB(GLfloat h, GLfloat s, GLfloat v, GLubyte*r, GLubyte*g, GLubyte*b);
		GLfloat hue2rgb(GLfloat p, GLfloat q, GLfloat t);

	public:
		KMeans();
		DataPoint* getVertices() { return vertices; };
		int getV(){ return V; };
		int getC(){	return C; };
		bool isConverged(){ return converged; };
		bool update();
		~KMeans();
	};

} // namespace KMeans

#endif /* KMEANS_H_ */
