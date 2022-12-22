
#include "drawFunctions.h"
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define PI 3.142857

#define SQUARE(x) ((x)*(x))

void crossproduct(float A[3], float B[3], float C[3]){
  C[0] = A[1]*B[2] - A[2]*B[1];
  C[1] = A[2]*B[0] - A[0]*B[2];
  C[2] = A[0]*B[1] - A[1]*B[0];
}

float magnitude(float A[3]){
  return sqrtf(A[0]*A[0] + A[1]*A[1] + A[2]*A[2]);
}

void inverte_vetor(int L[4][2]){
  int N = 4, tmp, i, j;
  for(i = 0; i < N/2; i++){
    for(j = 0; j < 2; j++){
      tmp = L[i][j];
      L[i][j] = L[N-1-i][j];
      L[N-1-i][j] = tmp;
    }
  }
}



void draw_cube(float d, GLuint tex[], int id1, int id2){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  int L[4][2] = {{-1,-1}, { 1,-1}, { 1, 1}, {-1, 1}};
  int i,j,k,l;
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_LINE);   
  for(k = -1; k <= 1; k += 2){
    inverte_vetor(L);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    glBindTexture(GL_TEXTURE_2D, tex[id1]);
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, k);
    for(l = 0; l < 4; l++){
      j = L[l][0];
      i = L[l][1];
      glTexCoord2f((j+1)/2, (i+1)/2);
      glVertex3f(j*d, i*d, k*d);
    }
    glEnd();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    glBindTexture(GL_TEXTURE_2D, tex[id1]);
    glBegin(GL_QUADS);
    glNormal3f(k, 0.0, 0.0);
    for(l = 3; l >= 0; l--){
      j = L[l][0];
      i = L[l][1];
      glTexCoord2f((j+1)/2, (i+1)/2);
      glVertex3f(k*d, i*d, j*d);
    }
    glEnd();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    glBindTexture(GL_TEXTURE_2D, tex[id2]);
    glBegin(GL_QUADS);
    glNormal3f(0.0, k, 0.0);
    for(l = 0; l < 4; l++){
      j = L[l][0];
      i = L[l][1];
      glTexCoord2f((j+1)/2, (i+1)/2);
      glVertex3f(i*d, k*d, j*d);
    }
    glEnd();
  }
}

void draw_cylinder_z(float radius0,
		     float radius1,
		     float z0,
		     float z1,
		     int dtheta){
  GLfloat angle, x0, y0, x1, y1, xb, yb, zb;
  float cos_angle, sin_angle;
  float mag;
  float C[3];
  int j;
  glBegin(GL_QUAD_STRIP);
  for(j = 0; j <= 360; j+= dtheta){
    angle = (float)j * (PI / 180.0f);
    cos_angle = cosf(angle);
    sin_angle = sinf(angle);
    y0 = radius0 * cos_angle;
    x0 = radius0 * sin_angle;
    y1 = radius1 * cos_angle;
    x1 = radius1 * sin_angle;
    
    C[0] = x0;
    C[1] = y0;
    C[2] = 0;
    mag = magnitude(C);
    C[0] /= mag;
    C[1] /= mag;
    C[2] /= mag;
    glNormal3f(C[0], C[1], C[2]);
    yb = y0;
    xb = x0;
    zb = z0;
    glVertex3f( xb, yb, zb );
    
    C[0] = x1;
    C[1] = y1;
    C[2] = 0;
    mag = magnitude(C);
    C[0] /= mag;
    C[1] /= mag;
    C[2] /= mag;
    glNormal3f(C[0], C[1], C[2]);
    yb = y1;
    xb = x1;
    zb = z1;
    glVertex3f( xb, yb, zb );
  }
  glEnd();
}


void draw_curved_cylinder_x(float radius0,
			    float radius1,
			    float bend_radius,
			    float bend_ang0,
			    float bend_ang1,
			    int slices){
  GLfloat w0, w1, ang0, ang1, angle, x0, y0, x1, y1, xb, yb, zb;
  float cos_angle, sin_angle, cos_ang0, sin_ang0, cos_ang1, sin_ang1;
  float rad0, rad1, mag;
  float C[3];
  int i, j;
  bend_ang0 *= (PI / 180.0f);
  bend_ang1 *= (PI / 180.0f);
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;
    ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
    ang1 = bend_ang0*(1.0-w1) + bend_ang1*w1;

    rad0 = radius0*(1.0-w0) + radius1*w0;
    rad1 = radius0*(1.0-w1) + radius1*w1;    

    sin_ang0 = sinf(ang0);
    cos_ang0 = cosf(ang0);
    sin_ang1 = sinf(ang1);
    cos_ang1 = cosf(ang1);
    
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j <= 360; j++){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = rad0 * cos_angle + bend_radius;
      y0 = rad0 * sin_angle;
      x1 = rad1 * cos_angle + bend_radius;
      y1 = rad1 * sin_angle;
      
      C[0] = sin_ang0*rad0*cos_angle;
      C[1] = y0;
      C[2] = cos_ang0*rad0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      xb = sin_ang0 * x0;
      yb = y0;
      zb = cos_ang0 * x0;
      glVertex3f( xb, yb, zb );
      
      C[0] = sin_ang1*rad1*cos_angle;
      C[1] = y1;
      C[2] = cos_ang1*rad1*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      xb = sin_ang1 * x1;
      yb = y1;
      zb = cos_ang1 * x1;
      glVertex3f( xb, yb, zb );
    }
    glEnd();
  }
}

