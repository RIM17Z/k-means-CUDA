
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include "UpdateStrategyCPU.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace KMeans {

	UpdateStrategyCPU::UpdateStrategyCPU(int _V, int _C, DataPoint *_vertices, DataPoint *_centroids, GLuint *_VBO, GLuint *_VBO2) : IUpdateStrategy(_V, _C, _vertices, _centroids, _VBO, _VBO2) {
		sums = new Pos[C];
		clusters_cnt = new int[C];
	}

	UpdateStrategyCPU::~UpdateStrategyCPU(){
		delete[] sums;
		delete[] clusters_cnt;
	}

	bool UpdateStrategyCPU::update(){
		bool converged = assignPoints();
		sumClusters();
		moveCentroids();
		return converged;
	}

	bool UpdateStrategyCPU::assignPoints(){
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

	void UpdateStrategyCPU::sumClusters(){
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
	}

	void UpdateStrategyCPU::moveCentroids(){
		for (int j = 0; j < C; ++j){
			if (clusters_cnt[j] != 0){
				centroids[j].pos.x = sums[j].x / (GLfloat)clusters_cnt[j];
				centroids[j].pos.y = sums[j].y / (GLfloat)clusters_cnt[j];
				centroids[j].pos.z = sums[j].z / (GLfloat)clusters_cnt[j];
			}
		}
	}

	void UpdateStrategyCPU::draw(){
		glPointSize(1);

		glBindBufferARB(GL_ARRAY_BUFFER, *VBO);
		glBufferDataARB(GL_ARRAY_BUFFER, V * 16, vertices,
			GL_DYNAMIC_DRAW);

		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 * sizeof(GLfloat)));
		glDrawArrays(GL_POINTS, 0, V);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPointSize(10);

		glBindBuffer(GL_ARRAY_BUFFER, *VBO2);
		glBufferDataARB(GL_ARRAY_BUFFER, C * 16, centroids,
			GL_DYNAMIC_DRAW);
		// Enable Vertex and Color arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Set the pointers to the vertices and colors
		glVertexPointer(3, GL_FLOAT, 16, 0);
		glColorPointer(3, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(3 * sizeof(GLfloat)));
		glDrawArrays(GL_POINTS, 0, C);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void UpdateStrategyCPU::resetCentroids(int C, DataPoint *_centroids){

	}
}