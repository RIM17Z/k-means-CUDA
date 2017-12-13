#include "UpdateStrategyCPU.h"
namespace KMeans {

	UpdateStrategyCPU::UpdateStrategyCPU() : IUpdateStrategy() { }

	UpdateStrategyCPU::~UpdateStrategyCPU(){ }

	bool UpdateStrategyCPU::update(int V, int C, DataPoint* vertices, DataPoint* centroids, Pos* sums, int* clusters_cnt){
		bool converged = assignPoints(V, C, vertices, centroids);
		memset(sums, 0, C * sizeof(Pos));
		for (int i = 0; i < C; ++i)
			clusters_cnt[i] = 0;

		for (int i = 0; i < V; ++i){
			//update distance sums and point counts for each group
			int id = vertices[i].cluster_id;
			sums[id].x += vertices[i].pos.x;
			sums[id].y += vertices[i].pos.y;
			sums[id].z += vertices[i].pos.z;
			++clusters_cnt[id];
		}

		moveCentroids(C, centroids, sums, clusters_cnt);
		return converged;
	}

	bool UpdateStrategyCPU::assignPoints(int V, int C, DataPoint* vertices, DataPoint* centroids){
		bool converged = true;
		for (int i = 0; i < V; ++i)
		{
			float distx = centroids[0].pos.x - vertices[i].pos.x;
			float disty = centroids[0].pos.y - vertices[i].pos.y;
			float distz = centroids[0].pos.z - vertices[i].pos.z;
			float distold = (distx * distx + disty * disty + distz * distz);
			int a = 0;
			for (int j = 1; j < C; ++j){
				float tmpx = centroids[j].pos.x - vertices[i].pos.x;
				float tmpy = centroids[j].pos.y - vertices[i].pos.y;
				float tmpz = centroids[j].pos.z - vertices[i].pos.z;
				float distnew = (tmpx * tmpx + tmpy * tmpy + tmpz * tmpz);
				if (distold > distnew){
					a = j;
					distold = distnew;
				}
			}
			if (vertices[i].cluster_id != a){
				vertices[i].cluster_id = a;
				vertices[i].r = centroids[a].r;
				vertices[i].g = centroids[a].g;
				vertices[i].b = centroids[a].b;
				converged = false;
			}
		}
		return converged;
	}


	void UpdateStrategyCPU::moveCentroids(int C, DataPoint* centroids, Pos* sums, int* clusters_cnt){
		for (int j = 0; j < C; ++j){
			if (clusters_cnt[j] != 0){
				centroids[j].pos.x = sums[j].x / (GLfloat)clusters_cnt[j];
				centroids[j].pos.y = sums[j].y / (GLfloat)clusters_cnt[j];
				centroids[j].pos.z = sums[j].z / (GLfloat)clusters_cnt[j];
			}
		}
	}
}