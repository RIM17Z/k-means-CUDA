#ifndef UPDATESTRATEGYCUDA_H_
#define UPDATESTRATEGYCUDA_H_
#include "vector_types.h"
#include "IUpdateStrategy.h"
#include "KMeansTypes.h"

namespace KMeans {

	class UpdateStrategyCUDA : public IUpdateStrategy{
	private:
		struct cudaGraphicsResource *cuda_vbo_resources[2];
		int *d_clusters_cnt;
		DataPoint *d_vertices, *d_centroids;
		Pos *sums, *d_sums;
		int *clusters_cnt;
		void allocateVerticesCuda();
		void allocateCentroidsCuda();
		void copyVerticesToCuda();
		void copyCentroidsToCuda();
		void deleteVertices();
		void deleteCentroids();
	public:
		UpdateStrategyCUDA(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids, GLuint *_VBO, GLuint *_VBO2);
		~UpdateStrategyCUDA();
		const char* getStrategyName() { return "CUDA"; };
		void resetCentroids(int _C, DataPoint *_centroids);
		void resetVertices(int V, DataPoint *_vertices);
		bool update();
		void draw();
	};

	extern "C" bool assignPoints(DataPoint* d_vertices, DataPoint* d_centroids, int V, int C);
	extern "C" void sumClusters(DataPoint* d_vertices, Pos* d_sums, int* d_clusters_cnt, int V, int C);
	extern "C" void moveCentroids(DataPoint* d_centroids, Pos* d_sums, int* d_clusters_cnt, int C);

} // namespace KMeans
#endif
