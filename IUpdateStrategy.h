#ifndef IUPDATESTRATEGY_H_
#define IUPDATESTRATEGY_H_
#include "KMeansTypes.h"

namespace KMeans {

	class IUpdateStrategy{
	protected:
		IUpdateStrategy(){};
	public:
		virtual const char* getStrategyName() = 0;
		virtual bool update(int V, int C, DataPoint* hv, DataPoint* hc, Pos* sums, int* clusters_cnt) = 0;
	};

} // namespace KMeans
#endif
