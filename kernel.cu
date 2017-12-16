#ifndef _KERNEL_H_
#define _KERNEL_H_
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "vector_types.h"
#include <float.h>
#include <stdio.h>
#include "KMeansTypes.h"

__device__ bool d_converged;

__global__ void assignKernel(float4* d_vertices, float4* d_centroids, int V, int C)
{
	float distold = FLT_MAX;
	__shared__ float4 s_centroids[512];
	float4 p;
	unsigned int a = 0;
	unsigned int j;

	unsigned int idx = (blockIdx.x * blockDim.x + threadIdx.x);

	if (idx == 0)
		d_converged = true;
	
	//TODO allocate d_centroids of size blockDim.x and fill excess with zeros
	// then remove if clause
	if (threadIdx.x < C)
		s_centroids[threadIdx.x] = d_centroids[threadIdx.x];
	else
		s_centroids[threadIdx.x] = make_float4(0.0, 0.0, 0.0, 0.0);

	// TODO same as for centroids
	if (idx < V)
		p = d_vertices[idx];
	else
		p = make_float4(0.0, 0.0, 0.0, 0.0);

	__syncthreads();

	if (idx < V){
		for (j = 0; j < C; j++){
			float tmp_x = s_centroids[j].x - p.x;
			float tmp_y = s_centroids[j].y - p.y;
			float tmp_z = s_centroids[j].z - p.z;
			float distnew = (tmp_x*tmp_x + tmp_y*tmp_y + tmp_z*tmp_z);
			if (distold > distnew){
				a = j;
				distold = distnew;
			}
		}
		__syncthreads();
		if (*((char*)&(p.w) + 3) != *((char*)&(s_centroids[j].w) + 3))
			d_converged = false;
		d_vertices[idx].w = s_centroids[a].w;
	}

}

__global__ void sumClustersKernel(float4* d_vertices, float3* d_sums, int* d_clusters_cnt, int V, int C){

	unsigned int idx = (blockIdx.x * blockDim.x + threadIdx.x);
	if (idx < V){
		float4 vertex = d_vertices[idx];
		unsigned int id = *((char*)&(vertex.w) + 3);
		__syncthreads();
		atomicAdd(&(d_sums[id].x), vertex.x);
		atomicAdd(&(d_sums[id].y), vertex.y);
		atomicAdd(&(d_sums[id].z), vertex.z);
		atomicAdd(&d_clusters_cnt[id], 1);
	}
}

__global__ void moveCentroidsKernel(float4* d_centroids, float3* d_sums, int* d_clusters_cnt, int C)
{
	unsigned int idx = (blockIdx.x*blockDim.x + threadIdx.x);
	
	if (idx < C){
		int cnt = d_clusters_cnt[idx];
		if (cnt > 0){
			d_centroids[idx].x = d_sums[idx].x / cnt;
			d_centroids[idx].y = d_sums[idx].y / cnt;
			d_centroids[idx].z = d_sums[idx].z / cnt;

		}
	}
}

extern "C" bool assignPoints(KMeans::DataPoint* d_vertices, KMeans::DataPoint* d_centroids, int V, int C)
{
	bool converged = false;
	assignKernel <<< (V + 511) / 512, 512 >>>((float4*)d_vertices, (float4*)d_centroids, V, C);
	//cudaMemcpyFromSymbol(&converged, (void*) d_converged, sizeof(converged), 0, cudaMemcpyDeviceToHost);
	return converged;
}

extern "C" void sumClusters(KMeans::DataPoint* d_vertices, KMeans::Pos* d_sums, int* d_clusters_cnt, int V, int C){
	sumClustersKernel << < (V + 511) / 512, 512 >> >((float4*)d_vertices, (float3*)d_sums, d_clusters_cnt, V, C);
}

extern "C" void moveCentroids(KMeans::DataPoint* d_centroids, KMeans::Pos* d_sums, int* d_clusters_cnt, int C)
{
	moveCentroidsKernel << < (C + 255) / 256, 256 >> >((float4*)d_centroids, (float3*)d_sums, d_clusters_cnt, C);
}
#endif // #ifndef _KERNEL_H_