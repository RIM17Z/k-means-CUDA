#ifndef UPDATESTRATEGYCPU_H_
#define UPDATESTRATEGYCPU_H_
#include "IUpdateStrategy.h"
#include "KMeansTypes.h"

namespace KMeans {

	class UpdateStrategyCPU : public IUpdateStrategy{
	private:
		Pos* sums;
		int *clusters_cnt;
		bool assignPoints();
		void sumClusters();
		void moveCentroids();
	public:
		UpdateStrategyCPU(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids);
		~UpdateStrategyCPU();
		const char* getStrategyName() { return "CPU"; };
		bool update();
	};

} // namespace KMeans
#endif
