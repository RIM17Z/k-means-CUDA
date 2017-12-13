#include "KMeans.h"
#include <string>
#include <vector>
#include <time.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
namespace km {
	const int CLUSTERS_CNT = 4;

	bool KMeans::update(Points hv, Points hc, Pos hsums){
		bool converged = assignPoints(hv, hc);
		memset(hsums.x, 0, c * sizeof(float));
		memset(hsums.y, 0, c * sizeof(float));
		memset(hsums.z, 0, c * sizeof(float));
		memset(hc.n, 0, c * sizeof(int));

		for (int i = 0; i < v; ++i){
			//update distance sums and point counts for each group
			int a = hv.n[i];
			hsums.x[a] += hv.pos.x[i];
			hsums.y[a] += hv.pos.y[i];
			hsums.z[a] += hv.pos.z[i];
			++hc.n[a];
		}

		moveCentroids(hc, hsums);
		return converged;
	}

	bool KMeans::assignPoints(Points hv, Points hc){
		bool converged = true;
		for (int i = 0; i < v; ++i)
		{
			float distx = hc.pos.x[0] - hv.pos.x[i];
			float disty = hc.pos.y[0] - hv.pos.y[i];
			float distz = hc.pos.z[0] - hv.pos.z[i];
			float distold = (distx * distx + disty * disty + distz * distz);
			int a = 0;
			for (int j = 1; j < c; ++j){
				float tmpx = hc.pos.x[j] - hv.pos.x[i];
				float tmpy = hc.pos.y[j] - hv.pos.y[i];
				float tmpz = hc.pos.z[j] - hv.pos.z[i];
				float distnew = (tmpx * tmpx + tmpy * tmpy + tmpz * tmpz);
				if (distold > distnew){
					a = j;
					distold = distnew;
				}
			}
			if (hv.n[i] != a){
				hv.n[i] = a;
				hv.color.r[i] = hc.color.r[a];
				hv.color.g[i] = hc.color.g[a];
				hv.color.b[i] = hc.color.b[a];
				converged = false;
			}
		}
		return converged;
	}

	void KMeans::moveCentroids(Points hc, Pos hsums){
		for (int j = 0; j < c; ++j){
			if (hc.n[j] != 0){
				hc.pos.x[j] = hsums.x[j] / hc.n[j];
				hc.pos.y[j] = hsums.y[j] / hc.n[j];
				hc.pos.z[j] = hsums.z[j] / hc.n[j];
			}
		}
	}