void draw_curved_cylinder_x(float radius,
			    float bend_radius,
			    float bend_ang,
			    float length,
			    int slices,
			    bool cw){
  float bend_ang0,bend_ang1,delta_theta;
  delta_theta = (180.0/PI)*(length/bend_radius);
  if(cw){
    bend_ang0 = bend_ang - delta_theta;
    bend_ang1 = bend_ang;
  }
  else{
    bend_ang0 = bend_ang;
    bend_ang1 = bend_ang + delta_theta;
  }
  draw_curved_cylinder_x(radius,
			 radius,
			 bend_radius,
			 bend_ang0,
			 bend_ang1,
			 slices);
}

void draw_curved_cylinder_y(float radius0,
			    float radius1,
			    float bend_radius,
			    float bend_ang0,
			    float bend_ang1,
			    int slices,
			    int dtheta){
  GLfloat w0, w1, ang0, ang1, angle, x0, y0, x1, y1, xb, yb, zb;
  float cos_angle, sin_angle, cos_ang0, sin_ang0, cos_ang1, sin_ang1;
  float rad0, rad1, mag;
  float C[3];
  int i, j;
  bend_ang0 *= (PI / 180.0f);
  bend_ang1 *= (PI / 180.0f);
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;
    ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
    ang1 = bend_ang0*(1.0-w1) + bend_ang1*w1;

    rad0 = radius0*(1.0-w0) + radius1*w0;
    rad1 = radius0*(1.0-w1) + radius1*w1;    

    sin_ang0 = sinf(ang0);
    cos_ang0 = cosf(ang0);
    sin_ang1 = sinf(ang1);
    cos_ang1 = cosf(ang1);
    
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j <= 360; j+= dtheta){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      y0 = rad0 * cos_angle + bend_radius;
      x0 = rad0 * sin_angle;
      y1 = rad1 * cos_angle + bend_radius;
      x1 = rad1 * sin_angle;
      
      C[0] = x0;
      C[1] = sin_ang0*rad0*cos_angle;
      C[2] = cos_ang0*rad0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      yb = sin_ang0 * y0;
      xb = x0;
      zb = cos_ang0 * y0;
      glVertex3f( xb, yb, zb );
      
      C[0] = x1;
      C[1] = sin_ang1*rad1*cos_angle;
      C[2] = cos_ang1*rad1*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      yb = sin_ang1 * y1;
      xb = x1;
      zb = cos_ang1 * y1;
      glVertex3f( xb, yb, zb );
    }
    glEnd();
  }
}


/* Horn */
void draw_curved_cylinder_y(float radius0,
			    float radius1,
			    float bend_radius0,
			    float bend_radius1,
			    float bend_ang0,
			    float bend_ang1,
			    float Dx,
			    int slices,
			    int dtheta){
  GLfloat w0, w1, ang0, ang1, angle, x0, y0, x1, y1, xb, yb, zb;
  float cos_angle, sin_angle, cos_ang0, sin_ang0, cos_ang1, sin_ang1;
  float rad0, rad1, mag, bend_rad0, bend_rad1, dx0, dx1;
  float C[3];
  int i, j;
  bend_ang0 *= (PI / 180.0f);
  bend_ang1 *= (PI / 180.0f);
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;
    ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
    ang1 = bend_ang0*(1.0-w1) + bend_ang1*w1;

    rad0 = radius0*(1.0-w0) + radius1*w0;
    rad1 = radius0*(1.0-w1) + radius1*w1;    

    bend_rad0 = bend_radius0*(1.0-w0) + bend_radius1*w0;
    bend_rad1 = bend_radius0*(1.0-w1) + bend_radius1*w1;

    dx0 = Dx*w0;
    dx1 = Dx*w1;
    
    sin_ang0 = sinf(ang0);
    cos_ang0 = cosf(ang0);
    sin_ang1 = sinf(ang1);
    cos_ang1 = cosf(ang1);
    
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j <= 360; j+= dtheta){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      y0 = rad0 * cos_angle + bend_rad0;
      x0 = rad0 * sin_angle;
      y1 = rad1 * cos_angle + bend_rad1;
      x1 = rad1 * sin_angle;
      
      C[0] = x0;
      C[1] = sin_ang0*rad0*cos_angle;
      C[2] = cos_ang0*rad0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      yb = sin_ang0 * y0;
      xb = x0 + dx0;
      zb = cos_ang0 * y0;
      glVertex3f( xb, yb, zb );
      
      C[0] = x1;
      C[1] = sin_ang1*rad1*cos_angle;
      C[2] = cos_ang1*rad1*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      yb = sin_ang1 * y1;
      xb = x1 + dx1;
      zb = cos_ang1 * y1;
      glVertex3f( xb, yb, zb );
    }
    glEnd();
  }
}

