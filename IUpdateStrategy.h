#ifndef IUPDATESTRATEGY_H_
#define IUPDATESTRATEGY_H_
#include "KMeansTypes.h"

namespace KMeans {

	class IUpdateStrategy{
	protected:
		const int V, C;
		DataPoint *vertices, *centroids;
		IUpdateStrategy(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids) : V(_V), C(_C), vertices(_vertices), centroids(_centroids) {};
	public:
		virtual const char* getStrategyName() = 0;
		virtual bool update() = 0;
		virtual const DataPoint* getVertices() { return vertices; };
	};

} // namespace KMeans
#endif
