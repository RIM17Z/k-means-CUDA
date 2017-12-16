#ifndef KMEANS_H_
#define KMEANS_H_
#include <GL/freeglut.h>
#include <vector>
#include "KMeansTypes.h"
#include "IUpdateStrategy.h"

namespace KMeans {
	class KMeans{
	private:
		int V, C, original_C, currentStrategyId;
		bool converged;
		DataPoint *vertices, *centroids, *original_vertices, *original_centroids;

		std::vector<IUpdateStrategy*> strategies;

		static float rand_normal(float mean, float stddev);
		void generate_set();
		void allocateVertices();
		void allocateCentroids();
		void getForgyCentroids();
		void init(GLuint *VBOS);
		void deleteVertices();
		void deleteCentroids();
		void toRGB(GLfloat h, GLfloat s, GLfloat v, GLubyte*r, GLubyte*g, GLubyte*b);
		GLfloat hue2rgb(GLfloat p, GLfloat q, GLfloat t);

	public:
		KMeans(GLuint* VBOS);
		int getV(){ return V; };
		int getC(){	return C; };
		bool isConverged(){ return converged; };
		bool update();
		void draw();
		const char* getStrategyName();
		~KMeans();
	};

} // namespace KMeans

#endif /* KMEANS_H_ */