void draw_curved_cylinder_y(float radius,
			    float bend_radius,
			    float bend_ang,
			    float length,
			    int slices,
			    bool cw,
			    int dtheta){
  float bend_ang0,bend_ang1,delta_theta;
  delta_theta = (180.0/PI)*(length/bend_radius);
  if(cw){
    bend_ang0 = bend_ang - delta_theta;
    bend_ang1 = bend_ang;
  }
  else{
    bend_ang0 = bend_ang;
    bend_ang1 = bend_ang + delta_theta;
  }
  draw_curved_cylinder_y(radius,
			 radius,
			 bend_radius,
			 bend_ang0,
			 bend_ang1,
			 slices,
			 dtheta);
}

void draw_ellipsoid(float a,
		    float b,
		    float c,
		    int slices){
  GLfloat w0, w1, angle, x0, y0, z0, x1, y1, z1, xb, yb, zb;
  float cos_angle, sin_angle;
  float mag, a0, a1, b0, b1;
  float C[3];
  int i, j;
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;

    z0 = (-c)*(1.0-w0) + c*w0;
    z1 = (-c)*(1.0-w1) + c*w1;    

    b0 = sqrtf(b*b*(1.0 - (z0*z0)/(c*c)));
    b1 = sqrtf(b*b*(1.0 - (z1*z1)/(c*c)));

    a0 = sqrtf(a*a*(1.0 - (z0*z0)/(c*c)));
    a1 = sqrtf(a*a*(1.0 - (z1*z1)/(c*c)));
    
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j <= 360; j++){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = a0 * cos_angle;
      y0 = b0 * sin_angle;
      x1 = a1 * cos_angle;
      y1 = b1 * sin_angle;
      
      C[0] = (2.0*x0)/(a*a); 
      C[1] = (2.0*y0)/(b*b); 
      C[2] = (2.0*z0)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x0, y0, z0 );

      C[0] = (2.0*x1)/(a*a); 
      C[1] = (2.0*y1)/(b*b); 
      C[2] = (2.0*z1)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x1, y1, z1 );
    }
    glEnd();
  }
}

void draw_ellipsoid_lune(float a,
			 float b,
			 float c,
			 int angle1,
			 int angle2,
			 int slices){
  GLfloat w0, w1, angle, x0, y0, z0, x1, y1, z1, xb, yb, zb;
  float cos_angle, sin_angle;
  float mag, a0, a1, b0, b1;
  float C[3];
  int i, j;
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;

    z0 = (-c)*(1.0-w0) + c*w0;
    z1 = (-c)*(1.0-w1) + c*w1;    

    b0 = sqrtf(b*b*(1.0 - (z0*z0)/(c*c)));
    b1 = sqrtf(b*b*(1.0 - (z1*z1)/(c*c)));

    a0 = sqrtf(a*a*(1.0 - (z0*z0)/(c*c)));
    a1 = sqrtf(a*a*(1.0 - (z1*z1)/(c*c)));
    
    glBegin(GL_QUAD_STRIP);
    for(j = angle1; j <= angle2; j++){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = a0 * cos_angle;
      y0 = b0 * sin_angle;
      x1 = a1 * cos_angle;
      y1 = b1 * sin_angle;
      
      C[0] = (2.0*x0)/(a*a); 
      C[1] = (2.0*y0)/(b*b); 
      C[2] = (2.0*z0)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x0, y0, z0 );

      C[0] = (2.0*x1)/(a*a); 
      C[1] = (2.0*y1)/(b*b); 
      C[2] = (2.0*z1)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x1, y1, z1 );
    }
    glEnd();
  }
}


void draw_hairy_ellipsoid(float a,
			  float b,
			  float c,
			  int slices){
  GLfloat w0, w1, angle, x0, y0, z0, x1, y1, z1, xb, yb, zb;
  float cos_angle, sin_angle, hair_theta, hair_R;
  float mag, a0, a1, b0, b1, th0, th1, c_th0, s_th0;
  float C[3];
  int i, j, dj, di = 3;
  hair_R = 8.0; //8.0; //32.0;
  th0 = 90.0; //90.0; //50.0;
  th1 = 180.0; //180.0; //110.0; //90.0;
  c_th0 = cosf(th0*(PI / 180.0f));
  s_th0 = sinf(th0*(PI / 180.0f));
  
  for(i = 1; i < 0.8*slices; i++){
    if(i%di != 0) continue;
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;

    z0 = (-c)*(1.0-w0) + c*w0;
    z1 = (-c)*(1.0-w1) + c*w1;    

    b0 = sqrtf(b*b*(1.0 - (z0*z0)/(c*c)));
    b1 = sqrtf(b*b*(1.0 - (z1*z1)/(c*c)));

    a0 = sqrtf(a*a*(1.0 - (z0*z0)/(c*c)));
    a1 = sqrtf(a*a*(1.0 - (z1*z1)/(c*c)));

    if(dj == 0)
      dj = 10; 
    else
      dj = 0;
    
    for(j = dj; j <= 360; j+=20){
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = a0 * cos_angle;
      y0 = b0 * sin_angle;
      x1 = a1 * cos_angle;
      y1 = b1 * sin_angle;
      
      C[0] = (2.0*x0)/(a*a); 
      C[1] = (2.0*y0)/(b*b); 
      C[2] = (2.0*z0)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;

      hair_theta = -(atan2f(x0, -z0)*(180.0f/PI))/2.0;
      
      glPushMatrix();
      glTranslatef(x0 - C[0]*2,
		   y0 - C[1]*2 - s_th0*hair_R,
		   z0 - C[2]*2 - c_th0*hair_R);
      glRotatef(hair_theta, 0.0, 1.0, 0.0 );
      draw_curved_cylinder_y(3.0,
			     0.0,
			     hair_R,
			     th0,
			     th1,
			     20,
			     4);
      glPopMatrix();
      
      C[0] = (2.0*x1)/(a*a); 
      C[1] = (2.0*y1)/(b*b); 
      C[2] = (2.0*z1)/(c*c); 
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
    }
  }
}




