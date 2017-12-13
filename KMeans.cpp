#include "KMeans.h"
#include <time.h>
#include <cmath>
#ifdef __NVCC__
#include "UpdateStrategyGPU.h"
#endif
#include "UpdateStrategyCPU.h"

namespace KMeans {
	const int NUM_CLUSTERS = 4;

	float KMeans::rand_normal(float mean, float stddev) {
		static float n2 = 0.0;
		static int n2_cached = 0;
		if (!n2_cached) {
			double x, y, r;
			do {
				x = 2.0*rand() / RAND_MAX - 1;
				y = 2.0*rand() / RAND_MAX - 1;

				r = x*x + y*y;
			} while (r == 0.0 || r > 1.0);
			{
				float d = (float)sqrt(-2.0*log(r) / r);
				float n1 = (float)x*d;
				n2 = (float)y*d;
				float result = n1*stddev + mean;
				n2_cached = 1;
				return result;
			}
		}
		else {
			n2_cached = 0;
			return n2*stddev + mean;
		}
	}

	void KMeans::generate_set() {
		srand((unsigned int)time(0));
		float cx, cy, cz;
		int current_cluster = -1;
		original_C = NUM_CLUSTERS;
		for (int i = 0; i < V; ++i) {
			if (i % (V / NUM_CLUSTERS) == 0) {
				cx = (float)(rand() % 1000) / 1000;
				cy = (float)(rand() % 1000) / 1000;
				cz = (float)(rand() % 1000) / 1000;
				current_cluster++;
			}
			vertices[i].pos.x = cx + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
			vertices[i].pos.y = cy + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
			vertices[i].pos.z = cz + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
			original_vertices[i].cluster_id = current_cluster;
		}
	}

	KMeans::KMeans() {
		V = 1000000;
		C = NUM_CLUSTERS;
		allocateVertices();
		generate_set();
		init();
	}


	const char* KMeans::getStrategyName() {
		return strategies[currentStrategyId]->getStrategyName();
	}

	void KMeans::allocateVertices() {
		// vertices
		vertices = new DataPoint[V];
		// vertex assignment
		original_vertices = new DataPoint[V];
	}

	void KMeans::allocateCentroids() {
		// centroids
		centroids = new DataPoint[C];
		sums = new Pos[C];
		clusters_cnt = new int[C];
	}

	void KMeans::getForgyCentroids() {
		srand((unsigned int)time(0));
		for (int i = 0; i < C; i++) {
			int index = rand() % V;
			centroids[i].pos.x = vertices[index].pos.x;
			centroids[i].pos.y = vertices[index].pos.y;
			centroids[i].pos.z = vertices[index].pos.z;
			GLfloat h = (float)(i) / (float)(C);
			toRGB(h, 1, 0.5, &centroids[i].r, &centroids[i].g, &centroids[i].b);
		}
	}

	void KMeans::init() {
		converged = false;
		allocateCentroids();
		for (int i = 0; i < V; ++i)
			vertices[i].cluster_id = 255;
		getForgyCentroids();
		strategies.push_back(new UpdateStrategyCPU());
#ifdef __NVCC__
		strategies.push_back(new UpdateStrategyGPU(v, c, hv, hc, hsums));
#endif
		currentStrategyId = 0;
	}

	void KMeans::deleteVertices() {
		delete[] vertices;
		delete[] original_vertices;
	}

	void KMeans::deleteCentroids() {
		delete[] centroids;
		delete[] sums;
		delete[] clusters_cnt;
	}

	KMeans::~KMeans() {
		deleteCentroids();
		deleteVertices();
		for (int i = 0; i < strategies.size(); ++i)
			delete strategies[i];
	}


	bool KMeans::update(){
		if (converged)
			return true;
		converged = strategies[currentStrategyId]->update(V, C, vertices, centroids, sums, clusters_cnt);
		return converged;
	}

	//color space conversion

	GLfloat KMeans::hue2rgb(GLfloat p, GLfloat q, GLfloat t) {
		if (t < 0) t += 1.;
		if (t > 1) t -= 1.;
		if (t < 1. / 6.) return p + (q - p) * 6.f * t;
		if (t < 1. / 2.) return q;
		if (t < 2. / 3.) return p + (q - p) * (2.f / 3.f - t) * 6;
		return p;
	}

	void KMeans::toRGB(GLfloat h, GLfloat s, GLfloat v,
		GLubyte *r, GLubyte*g, GLubyte*b) {
		GLfloat r_ = 0, g_ = 0, b_ = 0;
		if (s == 0) {
			r_ = g_ = b_ = v;  // achromatic
		}
		else {
			GLfloat q = v < 0.5 ? v * (1 + s) : v + s - v * s;
			GLfloat p = 2 * v - q;
			r_ = hue2rgb(p, q, h + 1.f / 3.f);
			g_ = hue2rgb(p, q, h);
			b_ = hue2rgb(p, q, h - 1.f / 3.f);
		}
		*r = r_ * 255;
		*g = g_ * 255;
		*b = b_ * 255;
	}

}  // namespace km
