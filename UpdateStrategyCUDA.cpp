
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
// includes, cuda
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
// CUDA helper functions
#include <helper_cuda.h>         // helper functions for CUDA error check
#include <helper_cuda_gl.h>      // helper functions for CUDA/GL interop
#include "UpdateStrategyCUDA.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace KMeans {

	UpdateStrategyCUDA::UpdateStrategyCUDA(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids, GLuint *_VBO, GLuint *_VBO2) : IUpdateStrategy(_V, _C, _vertices, _centroids, _VBO, _VBO2) {
		sums = new Pos[C];
		clusters_cnt = new int[C];
		checkCudaErrors(cudaSetDevice(0));

		allocateVerticesCuda();
		allocateCentroidsCuda();

		copyVerticesToCuda();
		copyCentroidsToCuda();

	}

	UpdateStrategyCUDA::~UpdateStrategyCUDA(){
		delete[] sums;
		delete[] clusters_cnt;
		checkCudaErrors(cudaDeviceSynchronize());
		deleteCentroids();
		deleteVertices();
	}

	void UpdateStrategyCUDA::allocateVerticesCuda(){
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, V * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// register this buffer object with CUDA
		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[0], *VBO, cudaGraphicsRegisterFlagsNone));

	} 
	
	void UpdateStrategyCUDA::allocateCentroidsCuda(){
		checkCudaErrors(cudaMalloc((void**)&d_sums, V * sizeof(float4)));
		checkCudaErrors(cudaMalloc((void**)&d_clusters_cnt, V * sizeof(int)));
		glBindBuffer(GL_ARRAY_BUFFER, *VBO2);
		glBufferData(GL_ARRAY_BUFFER, MAX_CLUSTERS_CNT * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// register this buffer object with CUDA
		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[1], *VBO2, cudaGraphicsRegisterFlagsNone));
	}

	void UpdateStrategyCUDA::copyVerticesToCuda(){
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaMemcpy(
			d_vertices, vertices, V * sizeof(DataPoint), cudaMemcpyHostToDevice));
		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));

	}

	void UpdateStrategyCUDA::copyCentroidsToCuda(){
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		checkCudaErrors(cudaMemset(d_centroids, 0x0, MAX_CLUSTERS_CNT * 4 * sizeof(float)));
		checkCudaErrors(cudaMemcpy(
			d_centroids, centroids, C * sizeof(DataPoint), cudaMemcpyHostToDevice));
		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));
	}

	void UpdateStrategyCUDA::deleteVertices(){
		cudaDeviceSynchronize();
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[0]));
	}

	void UpdateStrategyCUDA::deleteCentroids(){
		cudaDeviceSynchronize();
		cudaFree(d_sums);
		cudaFree(d_clusters_cnt);
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[1]));
	}

	bool UpdateStrategyCUDA::update(){
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		bool converged = assignPoints(d_vertices, d_centroids, V, C, 512, 512);
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaMemset(d_sums, 0x0, C * sizeof(Pos)));
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaMemset(d_clusters_cnt, 0x0, C * sizeof(int)));
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaDeviceSynchronize());
		
		
		sumClusters(d_vertices, d_sums, d_clusters_cnt, V, C, 512, 512);
		//--this part runs on CPU
		/*
		checkCudaErrors(cudaMemcpy(
			vertices, d_vertices, V * sizeof(DataPoint), cudaMemcpyDeviceToHost));
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
		checkCudaErrors(cudaMemcpy(
			d_sums, sums, C * sizeof(DataPoint), cudaMemcpyHostToDevice));
		checkCudaErrors(cudaMemcpy(
			d_clusters_cnt, clusters_cnt, C * sizeof(DataPoint), cudaMemcpyHostToDevice));
		*/
		//--

		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaDeviceSynchronize());
		moveCentroids(d_centroids, d_sums, d_clusters_cnt, C, 256, 256);
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaDeviceSynchronize());
		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));
		return converged;
	}

	void UpdateStrategyCUDA::draw(){
		glPointSize(1);

		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 * sizeof(GLfloat)));

		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDrawArrays(GL_POINTS, 0, V);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPointSize(10);

		glBindBuffer(GL_ARRAY_BUFFER, *VBO2);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 * sizeof(GLfloat)));

		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDrawArrays(GL_POINTS, 0, C);
		glDisableClientState(GL_VERTEX_ARRAY);
	}


	void UpdateStrategyCUDA::resetCentroids(int _C, DataPoint *_centroids){
		deleteCentroids();
		C = _C;
		centroids = _centroids;
		sums = new Pos[C];
		clusters_cnt = new int[C];

		allocateCentroidsCuda();
		copyCentroidsToCuda();
		copyVerticesToCuda();

	}

	void UpdateStrategyCUDA::resetVertices(int _V, DataPoint *_vertices){
		deleteVertices();
		V = _V;
		vertices = _vertices;

		allocateVerticesCuda();
		copyCentroidsToCuda();
		copyVerticesToCuda();
	}
}