void draw_curved_jagged_pattern(float bend_radius,
				float bend_ang0,
				float bend_ang1,
				float h,
				int slices){
  GLfloat w0, w1, ang0, ang1, xb, yb, zb;
  float A[3],B[3],C[3];
  float mag;
  int i;
  bend_ang0 *= (PI / 180.0f);
  bend_ang1 *= (PI / 180.0f);
  glBegin(GL_TRIANGLES);
  for(i = 0; i < slices; i++){
    w0 = (float)i / (float)slices;
    w1 = (float)(i+1) / (float)slices;    
    ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
    ang1 = bend_ang0*(1.0-w1) + bend_ang1*w1;
    
    xb = sinf( ang0 ) * bend_radius;
    yb = 0.0;
    zb = cosf( ang0 ) * bend_radius;
    C[0] = xb/bend_radius;
    C[1] = 0.0;
    C[2] = zb/bend_radius;
    glNormal3f(C[0], C[1], C[2]);
    glVertex3f( xb, yb, zb );

    xb = sinf( (ang0+ang1)/2.0 ) * bend_radius;
    yb = h;
    zb = cosf( (ang0+ang1)/2.0 ) * bend_radius;
    C[0] = xb/bend_radius;
    C[1] = 0.0;
    C[2] = zb/bend_radius;
    glNormal3f(C[0], C[1], C[2]);
    glVertex3f( xb, yb, zb );
    
    xb = sinf( ang1 ) * bend_radius;
    yb = 0.0;
    zb = cosf( ang1 ) * bend_radius;
    C[0] = xb/bend_radius;
    C[1] = 0.0;
    C[2] = zb/bend_radius;
    glNormal3f(C[0], C[1], C[2]);
    glVertex3f( xb, yb, zb );
  }
  glEnd();
}



void draw_curved_ellipse(float bend_radius,
			 float bend_ang0,
			 float bend_ang1,
			 float h,
			 int slices){
  GLfloat w0, ang0, xb, yb, zb;
  float A[3],B[3],C[3];
  float mag;
  int i;
  bend_ang0 *= (PI / 180.0f);
  bend_ang1 *= (PI / 180.0f);
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i <= slices; i++){
    w0 = (float)i / (float)slices;
    ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
    
    xb = sinf( ang0 ) * bend_radius;
    yb = 0.0;
    zb = cosf( ang0 ) * bend_radius;
    C[0] = xb/bend_radius;
    C[1] = 0.0;
    C[2] = zb/bend_radius;
    glNormal3f(C[0], C[1], C[2]);
    glVertex3f( xb, yb, zb );
    yb = h*sqrtf(1.0 - ((w0 - 0.5)*(w0 - 0.5))/0.25) + 0.1;
    glVertex3f( xb, yb, zb );
  }
  glEnd();
}


