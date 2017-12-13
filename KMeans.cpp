#include "KMeans.h"
#include <time.h>
#include <cmath>
namespace km {

	const int CLUSTERS_CNT = 4;

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
        float d = (float)sqrt(-2.0*log(r)/r);
        float n1 = (float)x*d;
        n2 = (float)y*d;
        float result = n1*stddev + mean;
        n2_cached = 1;
        return result;
      }
    } else {
      n2_cached = 0;
      return n2*stddev + mean;
    }
  }

  void KMeans::set(DataPoint *hv, DataPoint *ov, int *v, int *c2, int *c) {
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
      hv[i].pos.x = cx + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
      hv[i].pos.y = cy + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
      hv[i].pos.z = cz + rand_normal(.0f, .1f + (float)(rand() % 100) / 1000);
      ov[i].n = currentClass;
    }
  }

  KMeans::KMeans() {
    v = 1000000;
    c = 4;
    allocateVertices();
    set(hv, ov, &v, &c2, &c);
    init();
  }


  void KMeans::allocateVertices() {
    // vertices
    hv = new DataPoint[v];
    // vertex assignment
	ov = new DataPoint[v];
  }

  void KMeans::allocateCentroids() {
    // centroids
    hc = new DataPoint[c];
    hsums = new Pos[c];
	hccnt = new int[c];
  }

  void KMeans::getForgyCentroids() {
	  srand((unsigned int)time(0));
	  for (int i = 0; i < c; i++) {
		  int index = rand() % v;
		  hc[i].pos.x = hv[index].pos.x;
		  hc[i].pos.y = hv[index].pos.y;
		  hc[i].pos.z = hv[index].pos.z;
		  GLfloat h = (float)(i) / (float)(c);
		  toRGB(h, 1, 0.5, &hc[i].r, &hc[i].g, &hc[i].b);
	  }
  }

  void KMeans::init() {
    converged = false;
    allocateCentroids();
    for (int i = 0; i < v; ++i)
      hv[i].n = 255;
    getForgyCentroids();
  }

  void KMeans::deleteVertices() {
    delete [] hv;
    delete [] ov;
  }

  void KMeans::deleteCentroids() {
    delete [] hc;
	delete [] hsums;
	delete [] hccnt;
  }

  KMeans::~KMeans() {
    deleteCentroids();
    deleteVertices();
  }

  void KMeans::update() {
    if (converged)
      return;
    converged = update(hv, hc, hccnt, hsums);
  }

  bool KMeans::update(DataPoint *hv, DataPoint *hc, int *hccnt, Pos *hsums){
	  bool converged = assignPoints(hv, hc);
	  memset(hsums, 0, c * sizeof(Pos));
	  for (int i = 0; i < c; ++i)
		  hccnt[i] = 0;
	  
	  for (int i = 0; i < v; ++i){
		  //update distance sums and point counts for each group
		  int a = hv[i].n;
		  hsums[a].x += hv[i].pos.x;
		  hsums[a].y += hv[i].pos.y;
		  hsums[a].z += hv[i].pos.z;
		  ++hccnt[a];
	  }

	  moveCentroids(hc, hccnt, hsums);
	  return converged;
  }

  bool KMeans::assignPoints(DataPoint *hv, DataPoint *hc){
	  bool converged = true;
	  for (int i = 0; i < v; ++i)
	  {
		  float distx = hc[0].pos.x - hv[i].pos.x;
		  float disty = hc[0].pos.y - hv[i].pos.y;
		  float distz = hc[0].pos.z - hv[i].pos.z;
		  float distold = (distx * distx + disty * disty + distz * distz);
		  int a = 0;
		  for (int j = 1; j < c; ++j){
			  float tmpx = hc[j].pos.x - hv[i].pos.x;
			  float tmpy = hc[j].pos.y - hv[i].pos.y;
			  float tmpz = hc[j].pos.z - hv[i].pos.z;
			  float distnew = (tmpx * tmpx + tmpy * tmpy + tmpz * tmpz);
			  if (distold > distnew){
				  a = j;
				  distold = distnew;
			  }
		  }
		  if (hv[i].n != a){
			  hv[i].n = a;
			  hv[i].r = hc[a].r;
			  hv[i].g = hc[a].g;
			  hv[i].b = hc[a].b;
			  converged = false;
		  }
	  }
	  return converged;
  }

  void KMeans::moveCentroids(DataPoint *hc, int *hccnt, Pos *hsums){
	  for (int j = 0; j < c; ++j){
		  if (hccnt[j] != 0){
			  hc[j].pos.x = hsums[j].x / (GLfloat)hccnt[j];
			  hc[j].pos.y = hsums[j].y / (GLfloat)hccnt[j];
			  hc[j].pos.z = hsums[j].z / (GLfloat)hccnt[j];
		  }
	  }
  }

  //color space conversion

  GLfloat KMeans::hue2rgb(GLfloat p, GLfloat q, GLfloat t) {
    if (t < 0) t += 1.;
    if (t > 1) t -= 1.;
    if (t < 1./6.) return p + (q - p) * 6.f * t;
    if (t < 1./2.) return q;
    if (t < 2./3.) return p + (q - p) * (2.f/3.f - t) * 6;
    return p;
  }

  void KMeans::toRGB(GLfloat h, GLfloat s, GLfloat v,
	  GLubyte *r, GLubyte*g, GLubyte*b) {
    GLfloat r_ = 0, g_ = 0, b_ = 0;
    if (s == 0) {
      r_ = g_ = b_ = v;  // achromatic
    } else {
      GLfloat q = v < 0.5 ? v * (1 + s) : v + s - v * s;
      GLfloat p = 2 * v - q;
      r_ = hue2rgb(p, q, h + 1.f/3.f);
      g_ = hue2rgb(p, q, h);
      b_ = hue2rgb(p, q, h - 1.f/3.f);
    }
    *r = r_*255;
    *g = g_*255;
    *b = b_*255;
  }

}  // namespace km
