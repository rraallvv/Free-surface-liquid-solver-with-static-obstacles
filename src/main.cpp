#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cfloat>

#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>

#include "fluidsim.h"
#include "openglutils.h"
#include "array2_utils.h"

using namespace std;

//Try changing the grid resolution
int grid_resolution = 25;
int particles = 1500;

//Display properties
bool draw_grid = true;
bool draw_particles = true;
bool draw_velocities = false;
bool draw_boundaries = true;

float grid_width = 1;

FluidSim sim;

//Gluvi stuff
//-------------
int window_w = 500;
int window_h = 500;
double oldmousetime;
Vec2f oldmouse;
void display();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

//Boundary definition - several circles in a circular domain.

Vec2f c0(0.5f,0.5f), c1(0.7f,0.5f), c2(0.3f,0.35f), c3(0.5f,0.7f);
float rad0 = 0.459f,  rad1 = 0.1f,  rad2 = 0.1f,   rad3 = 0.1f;

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

void display(void)
{
	float dt = dumpTime();

	sim.advance(dt);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT);
		
	if(draw_grid) {
		glColor3f(.5,.5,.5);
		glLineWidth(1);
		draw_grid2d(Vec2f(0,0), sim.dx, sim.ni, sim.nj);
	}
	
	if(draw_boundaries) {
		glColor3f(1,1,1);
		glLineWidth(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Vec2f c = c0 - Vec2f(rad0,rad0);
		draw_box2d(c, 2*rad0, 2*rad0);
	}
	
	if(draw_particles) {
		glColor3f(0,0,1);
		glEnable( GL_POINT_SMOOTH );
		glPointSize(6);
		glColor3f(1,1,1);
		draw_points2d(sim.particles);
	}
	
	if(draw_velocities) {
		glColor3f(1,1,0);
		for(int j = 0;j < sim.nj; ++j) for(int i = 0; i < sim.ni; ++i) {
			Vec2f pos((i+0.5f)*sim.dx,(j+0.5f)*sim.dx);
			draw_arrow2d(pos, pos + 0.01f*sim.get_velocity(pos), 0.01f*sim.dx);
		}
	}
	
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	Vec2f newmouse;
	//double newmousetime=get_time_in_seconds();
	
	oldmouse=newmouse;
	//oldmousetime=newmousetime;
}

void motion(int x, int y)
{
	Vec2f newmouse;
	//double newmousetime=get_time_in_seconds();
	
	oldmouse=newmouse;
	//oldmousetime=newmousetime;
}

void idle() {
	glutPostRedisplay();
}

void keyboard( unsigned char key, int x, int y ) {
	key = toupper(key);
	switch (key) {
		case 27:
			exit(0);
			break;
			
		default:
			break;
	}
}

int main(int argc, char **argv)
{
	//Set up the simulation
	sim.initialize(grid_width, grid_resolution, grid_resolution);
	
	//set up a circle boundary
	sim.set_boundary(boundary_phi);
	
	//Stick some liquid particles in the domain
	for(int i = 0; i < particles; ++i) {
		float x = -rand()/(float)RAND_MAX * rad0;
		float y = (rand()/(float)RAND_MAX * 2.0f - 1.0f) * rad0;
		Vec2f pt(x,y);
		sim.add_particle(pt + c0);
	}
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GL_DOUBLE);
	glutInitWindowPosition ( 100, 100 );
	glutInitWindowSize ( window_w, window_h );
	glutCreateWindow(argv[0]);
	
	CGLContextObj context = CGLGetCurrentContext();
	const GLint SYNC_TO_REFRESH = 0;
	CGLSetParameter(context, kCGLCPSwapInterval, &SYNC_TO_REFRESH);
	
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc (motion);
	glutDisplayFunc(display);
	//glutReshapeFunc(reshape);
	glutMainLoop();
	
	return 0;
}
