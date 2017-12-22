#ifndef IUPDATESTRATEGY_H_
#define IUPDATESTRATEGY_H_
#include "KMeansTypes.h"

namespace KMeans {

	class IUpdateStrategy{
	protected:
		int V, C;
		const GLuint *VBO, *VBO2;
		DataPoint *vertices, *centroids;
		IUpdateStrategy(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids, GLuint *_VBO, GLuint *_VBO2) : V(_V), C(_C), VBO(_VBO), VBO2(_VBO2), vertices(_vertices), centroids(_centroids) {};
	public:
		virtual const char* getStrategyName() = 0;
		virtual bool update() = 0;
		virtual void draw() = 0;
		virtual void resetCentroids(int C, DataPoint *_centroids) = 0;
		virtual void resetVertices(int V, DataPoint *_vertices) = 0;
	};

} // namespace KMeans
#endif