void draw_curved_Gielis_Equation(float bend_radius,
				 float bend_ang0,
				 float A,
				 float B,
				 float n1,
				 float n2,
				 float n3,
				 int m,
				 int slices){
  float r, phi, sum, x, y, z, w0, ang0;
  int i;
  bend_ang0 *= (PI / 180.0f);
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i <= slices/2; i++){
    //-------------------    
    phi = (i*(360.0/slices))*(PI/180.0);
    sum =  powf(fabsf(cosf((m/4.0)*phi)/A), n2);
    sum += powf(fabsf(sinf((m/4.0)*phi)/B), n3);
    r = powf(sum, -1.0/n1);
    z = r*cosf(phi);
    x = r*sinf(phi);
    //-------------------
    ang0 = bend_ang0 - z/bend_radius;
    y = sinf(ang0) * bend_radius;
    z = cosf(ang0) * bend_radius;
    //-------------------
    glVertex3f(0.0, y, z);
    glVertex3f(x, y, z);
  }
  glEnd();
  glBegin(GL_QUAD_STRIP);
  i--;
  for(; i <= slices; i++){
    //------------------- 
    phi = (i*(360.0/slices))*(PI/180.0);
    sum =  powf(fabsf(cosf((m/4.0)*phi)/A), n2);
    sum += powf(fabsf(sinf((m/4.0)*phi)/B), n3);
    r = powf(sum, -1.0/n1);
    z = r*cosf(phi);
    x = r*sinf(phi);
    //-------------------
    ang0 = bend_ang0 - z/bend_radius;
    y = sinf(ang0) * bend_radius;
    z = cosf(ang0) * bend_radius;
    //-------------------
    glVertex3f(0.0, y, z);
    glVertex3f(x, y, z);
  }
  glEnd();  
}




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
	      float radius){
  float x,y,z,xy;
  float nx,ny,nz, Rinv = 1.0/radius;
  float s,t;
  float sectorStep; // = 2*PI/sectorCount;
  float stackStep; //= PI/stackCount;
  float sectorAngle, stackAngle;
  int i,j,p,q,qi,qj,n;
  GLfloat **V;
  sectorAng0 *= (PI/180.0);
  sectorAng1 *= (PI/180.0);
  stackAng0 *= (PI/180.0);
  stackAng1 *= (PI/180.0);
  sectorStep = (sectorAng1-sectorAng0)/sectorCount;
  stackStep = (stackAng1-stackAng0)/stackCount;
  n = (sectorCount+1)*(stackCount+1);
  V = (GLfloat **)calloc(n, sizeof(GLfloat *));
  if(V == NULL)
    return;
  for(i = 0; i < n; i++){
    V[i] = (GLfloat *)calloc(3, sizeof(GLfloat));
    if(V[i] == NULL) return;
  }
  
  for(i = 0; i <= stackCount; i++){
    stackAngle = stackAng0 + i*stackStep; //-PI/2.0 + i*stackStep;
    xy = radius*cosf(stackAngle);
    z = radius*sinf(stackAngle);
    for(j = 0; j <= sectorCount; j++){
      sectorAngle = sectorAng0 + j*sectorStep; //j*sectorStep;
      x = xy*cosf(sectorAngle);
      y = xy*sinf(sectorAngle);
      p = i*(sectorCount+1) +  j;
      V[p][0] = x;
      V[p][1] = y;
      V[p][2] = z;
    }
  }

  glBegin(GL_QUADS);
  for(i = 0; i < stackCount; i++){
    for(j = 0; j < sectorCount; j++){
      p = i*(sectorCount+1) +  j;
      nx = V[p][0]*Rinv; ny = V[p][1]*Rinv; nz = V[p][2]*Rinv;
      s = (float)j/sectorCount; t = (float)i/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[p]);

      qi = i; qj = j + 1;
      q = qi*(sectorCount+1) +  qj;      
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);

      qi = i + 1; qj = j + 1;
      q = qi*(sectorCount+1) +  qj;
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);
      
      qi = i + 1; qj = j;
      q = qi*(sectorCount+1) +  qj;
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);
    }
  }
  glEnd();
  
  for(i = 0; i < n; i++)
    free(V[i]);
  free(V);
}



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
	      float radius){
  float x,y,z,xz;
  float nx,ny,nz, Rinv = 1.0/radius;
  float s,t;
  float sectorStep; // = 2*PI/sectorCount;
  float stackStep; //= PI/stackCount;
  float sectorAngle, stackAngle;
  int i,j,p,q,qi,qj,n;
  GLfloat **V;
  sectorAng0 *= (PI/180.0);
  sectorAng1 *= (PI/180.0);
  stackAng0 *= (PI/180.0);
  stackAng1 *= (PI/180.0);
  sectorStep = (sectorAng1-sectorAng0)/sectorCount;
  stackStep = (stackAng1-stackAng0)/stackCount;
  n = (sectorCount+1)*(stackCount+1);
  V = (GLfloat **)calloc(n, sizeof(GLfloat *));
  if(V == NULL)
    return;
  for(i = 0; i < n; i++){
    V[i] = (GLfloat *)calloc(3, sizeof(GLfloat));
    if(V[i] == NULL) return;
  }
  
  for(i = 0; i <= stackCount; i++){
    stackAngle = stackAng0 + i*stackStep; //-PI/2.0 + i*stackStep;
    xz = radius*cosf(stackAngle);
    y = radius*sinf(stackAngle);
    for(j = 0; j <= sectorCount; j++){
      sectorAngle = sectorAng0 + j*sectorStep; //j*sectorStep;
      x = xz*cosf(sectorAngle);
      z = xz*sinf(sectorAngle);
      p = i*(sectorCount+1) +  j;
      V[p][0] = x;
      V[p][1] = y;
      V[p][2] = z;
    }
  }

  glBegin(GL_QUADS);
  for(i = 0; i < stackCount; i++){
    for(j = 0; j < sectorCount; j++){
      p = i*(sectorCount+1) +  j;
      nx = V[p][0]*Rinv; ny = V[p][1]*Rinv; nz = V[p][2]*Rinv;
      s = (float)j/sectorCount; t = (float)i/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[p]);

      qi = i; qj = j + 1;
      q = qi*(sectorCount+1) +  qj;      
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);

      qi = i + 1; qj = j + 1;
      q = qi*(sectorCount+1) +  qj;
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);
      
      qi = i + 1; qj = j;
      q = qi*(sectorCount+1) +  qj;
      nx = V[q][0]*Rinv; ny = V[q][1]*Rinv; nz = V[q][2]*Rinv;
      s = (float)qj/sectorCount; t = (float)qi/stackCount;
      glNormal3f(nx, ny, nz);
      glTexCoord2f(s, t);
      glVertex3fv(V[q]);
    }
  }
  glEnd();
  
  for(i = 0; i < n; i++)
    free(V[i]);
  free(V);
}


