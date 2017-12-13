#ifndef UPDATESTRATEGYCPU_H_
#define UPDATESTRATEGYCPU_H_
#include "IUpdateStrategy.h"
#include "KMeansTypes.h"

namespace KMeans {

	class UpdateStrategyCPU : public IUpdateStrategy{
	private:
		bool assignPoints(int V, int C, DataPoint* vertices, DataPoint* centroids);
		void moveCentroids(int C, DataPoint* centroids, Pos* sums, int* clusters_cnt);
	public:
		UpdateStrategyCPU();
		~UpdateStrategyCPU();
		const char* getStrategyName() { return "CPU"; };
		bool update(int V, int C, DataPoint* vertices, DataPoint* centroids, Pos* sums, int* clusters_cnt);
	};

} // namespace KMeans
#endif
