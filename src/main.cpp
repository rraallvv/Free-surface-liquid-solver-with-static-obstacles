#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cfloat>

#include "gluvi.h"
#include "fluidsim.h"
#include "openglutils.h"
#include "array2_utils.h"

using namespace std;

//Try changing the grid resolution
int grid_resolution = 25;
float timestep = 1./60;

//Display properties
bool draw_grid = true;
bool draw_particles = true;
bool draw_velocities = false;
bool draw_boundaries = true;

float grid_width = 1;

FluidSim sim;

//Gluvi stuff
//-------------
Gluvi::PanZoom2D cam(-0.1f, -0.35f, 1.2f);
double oldmousetime;
Vec2f oldmouse;
void display();
void mouse(int button, int state, int x, int y);
void drag(int x, int y);
void timer(int junk);

//Boundary definition - several circles in a circular domain.

Vec2f c0(0.5f,0.5f), c1(0.7f,0.5f), c2(0.3f,0.35f), c3(0.5f,0.7f);
float rad0 = 0.4f,  rad1 = 0.1f,  rad2 = 0.1f,   rad3 = 0.1f;

float square_phi(const Vec2f& position, const Vec2f& centre, float radius) {
	Vec2f p = position - centre;
	float a = atan2(p[1], p[0]);
	float h = hypot(p[0], p[1]);
	
	if (abs(a) < 0.25 * M_PI || abs(a) > 0.75 * M_PI)
		h *= abs(cos(a));
	else
		h *= abs(sin(a));

	return h - radius;
}

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius) {
	return (dist(position,centre) - radius);
}

float boundary_phi(const Vec2f& position) {
	float phi0 = -square_phi(position, c0, rad0);
	float phi1 = circle_phi(position, c1, rad1);
	float phi2 = square_phi(position, c2, rad2);
	float phi3 = circle_phi(position, c3, rad3);
	
	return phi0;
	//return min(min(phi0,phi1),min(phi2,phi3));
}


//Main testing code
//-------------
int main(int argc, char **argv)
{
	
	//Setup viewer stuff
	Gluvi::init("GFM Free Surface Liquid Solver with Static Variational Boundaries", &argc, argv);
	Gluvi::camera=&cam;
	Gluvi::userDisplayFunc=display;
	Gluvi::userMouseFunc=mouse;
	Gluvi::userDragFunc=drag;
	glClearColor(1,1,1,1);
	
	glutTimerFunc(1000, timer, 0);
	
	//Set up the simulation
	sim.initialize(grid_width, grid_resolution, grid_resolution);
	
	//set up a circle boundary
	sim.set_boundary(boundary_phi);
	
	//Stick some liquid particles in the domain
	for(int i = 0; i < sqr(grid_resolution); ++i) {
		float x = randhashf(i*2, 0,1);
		float y = randhashf(i*2+1, 0,1);
		Vec2f pt(x,y);
		if(boundary_phi(pt) > 0 && pt[0] < 0.5)
			sim.add_particle(pt);
	}
	
	Gluvi::run();
	
	return 0;
}


void display(void)
{
	
	if(draw_grid) {
		glColor3f(0,0,0);
		glLineWidth(1);
		draw_grid2d(Vec2f(0,0), sim.dx, sim.ni, sim.nj);
	}
	
	if(draw_boundaries) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		draw_circle2d(c0, rad0, 50);
		
		//There's a bug, so draw one more(?)
		draw_circle2d(c3, 0, 10);
	}
	
	if(draw_particles) {
		glColor3f(0,0,1);
		glPointSize(3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		draw_points2d(sim.particles);
		for(unsigned int p = 0; p < sim.particles.size(); ++p) {
			draw_circle2d(sim.particles[p], sim.particle_radius, 20);
		}
	}
	
	if(draw_velocities) {
		glColor3f(1,0,0);
		for(int j = 0;j < sim.nj; ++j) for(int i = 0; i < sim.ni; ++i) {
			Vec2f pos((i+0.5f)*sim.dx,(j+0.5f)*sim.dx);
			draw_arrow2d(pos, pos + 0.01f*sim.get_velocity(pos), 0.01f*sim.dx);
		}
	}
	
	
}

void mouse(int button, int state, int x, int y)
{
	Vec2f newmouse;
	cam.transform_mouse(x, y, newmouse.v);
	//double newmousetime=get_time_in_seconds();
	
	oldmouse=newmouse;
	//oldmousetime=newmousetime;
}

void drag(int x, int y)
{
	Vec2f newmouse;
	cam.transform_mouse(x, y, newmouse.v);
	//double newmousetime=get_time_in_seconds();
	
	oldmouse=newmouse;
	//oldmousetime=newmousetime;
}


void timer(int junk)
{
	
	sim.advance(timestep);
	
	glutPostRedisplay();
	glutTimerFunc(1, timer, 0);
	
}