void DrawFluffy(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;

  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();
  glPushMatrix();
  glTranslatef( -a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  //hands:
  glPushMatrix();
  glTranslatef( a*0.95, 0.0,  c*0.75);
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  glPushMatrix();
  glTranslatef( -a*0.95, 0.0,  c*0.75);
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();

}



void DrawFluffy_push(bool reset){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float length_forearm = 0.0;
  static float step_forearm = 0.1;

  if (reset){
    length_forearm = 0;
    step_forearm = step_forearm > 0? step_forearm : -step_forearm;
  }
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();
  glPushMatrix();
  glTranslatef( -a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  //hands:
  //left arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( a*0.95 - 8.0, -c*0.25,  c*0.75 - a*0.10);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 90.0, //50.0,
			 180.0,
			 20);
  glPopMatrix();
  //left forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( a*0.95, -c*0.25,  c*0.75 - a*0.10);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( a*0.95, 0.0,  c*0.75 + d*length_forearm);
  glRotatef(90.0, 0.0, 0.0, 1.0 );
  glRotatef(90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();

  //right arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( -a*0.95 + 8.0, -c*0.25,  c*0.75 - a*0.10);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 180.0, //50.0,
			 270.0,
			 20);
  glPopMatrix();
  //right forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( -a*0.95, -c*0.25,  c*0.75 - a*0.10);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //right hand:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( -a*0.95, 0.0,  c*0.75 + d*length_forearm);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glRotatef(-90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();

  length_forearm += step_forearm;
  if(length_forearm > 1.0){
    step_forearm = -step_forearm;
    length_forearm = 1.0;
  }
  else if(length_forearm < 0.0){
    step_forearm = -step_forearm;
    length_forearm = 0.0;
  }
  
  //topete:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  //topete: (fio central)
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();
  
  //topete: (fio direito do fluffy)
  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //topete: (fio esquerdo do fluffy)
  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  
  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}


void DrawFluffy_pull(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float length_forearm = 0.3;
  static float step_forearm = 0.0;
  static float foot_angle = 0.0;
  static float step_foot_angle = 4.0;
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

  glPushMatrix();
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  glTranslatef( a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  glPushMatrix();
  glRotatef(-foot_angle, 1.0, 0.0, 0.0 );
  glTranslatef( -a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  foot_angle += step_foot_angle;
  if(foot_angle > 45.0){
    foot_angle = 45.0;
    step_foot_angle = -step_foot_angle;
  }
  else if(foot_angle < -45.0){
    foot_angle = -45.0;
    step_foot_angle = -step_foot_angle;
  }
  
  //hands:
  //left arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( a*0.95 - 8.0, -c*0.25,  c*0.75 - a*0.10);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 90.0, //50.0,
			 180.0,
			 20);
  glPopMatrix();
  //left forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( a*0.95, -c*0.25,  c*0.75 - a*0.10);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  //glTranslatef( a*0.95, 0.0,  c*0.75 + d*length_forearm);
  glTranslatef( a*1.15, 0.0,  c*0.75 + d*length_forearm);
  glRotatef(90.0, 0.0, 0.0, 1.0 );
  glRotatef(90.0, 0.0, 1.0, 0.0 );
  glRotatef(90.0, 1.0, 0.0, 0.0 ); 
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21);
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();

  //right arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( -a*0.95 + 8.0, -c*0.25,  c*0.75 - a*0.10);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 180.0, //50.0,
			 270.0,
			 20);
  glPopMatrix();
  //right forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( -a*0.95, -c*0.25,  c*0.75 - a*0.10);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //right hand:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( -a*1.15, 0.0,  c*0.75 + d*length_forearm);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glRotatef(-90.0, 0.0, 1.0, 0.0 );
  glRotatef( 90.0, 1.0, 0.0, 0.0 );  
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  
  //topete:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  //topete: (fio central)
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();
  
  //topete: (fio direito do fluffy)
  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //topete: (fio esquerdo do fluffy)
  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  
  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}