  float KMeans::rand_normal(float mean, float stddev) {
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
      double x, y, r;
      do {
        x = 2.0*rand()/RAND_MAX - 1;
        y = 2.0*rand()/RAND_MAX - 1;

        r = x*x + y*y;
      } while (r == 0.0 || r > 1.0);
      {
        float d = sqrt(-2.0*log(r)/r);
        float n1 = x*d;
        n2 = y*d;
        float result = n1*stddev + mean;
        n2_cached = 1;
        return result;
      }
    } else {
      n2_cached = 0;
      return n2*stddev + mean;
    }
  }

  void KMeans::set1(Pos p, int *n, int *v, int *c2, int *c) {
    srand((unsigned int)time(0));
    float cx, cy, cz;
    int currentClass = -1;
	*c2 = CLUSTERS_CNT;
    for (int i = 0; i < *v; ++i) {
      if (i % (*v / 4) == 0) {
        cx = (float)(rand()%1000)/1000;
        cy = (float)(rand()%1000)/1000;
        cz = (float)(rand()%1000)/1000;
        currentClass++;
      }
      p.x[i] = cx + rand_normal(.0, .1 + (float)(rand() % 100) / 1000);
      p.y[i] = cy + rand_normal(.0, .1 + (float)(rand() % 100) / 1000);
      p.z[i] = cz + rand_normal(.0, .1 + (float)(rand() % 100) / 1000);
      n[i] = currentClass;
    }
  }

  KMeans::KMeans() {
    v = 1000000;
    c = 4;
    allocateVertices();
    set1(hv.pos, ov.n, &v, &c2, &c);
    init();
  }


  void KMeans::allocateVertices() {
    // vertices
    hv.pos.x = new GLfloat[v];
    hv.pos.y = new GLfloat[v];
    hv.pos.z = new GLfloat[v];
    hv.color.r = new GLfloat[v];
    hv.color.g = new GLfloat[v];
    hv.color.b = new GLfloat[v];
    // vertex assignment
    hv.n = new int[v];
    ov.n = new int[v];
    ov.color.r = new GLfloat[v];
    ov.color.g = new GLfloat[v];
    ov.color.b = new GLfloat[v];
  }

  void KMeans::allocateCentroids() {
    // centroids
    hc.pos.x = new GLfloat[c];
    hc.pos.y = new GLfloat[c];
    hc.pos.z = new GLfloat[c];
    hc.color.r = new GLfloat[c];
    hc.color.g = new GLfloat[c];
    hc.color.b = new GLfloat[c];
    hsums.x = new float[c];
    hsums.y = new float[c];
    hsums.z = new float[c];
    hc.n = new int[c];
  }

  void KMeans::init() {
    converged = false;
    allocateCentroids();
    for (int i = 0; i < v; ++i)
      hv.n[i] = -1;
    getForgyCentroids();
  }

  void KMeans::deleteVertices() {
    delete [] hv.pos.x;
    delete [] hv.pos.y;
    delete [] hv.pos.z;
    delete [] hv.color.r;
    delete [] hv.color.g;
    delete [] hv.color.b;
    delete [] hv.n;
    delete [] ov.color.r;
    delete [] ov.color.g;
    delete [] ov.color.b;
    delete [] ov.n;
  }

  void KMeans::deleteCentroids() {
    delete [] hc.pos.x;
    delete [] hc.pos.y;
    delete [] hc.pos.z;
    delete [] hc.color.r;
    delete [] hc.color.g;
    delete [] hc.color.b;
    delete [] hsums.x;
    delete [] hsums.y;
    delete [] hsums.z;
    delete [] hc.n;
  }

  KMeans::~KMeans() {
    deleteCentroids();
    deleteVertices();
  }

  void KMeans::update() {
    if (converged)
      return;
    converged = update(hv, hc, hsums);
  }

  void KMeans::getForgyCentroids() {
    srand((unsigned int)time(0));
    for (int i = 0; i < c; i++) {
      int index = rand() % v;
      hc.pos.x[i] = hv.pos.x[index];
      hc.pos.y[i] = hv.pos.y[index];
      hc.pos.z[i] = hv.pos.z[index];
      GLfloat h = (float)(i) / (float)(c);
      toRGB(h, 1, 0.5, &hc.color.r[i], &hc.color.g[i], &hc.color.b[i]);
    }
  }

  //color space conversion

  GLfloat KMeans::hue2rgb(GLfloat p, GLfloat q, GLfloat t) {
    if (t < 0) t += 1.;
    if (t > 1) t -= 1.;
    if (t < 1./6.) return p + (q - p) * 6. * t;
    if (t < 1./2.) return q;
    if (t < 2./3.) return p + (q - p) * (2./3. - t) * 6;
    return p;
  }

  void KMeans::toRGB(GLfloat h, GLfloat s, GLfloat v,
                     GLfloat*r, GLfloat*g, GLfloat*b) {
    GLfloat r_ = 0, g_ = 0, b_ = 0;
    if (s == 0) {
      r_ = g_ = b_ = v;  // achromatic
    } else {
      GLfloat q = v < 0.5 ? v * (1 + s) : v + s - v * s;
      GLfloat p = 2 * v - q;
      r_ = hue2rgb(p, q, h + 1./3.);
      g_ = hue2rgb(p, q, h);
      b_ = hue2rgb(p, q, h - 1./3.);
    }
    *r = r_;
    *g = g_;
    *b = b_;
  }

  void KMeans::draw() {
	  //draw points
      glBegin(GL_POINTS);
      for (int i = 0; i < v; ++i) {
        glColor3f(hv.color.r[i], hv.color.g[i], hv.color.b[i]);
        glVertex3f(hv.pos.x[i], hv.pos.y[i], hv.pos.z[i]);
      }
      glEnd();

	  //draw centroids
      glPointSize(10);
      glBegin(GL_POINTS);
      for (int i = 0; i < c; ++i) {
        glColor3f(hc.color.r[i], hc.color.g[i], hc.color.b[i]);
        glVertex3f(hc.pos.x[i], hc.pos.y[i], hc.pos.z[i]);
      }
      glEnd();
  }

}  // namespace km
