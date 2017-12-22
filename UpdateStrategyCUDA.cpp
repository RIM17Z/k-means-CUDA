
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

		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, V * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, *VBO2);
		glBufferData(GL_ARRAY_BUFFER, C * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		allocateVerticesCuda();
		allocateCentroidsCuda();

		// register this buffer object with CUDA
		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[0], *VBO, cudaGraphicsRegisterFlagsNone));
		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[1], *VBO2, cudaGraphicsRegisterFlagsNone));

		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		copyVerticesToCuda();
		copyCentroidsToCuda();

		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));
	}

	UpdateStrategyCUDA::~UpdateStrategyCUDA(){
		delete[] sums;
		delete[] clusters_cnt;
		checkCudaErrors(cudaDeviceSynchronize());
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[1]));
		deleteCentroids();
		deleteVertices();
	}

	void UpdateStrategyCUDA::allocateVerticesCuda(){
		//checkCudaErrors(cudaMalloc((void**)&d_vertices, V * sizeof(DataPoint)));
	} 
	
	void UpdateStrategyCUDA::allocateCentroidsCuda(){
		//checkCudaErrors(cudaMalloc((void**)&d_centroids, C * sizeof(DataPoint)));
		checkCudaErrors(cudaMalloc((void**)&d_sums, V * sizeof(float4)));
		checkCudaErrors(cudaMalloc((void**)&d_clusters_cnt, V * sizeof(int)));
	}

	void UpdateStrategyCUDA::copyVerticesToCuda(){
		checkCudaErrors(cudaMemcpy(
			d_vertices, vertices, V * sizeof(DataPoint), cudaMemcpyHostToDevice));

	}

	void UpdateStrategyCUDA::copyCentroidsToCuda(){
		checkCudaErrors(cudaMemcpy(
			d_centroids, centroids, C * sizeof(DataPoint), cudaMemcpyHostToDevice));
	}

	void UpdateStrategyCUDA::deleteVertices(){
		cudaDeviceSynchronize();
		//cudaFree(d_vertices);
	}

	void UpdateStrategyCUDA::deleteCentroids(){
		cudaDeviceSynchronize();
		//cudaFree(d_centroids);
		cudaFree(d_sums);
		cudaFree(d_clusters_cnt);
	}

	bool UpdateStrategyCUDA::update(){
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		bool converged = assignPoints(d_vertices, d_centroids, V, C);
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaMemset(d_sums, 0x0, C * sizeof(Pos)));
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaMemset(d_clusters_cnt, 0x0, C * sizeof(int)));
		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaDeviceSynchronize());
		
		
		//sumClusters(d_vertices, d_sums, d_clusters_cnt, V, C, d_sum_id, d_keys);
		//--this part runs on CPU
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
		//--

		checkCudaErrors(cudaGetLastError());
		checkCudaErrors(cudaDeviceSynchronize());
		moveCentroids(d_centroids, d_sums, d_clusters_cnt, C);
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
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[1]));
		C = _C;
		centroids = _centroids;
		sums = new Pos[C];
		clusters_cnt = new int[C];

		glBindBuffer(GL_ARRAY_BUFFER, *VBO2);
		glBufferData(GL_ARRAY_BUFFER, C * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		allocateCentroidsCuda();
		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[1], *VBO2, cudaGraphicsRegisterFlagsNone));

		// register this buffer object with CUDA
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		copyCentroidsToCuda();
		copyVerticesToCuda();
		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));

	}

	void UpdateStrategyCUDA::resetVertices(int _V, DataPoint *_vertices){
		checkCudaErrors(cudaGraphicsUnregisterResource(cuda_vbo_resources[0]));
		V = _V;
		vertices = _vertices;

		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, V * 4 * sizeof(float), 0,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_vbo_resources[0], *VBO, cudaGraphicsRegisterFlagsNone));

		// register this buffer object with CUDA
		checkCudaErrors(cudaGraphicsMapResources(2, cuda_vbo_resources, 0));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_vertices, NULL,
			cuda_vbo_resources[0]));
		checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_centroids, NULL,
			cuda_vbo_resources[1]));
		copyCentroidsToCuda();
		copyVerticesToCuda();
		checkCudaErrors(cudaGraphicsUnmapResources(2, cuda_vbo_resources, 0));

	}
}