void DrawFluffy_walk(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float foot_angle = 0.0;
  static float step_foot_angle = 4.0;
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

  glPushMatrix();
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  glTranslatef( a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  glPushMatrix();
  glRotatef(-foot_angle, 1.0, 0.0, 0.0 );
  glTranslatef( -a*0.35, -b,  c*0.2);
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  foot_angle += step_foot_angle;
  if(foot_angle > 45.0){
    foot_angle = 45.0;
    step_foot_angle = -step_foot_angle;
  }
  else if(foot_angle < -45.0){
    foot_angle = -45.0;
    step_foot_angle = -step_foot_angle;
  }
  
  //hands:
  glPushMatrix();
  glTranslatef( a*0.95, 0.0,  c*0.75);
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  glPushMatrix();
  glTranslatef( -a*0.95, 0.0,  c*0.75);
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}


void DrawFluffy_hang(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float length_forearm = 0.75;
  static float step_forearm = 0.00;
  static float foot_angle = 25.0;
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  glTranslatef( 0.0, b*0.20,  0.0);
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

  glPushMatrix();
  glTranslatef( a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  glPushMatrix();
  glTranslatef( -a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  //hands:
  //left arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( a*0.95 - 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(-50.0, 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 90.0, //50.0,
			 180.0,
			 20);
  glPopMatrix();
  //left forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glRotatef(-50.0, 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( a*0.95, b*0.7,  c*0.75 + d*0.3);
  glRotatef(90.0, 0.0, 0.0, 1.0 );
  glRotatef(90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();

  //right arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( -a*0.95 + 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(-50.0, 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 180.0, //50.0,
			 270.0,
			 20);
  glPopMatrix();
  //right forearm:
  if(length_forearm > 0.0){
    glPushMatrix();
    glTranslatef( -a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glRotatef(-50.0, 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*length_forearm,
		    5);
    glPopMatrix();
  }
  //right hand:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef( -a*0.95, b*0.7,  c*0.75 + d*0.3);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glRotatef(-90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();

  //topete:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  //topete: (fio central)
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();
  
  //topete: (fio direito do fluffy)
  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //topete: (fio esquerdo do fluffy)
  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  
  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}


void DrawFluffy_hangright(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float length_forearm = 0.75;
  static float step_forearm = 0.00;
  static float foot_angle = 25.0;
  static float arm_angle_left  = 0.0; //20.0;
  static float arm_angle_right = 0.0;
  static float step_arm_angle = 15.0;
  static int   turn_arm = 0;
  float displacement, L_proj, L_right, gamma_right, L_left, gamma_left;
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  glTranslatef( 0.0, b*0.20,  0.0);
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

  glPushMatrix();
  glTranslatef( a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  glPushMatrix();
  glTranslatef( -a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  //hands:
  if(turn_arm == 0){
    arm_angle_right += step_arm_angle;
    if(arm_angle_right > 20.0){
      arm_angle_right = 20.0;
      turn_arm = 1;
    }
  }
  else if(turn_arm == 1){
    arm_angle_right -= step_arm_angle;
    arm_angle_left  -= step_arm_angle;
    if(arm_angle_right < 0.0 || arm_angle_left < -20.0){
      arm_angle_right = 0.0;
      arm_angle_left  = -20.0;
      turn_arm = 2;
    }
  }
  else{
    arm_angle_left += step_arm_angle;
    if(arm_angle_left > 0.0){
      arm_angle_left = 0.0;
      turn_arm = 0;
    }
  }
  
  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_right/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_right = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_right = 180.0*asinf(displacement/L_right)/PI - 50;

  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_left/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_left = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_left = 180.0*asinf(displacement/L_left)/PI - 50;  
  
  //left arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( a*0.95 - 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(arm_angle_left, 0.0, 0.0, 1.0);
  glRotatef(-(50.0+gamma_left), 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 90.0, //50.0,
			 180.0,
			 20);
  //left forearm:
  if(length_forearm > 0.0){
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    glTranslatef( 8.0, 0.0,  0.0);
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_left, //d*length_forearm,
		    5);
  }
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  //glPushMatrix();
  glTranslatef( 0.0, 0.0, d*L_left);
  glRotatef(50.0+gamma_left, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  glRotatef(-arm_angle_left, 0.0, 0.0, 1.0);
  glRotatef(90.0, 0.0, 0.0, 1.0 );
  glRotatef(90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21);

  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  
  //right arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( -a*0.95 + 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(arm_angle_right, 0.0, 0.0, 1.0);
  glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 180.0, //50.0,
			 270.0,
			 20);
  //right forearm:
  if(length_forearm > 0.0){
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    glTranslatef( -8.0, 0.0,  0.0);    
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_right, //d*length_forearm,
		    5);
  }
  //glPopMatrix();
  //right hand:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glTranslatef( 0.0, 0.0, d*L_right);
  glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  glRotatef(-arm_angle_right, 0.0, 0.0, 1.0);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glRotatef(-90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  
  //topete:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  //topete: (fio central)
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();
  
  //topete: (fio direito do fluffy)
  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //topete: (fio esquerdo do fluffy)
  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  
  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}


void DrawFluffy_hangleft(){
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat  gray[] = { 0.65f, 0.65f,  0.65f, 1.0f };
  static GLfloat   red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static GLfloat  opaco[] = { 0.0 };
  float d = 30.0, a,b,c;
  static int eye_opening = 200;
  static int step_eye_opening = 6;
  static float length_forearm = 0.75;
  static float step_forearm = 0.00;
  static float foot_angle = 25.0;
  static float arm_angle_left  = 0.0; //20.0;
  static float arm_angle_right = 0.0;
  static float step_arm_angle = 15.0;
  static int   turn_arm = 0;
  float displacement, L_proj, L_right, gamma_right, L_left, gamma_left;
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPolygonMode(GL_BACK, GL_FILL);

  a = 0.8*d;
  b = d;
  c = 0.8*d;
  glTranslatef( 0.0, b*0.20,  0.0);
  draw_hairy_ellipsoid(a, d, c, 40);

  glPushMatrix();
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(a, b, c, -5, 340, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -16,   0, 40);
  draw_ellipsoid_lune(a*0.95, b*0.95, c*0.95, -40, -18, 40);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.94, b*0.94, c*0.94, -40, 0, 40);    
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();
  glPushMatrix();
  glRotatef(90, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a, c, b,   0,  60, 40);
  draw_ellipsoid_lune(a, c, b, 120, 180, 40);
  glMaterialfv(GL_FRONT, GL_SHININESS, opaco);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97,  55,  65, 40);
  draw_ellipsoid_lune(a*0.97, c*0.97, b*0.97, 115, 125, 40);  
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPopMatrix();

  //foot:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

  glPushMatrix();
  glTranslatef( a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  glPushMatrix();
  glTranslatef( -a*0.35, -b,  c*0.2);
  glRotatef(foot_angle, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(a*0.30, b*0.10, c*0.70, 0,  180, 40);
  glPopMatrix();

  //hands:
  if(turn_arm == 0){
    arm_angle_left -= step_arm_angle;
    if(arm_angle_left < -20.0){
      arm_angle_left = -20.0;
      turn_arm = 1;
    }
  }
  else if(turn_arm == 1){
    arm_angle_right += step_arm_angle;
    arm_angle_left  += step_arm_angle;
    if(arm_angle_left > 0.0 || arm_angle_right > 20.0){
      arm_angle_left = 0.0;
      arm_angle_right = 20.0;
      turn_arm = 2;
    }
  }
  else{
    arm_angle_right -= step_arm_angle;
    if(arm_angle_right < 0.0){
      arm_angle_right = 0.0;
      turn_arm = 0;
    }
  }
  
  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_right/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_right = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_right = 180.0*asinf(displacement/L_right)/PI - 50;

  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_left/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_left = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_left = 180.0*asinf(displacement/L_left)/PI - 50;
  
  //left arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( a*0.95 - 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(arm_angle_left, 0.0, 0.0, 1.0);
  glRotatef(-(50.0+gamma_left), 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 90.0, //50.0,
			 180.0,
			 20);
  //left forearm:
  if(length_forearm > 0.0){
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    glTranslatef( 8.0, 0.0,  0.0);
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_left, //d*length_forearm,
		    5);
  }
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glTranslatef( 0.0, 0.0, d*L_left);
  glRotatef(50.0+gamma_left, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  glRotatef(-arm_angle_left, 0.0, 0.0, 1.0);
  glRotatef(90.0, 0.0, 0.0, 1.0 );
  glRotatef(90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef(-a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef( a*0.06, 0.0, -c*0.21); 
  
  glTranslatef( -a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(-12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  
  //right arm:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
  glTranslatef( -a*0.95 + 8.0, -c*0.10,  c*0.75 - a*0.25);
  glRotatef(arm_angle_right, 0.0, 0.0, 1.0);
  glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
  draw_curved_cylinder_x(3.0,
			 3.0,
			 8.0, //32.0,
			 180.0, //50.0,
			 270.0,
			 20);
  //glPopMatrix();
  //right forearm:
  if(length_forearm > 0.0){
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    glTranslatef( -8.0, 0.0,  0.0);    
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_right, //d*length_forearm,
		    5);
  }
  //right hand:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glTranslatef( 0.0, 0.0, d*L_right);
  glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  glRotatef(-arm_angle_right, 0.0, 0.0, 1.0);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glRotatef(-90.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(a*0.15, b*0.30, c*0.40, 40);

  glTranslatef( a*0.06, 0.0,  c*0.21); 
  draw_ellipsoid(a*0.18, b*0.26, c*0.20, 40);
  glTranslatef(-a*0.06, 0.0, -c*0.21);

  glTranslatef( a*0.045, b*0.2, -c*0.1);  
  glRotatef(-45.0, 1.0, 0.0, 0.0 );
  glRotatef(12.0, 0.0, 1.0, 0.0 );  
  draw_ellipsoid(a*0.10, b*0.10, c*0.30, 40);
  glPopMatrix();
  
  //topete:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  //topete: (fio central)
  glPushMatrix();
  glTranslatef(0.0, b*0.9, c*0.15); //-c*0.5);
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();
  
  //topete: (fio direito do fluffy)
  glPushMatrix();
  glTranslatef(-5.0, b*0.9, c*0.15);
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  //topete: (fio esquerdo do fluffy)
  glPushMatrix();
  glTranslatef(5.0, b*0.9, c*0.15);
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_curved_cylinder_y(2.0,
			 0.0,
			 8.0, //32.0,
			 -20.0, //50.0,
			 130.0,
			 20,
			 4);
  glPopMatrix();

  
  //cauda:
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.05);
  draw_ellipsoid(d*0.30, d*0.30, d*0.30, 10);
  glPopMatrix();
  
  //sobrancelha:
  glPushMatrix();
  glTranslatef(-a*0.15, b*0.55, c*0.75);
  glRotatef(-35.0, 0.0, 0.0, 1.0 );
  glRotatef(-30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //sobrancelha:
  glPushMatrix();
  glTranslatef(a*0.15, b*0.55, c*0.75);
  glRotatef(35.0, 0.0, 0.0, 1.0 );
  glRotatef(30.0, 0.0, 1.0, 0.0 );
  draw_ellipsoid(d*0.25, d*0.15, d*0.15, 10);
  glPopMatrix();

  //olhos:
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
  glPushMatrix();
  glTranslatef(0.0, b*0.25, c*0.5);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270+eye_opening/2-360, 270-eye_opening/2, 10);
  glPopMatrix();

  if(eye_opening > 340 || eye_opening < 200)
    step_eye_opening = -step_eye_opening;
  eye_opening += step_eye_opening;

  //focinho:
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
  glTranslatef(0.0, b*0.15, c*1.05); //0.0, 0.0, c
  draw_ellipsoid(d*0.20, d*0.10, d*0.15, 10);
  glPopMatrix();
}

void SetInitialView(float theta_y, float zoom, float rotacao){
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, zoom*1.0);
  glRotatef(rotacao*1.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
}