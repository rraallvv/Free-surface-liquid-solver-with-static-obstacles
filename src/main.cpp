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
int grid_width = 20;
int grid_height = 30;
int particles = 1500;

//Display properties
bool draw_grid = false;
bool draw_particles = true;
bool draw_velocities = false;
bool draw_boundaries = true;

FluidSim sim;

//Gluvi stuff
//-------------
int window_w = 400;
int window_h = 600;
float prev_x;
float prev_y;

//Boundary definition - several circles in a circular domain.
float c0 = 0.5f;
float c1 = 0.75f;
float rad0 = 0.5f - 1.0f / (float) grid_width - 0.001f;
float rad1 = 0.75 - 1.0f / (float) grid_width - 0.001f;

float boundary_phi(const Vec2f& position) {
	float p0 = position[0] - c0;
	float p1 = position[1] - c1;
	
	float a = atan2(p1, p0);
	float h = hypot(p0, p1);
	
	float phi;
	
	float a0 = atan2(rad1, rad0);
	
	if (abs(a) < a0 || abs(a) > M_PI - a0) {
		h *= abs(cos(a));
		phi = rad0 - h;
	}
	else {
		h *= abs(sin(a));
		phi = rad1 - h;
	}
	
	return phi;
}

//Main testing code
//-------------

void init(int np, int gm, int gn)
{
	//Set up the simulation
	sim.initialize(1, gm, gn);
	
	//set up a circle boundary
	sim.set_boundary(boundary_phi);
	
	//Stick some liquid particles in the domain
	for(int i = 0; i < particles; ++i) {
		float x = -rand()/(float)RAND_MAX * rad0;
		float y = (rand()/(float)RAND_MAX * 3.0f - 1.5f) * rad1;
		Vec2f pt(x+c0,y+c1);
		sim.add_particle(pt);
	}
}

void display(void)
{
	float dt = dumpTime();
	
	Vec2f p = Vec2f(0.25, 0.25);
	Vec2f d = Vec2f(0, 0.1);
	
	sim.add_velocity(p, d);

	sim.advance(dt);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1.5, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT);
		
	if(draw_grid) {
		glColor3f(.5,.5,.5);
		glLineWidth(1);
		draw_grid2d(Vec2f(0,0), sim.dx, sim.ni, sim.nj);
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
	
	glColor3f(1, 1, 1);
	glRasterPos2d(0.01, 0.01);

	static unsigned int frames = 0;
	static float total_t = 0;
	total_t += dt;
	drawStr(GLUT_BITMAP_HELVETICA_18, "FPS AGV: ~%.2f", frames/total_t);
	frames++;
	
	glutSwapBuffers();
}

void mouse ( int button, int state, int x, int y ) {
	prev_x = x;
	prev_y = y;
}

void motion ( int x, int y ) {
	float s = 8.0;
	
	Vec2f p = Vec2f(x/(float)window_w, 1.0f - y/(float)window_h);
	Vec2f d = Vec2f(s * (x - prev_x)/(float)window_w, s * -(y - prev_y)/(float)window_h);
	
	sim.add_velocity(p, d);
	
	prev_x = x;
	prev_y = y;
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
	init(particles, grid_width, grid_height);
	
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
