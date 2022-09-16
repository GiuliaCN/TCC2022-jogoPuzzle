// drawFunctions.h

#ifndef DRAWFUNCTIONS_H // include guard
#define DRAWFUNCTIONS_H

#include <GL/gl.h>

void crossproduct(float A[3], float B[3], float C[3]);

float magnitude(float A[3]);

void inverte_vetor(int L[4][2]);

void draw_cube(float d, GLuint tex[], int id1, int id2);

void draw_curved_cylinder_x(float radius0,
			    float radius1,
			    float bend_radius,
			    float bend_ang0,
			    float bend_ang1,
			    int slices);

void draw_curved_cylinder_x(float radius,
			    float bend_radius,
			    float bend_ang,
			    float length,
			    int slices,
			    bool cw);

void draw_curved_cylinder_y(float radius0,
			    float radius1,
			    float bend_radius,
			    float bend_ang0,
			    float bend_ang1,
			    int slices,
			    int dtheta);

/* Horn */
void draw_curved_cylinder_y(float radius0,
			    float radius1,
			    float bend_radius0,
			    float bend_radius1,
			    float bend_ang0,
			    float bend_ang1,
			    float Dx,
			    int slices,
			    int dtheta);

void draw_curved_cylinder_y(float radius,
			    float bend_radius,
			    float bend_ang,
			    float length,
			    int slices,
			    bool cw,
			    int dtheta);



void draw_ellipsoid(float a,
		    float b,
		    float c,
		    int slices);

void draw_RedDragon_Eyes(float a,
			 float b,
			 float c,
			 int slices);



void draw_ellipsoid_lune(float a,
			 float b,
			 float c,
			 int angle1,
			 int angle2,
			 int slices);



void draw_hairy_ellipsoid(float a,
			  float b,
			  float c,
			  int slices);



void draw_curved_jagged_pattern(float bend_radius,
				float bend_ang0,
				float bend_ang1,
				float h,
				int slices);


void draw_curved_ellipse(float bend_radius,
			 float bend_ang0,
			 float bend_ang1,
			 float h,
			 int slices);



void draw_Gielis_Equation(float A,
			  float B,
			  float n1,
			  float n2,
			  float n3,
			  int m,
			  int slices);



void draw_curved_Gielis_Equation(float bend_radius,
				 float bend_ang0,
				 float A,
				 float B,
				 float n1,
				 float n2,
				 float n3,
				 int m,
				 int slices);




/*
North and South along z axis.
sectorAng0, sectorAng1 \in [0, 360].
stackAng0, stackAng1 \in [-90, 90].
*/
void sphere_z(int sectorCount,
	      int stackCount,
	      float sectorAng0,
	      float sectorAng1,
	      float stackAng0,
	      float stackAng1,
	      float radius);


/*
North and South along y axis.
sectorAng0, sectorAng1 \in [0, 360].
stackAng0, stackAng1 \in [-90, 90].
*/
void sphere_y(int sectorCount,
	      int stackCount,
	      float sectorAng0,
	      float sectorAng1,
	      float stackAng0,
	      float stackAng1,
	      float radius);


void DrawFluffy();

//Red Dragon:
void DrawRedDragon();

void DrawMonsterPlant(GLuint tex[], float theta_y);

void SetInitialView(float theta_y);

void triangle(float x1, float y1, float z1,
	      float x2, float y2, float z2,
	      float x3, float y3, float z3);


#endif