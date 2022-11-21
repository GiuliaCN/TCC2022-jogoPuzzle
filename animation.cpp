//#include <SDL/SDL.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define SQUARE(x) ((x)*(x))

#define MAX_Z 6 //4
#define MAX_Y 30
#define MAX_X 9 //7

#define PI 3.142857


enum animationType {Normal, Pushing, Walking, Pulling, Hanging, HangingLeft, HangingRight};

enum animationType anim;


float theta_y = -30.0;

struct GameData{
  int Level[MAX_Z][MAX_Y][MAX_X];
  //Player position:
  int  px;
  int  py;
  int  pz;
  int  p_dirx;
  int  p_dirz;
  bool p_hanging;
  //Camera position:
  float cx;
  float cy;
  float cz;
  //Camera velocity:
  float c_vely;
};



SDL_Surface *load_image( char *filename ) { 
  //The image that's loaded 
  SDL_Surface* loadedImage = NULL; 
  //The optimized image that will be used 
  //SDL_Surface* optimizedImage = NULL; 
  //Load the image using SDL_image 
  loadedImage = IMG_Load( filename ); 

  if( loadedImage == NULL ) {
    printf("Error: Load Image %s\n",filename);
    exit(1);
  }
  //If the image loaded 
  //if( loadedImage != NULL ) { 
    //Create an optimized image 
    //optimizedImage = SDL_DisplayFormat( loadedImage ); 
    //Free the old image 
    //SDL_FreeSurface( loadedImage ); 
  //}
  //Return the optimized image 
  return loadedImage; //optimizedImage; 
}


void crossproduct(float A[3], float B[3], float C[3]){
  C[0] = A[1]*B[2] - A[2]*B[1];
  C[1] = A[2]*B[0] - A[0]*B[2];
  C[2] = A[0]*B[1] - A[1]*B[0];
}

float magnitude(float A[3]){
  return sqrtf(A[0]*A[0] + A[1]*A[1] + A[2]*A[2]);
}


void LoadLevel01(struct GameData *GD){ //int Level[MAX_Z][MAX_Y][MAX_X]){
  int x,y,z;
  for(z = 0; z < MAX_Z; z++)
    for(y = 0; y < MAX_Y; y++)
      for(x = 0; x < MAX_X; x++)
	GD->Level[z][y][x] = 0;
  y = 0;
  for(z = 1; z < MAX_Z-1; z++)
    for(x = 1; x < MAX_X-1; x++)
      GD->Level[z][y][x] = 1;
  z = 1;
  x = (MAX_X-2)/2 + 1;
  for(y = 0; y < MAX_Y; y++)
      GD->Level[z][y][x] = 1;
  GD->Level[3][1][3] = 1;
  GD->pz = 4;
  GD->py = 1;
  GD->px = 5;
  GD->cz = GD->pz;
  GD->cy = GD->py;
  GD->cx = GD->px;
  GD->p_dirx = 0;
  GD->p_dirz = 1;
  GD->c_vely = 0.025;
}



static void quit_game( int code ){
  /*
   * Quit SDL so we can release the fullscreen
   * mode and restore the previous video settings,
   * etc.
   */
  SDL_Quit( );
  
  /* Exit program. */
  exit( code );
}


void move_player_front(struct GameData *GD, bool push){
  int nx,nz;
  nz = GD->pz + GD->p_dirz;
  nx = GD->px + GD->p_dirx;
  if(nz >= MAX_Z || nz < 0 || nx >= MAX_X || nx < 0)
    return;
  else if(!push &&
	  GD->Level[nz][GD->py][nx] != 0 &&
	  GD->Level[nz][GD->py+1][nx] != 0)
    return;
  else if(push &&
	  GD->Level[nz][GD->py][nx] != 0 &&
	  GD->Level[nz+ GD->p_dirz][GD->py][nx+ GD->p_dirx] != 0)
    return;

  if(push &&
     GD->Level[nz][GD->py][nx] != 0){
    GD->Level[nz+ GD->p_dirz][GD->py][nx+ GD->p_dirx] = GD->Level[nz][GD->py][nx];
    GD->Level[nz][GD->py][nx] = 0;
  }
  
  if(!push &&
     GD->Level[nz][GD->py][nx] != 0)
    GD->py += 1;
  else if(GD->Level[nz][GD->py-1][nx] == 0)
    GD->py -= 1;
  GD->pz = nz;
  GD->px = nx;  
}



void move_player_back(struct GameData *GD, bool pull){
  int nx,nz;
  nz = GD->pz - GD->p_dirz;
  nx = GD->px - GD->p_dirx;
  if(nz >= MAX_Z || nz < 0 || nx >= MAX_X || nx < 0)
    return;
  else if(GD->Level[nz][GD->py][nx] != 0)
    return;

  if(pull){
    GD->Level[GD->pz][GD->py][GD->px] = GD->Level[GD->pz+GD->p_dirz][GD->py][GD->px+GD->p_dirx];
    GD->Level[GD->pz+GD->p_dirz][GD->py][GD->px+GD->p_dirx] = 0;
  }
  
  if(GD->Level[nz][GD->py-1][nx] == 0)
    GD->py -= 1;
  GD->pz = nz;
  GD->px = nx;  
}



void rotate_clockwise(struct GameData *GD){
  if(GD->p_dirx == 1 && GD->p_dirz == 0){
    GD->p_dirx = 0;
    GD->p_dirz = 1;
  }
  else if(GD->p_dirx == -1 && GD->p_dirz == 0){
    GD->p_dirx = 0;
    GD->p_dirz = -1;
  }
  else if(GD->p_dirx == 0 && GD->p_dirz == 1){
    GD->p_dirx = -1;
    GD->p_dirz = 0;
  }
  else if(GD->p_dirx == 0 && GD->p_dirz == -1){
    GD->p_dirx = 1;
    GD->p_dirz = 0;
  }
}



void rotate_counter_clockwise(struct GameData *GD){
  if(GD->p_dirx == 1 && GD->p_dirz == 0){
    GD->p_dirx = 0;
    GD->p_dirz = -1;
  }
  else if(GD->p_dirx == -1 && GD->p_dirz == 0){
    GD->p_dirx = 0;
    GD->p_dirz = 1;
  }
  else if(GD->p_dirx == 0 && GD->p_dirz == 1){
    GD->p_dirx = 1;
    GD->p_dirz = 0;
  }
  else if(GD->p_dirx == 0 && GD->p_dirz == -1){
    GD->p_dirx = -1;
    GD->p_dirz = 0;
  }
}



static void handle_key( SDL_KeyboardEvent *key, struct GameData *GD, bool down){
  static bool hold_ctrl = false;
  switch( key->keysym.sym ) {
  case SDLK_ESCAPE:
    if(down) quit_game( 0 );
    break;
  case SDLK_SPACE:
    break;
  case SDLK_LEFT:
    if(down) rotate_counter_clockwise(GD);
    break;
  case SDLK_RIGHT:
    if(down) rotate_clockwise(GD);
    break;
  case SDLK_DOWN:
    if(down) move_player_back(GD, hold_ctrl);
    break;
  case SDLK_UP:
    if(down) move_player_front(GD, hold_ctrl);
    break;
  case SDLK_z:
    if(down) theta_y -= 1.0;
    break;
  case SDLK_x:
    if(down) theta_y += 1.0;
    break;    
  case SDLK_LCTRL: //Left Ctrl
    if(down) hold_ctrl = true;
    else     hold_ctrl = false;
    break;
  case SDLK_RALT: //Right Alt
    break;
  case SDLK_0:
    anim = Normal;
    break;
  case SDLK_1:
    anim = Pushing;
    break;
  case SDLK_2:
    anim = Walking;
    break;
  case SDLK_3:
    anim = Pulling;
    break;
  case SDLK_4:
    anim = Hanging;
    break;
  case SDLK_5:
    anim = HangingRight;
    break;
  case SDLK_6:
    anim = HangingLeft;
    break;
  default:
    break;
  }
}


static void process_events(struct GameData *GD){
    /* Our SDL event placeholder. */
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {

        switch( event.type ) {
        case SDL_KEYDOWN:
	  /* Handle key presses. */
	  handle_key( &event.key, GD, true);
	  break;
	case SDL_KEYUP:
	  handle_key( &event.key, GD, false);
	  break;
	case SDL_QUIT:
	  /* Handle quit requests (like Ctrl-c). */
	  quit_game( 0 );
	  break;
        }

    }

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
      glTexCoord2f((j+1)/2, 1-(i+1)/2);
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
      glTexCoord2f((j+1)/2, 1-(i+1)/2);
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



void draw_clock_tower(float d, GLuint tex[], int id1, int id2){
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
      glTexCoord2f((j+1)/2, 1-(i+1)/2);
      glVertex3f(j*d, i*d, k*d*0.1);
    }
    glEnd();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    glBindTexture(GL_TEXTURE_2D, tex[id2]);
    glBegin(GL_QUADS);
    glNormal3f(k, 0.0, 0.0);
    for(l = 3; l >= 0; l--){
      j = L[l][0];
      i = L[l][1];
      glTexCoord2f((j+1)/2, 1-(i+1)/2);
      glVertex3f(k*d, i*d, j*d*0.1);
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
      glVertex3f(i*d, k*d, j*d*0.1);
    }
    glEnd();
  }
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
      
      C[0] = (2.0*x0)/(a*a); //b0*cos_angle;
      C[1] = (2.0*y0)/(b*b); //y0;
      C[2] = (2.0*z0)/(c*c); //b0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x0, y0, z0 );

      C[0] = (2.0*x1)/(a*a); //b1*cos_angle;
      C[1] = (2.0*y1)/(b*b); //y1;
      C[2] = (2.0*z1)/(c*c); //b1*cos_angle;
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



void draw_RedDragon_Eyes(float a,
			 float b,
			 float c,
			 int slices){
  GLfloat w0, w1, angle, x0, y0, z0, x1, y1, z1, xb, yb, zb;
  float cos_angle, sin_angle;
  float mag, a0, a1, b0, b1, ty, tz, tang;
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
    
    for(j = 90-2*24; j <= 90+2*24; j+=24){
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
      //glNormal3f(C[0], C[1], C[2]);

      ty = (2.0*b0)/(b*b);
      tz = (2.0*z0)/(c*c);
      tang = (180.0/PI)*atan2f(tz, ty);
      
      if(i == int(slices*0.7 + 0.5)){
	glPushMatrix();
	glTranslatef( x0, y0, z0 );
	glRotatef(j-90.0, 0.0, 0.0, 1.0 );
	glRotatef(tang,   1.0, 0.0, 0.0 );
	draw_ellipsoid(2.5, 2.5, 10.0, 30);
	glPopMatrix();
      }
    }

    for(j = 90-1*18; j <= 90+1*18; j+=18){
      if(j == 90) continue;
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = a0 * cos_angle;
      y0 = b0 * sin_angle;
      x1 = a1 * cos_angle;
      y1 = b1 * sin_angle;
      
      C[0] = (2.0*x0)/(a*a); //b0*cos_angle;
      C[1] = (2.0*y0)/(b*b); //y0;
      C[2] = (2.0*z0)/(c*c); //b0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      //glNormal3f(C[0], C[1], C[2]);

      ty = (2.0*b0)/(b*b);
      tz = (2.0*z0)/(c*c);
      tang = (180.0/PI)*atan2f(tz, ty);
      
      if(i == int(slices*0.87 + 0.5)){
	glPushMatrix();
	glTranslatef( x0, y0, z0 );
	glRotatef(j-90.0, 0.0, 0.0, 1.0 );
	glRotatef(tang,   1.0, 0.0, 0.0 );
	draw_ellipsoid(2.0, 2.0, 5.0, 30);
	glPopMatrix();
      }
    }
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
      
      C[0] = (2.0*x0)/(a*a); //b0*cos_angle;
      C[1] = (2.0*y0)/(b*b); //y0;
      C[2] = (2.0*z0)/(c*c); //b0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      glNormal3f(C[0], C[1], C[2]);
      
      glVertex3f( x0, y0, z0 );

      C[0] = (2.0*x1)/(a*a); //b1*cos_angle;
      C[1] = (2.0*y1)/(b*b); //y1;
      C[2] = (2.0*z1)/(c*c); //b1*cos_angle;
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
      dj = 10; //5;
    else
      dj = 0;
    
    for(j = dj; j <= 360; j+=20){ //10
      angle = (float)j * (PI / 180.0f);
      cos_angle = cosf(angle);
      sin_angle = sinf(angle);
      x0 = a0 * cos_angle;
      y0 = b0 * sin_angle;
      x1 = a1 * cos_angle;
      y1 = b1 * sin_angle;
      
      C[0] = (2.0*x0)/(a*a); //b0*cos_angle;
      C[1] = (2.0*y0)/(b*b); //y0;
      C[2] = (2.0*z0)/(c*c); //b0*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      //glNormal3f(C[0], C[1], C[2]);
      //glVertex3f( x0, y0, z0 );

      hair_theta = -(atan2f(x0, -z0)*(180.0f/PI))/2.0;
      //hair_theta = -60;
      //hair_theta = 0;      
      
      glPushMatrix();
      glTranslatef(x0 - C[0]*2,
		   y0 - C[1]*2 - s_th0*hair_R,
		   z0 - C[2]*2 - c_th0*hair_R);
      glRotatef(hair_theta, 0.0, 1.0, 0.0 );
      //glTranslatef(0.0, 0.0, -10.0);
      draw_curved_cylinder_y(3.0,
			     0.0,
			     hair_R,
			     th0,
			     th1,
			     20,
			     4);
      glPopMatrix();
      
      C[0] = (2.0*x1)/(a*a); //b1*cos_angle;
      C[1] = (2.0*y1)/(b*b); //y1;
      C[2] = (2.0*z1)/(c*c); //b1*cos_angle;
      mag = magnitude(C);
      C[0] /= mag;
      C[1] /= mag;
      C[2] /= mag;
      //glNormal3f(C[0], C[1], C[2]);
      //glVertex3f( x1, y1, z1 );
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
    //yb = 2.0*h*MIN(w0, (1.0-w0))+0.1;
    yb = h*sqrtf(1.0 - ((w0 - 0.5)*(w0 - 0.5))/0.25) + 0.1;
    glVertex3f( xb, yb, zb );
  }
  glEnd();
}




void draw_Gielis_Equation(float A,
			  float B,
			  float n1,
			  float n2,
			  float n3,
			  int m,
			  int slices){
  float r, phi, sum, x, y, z;
  int i;
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  for(i = 0; i <= slices; i++){
    phi = (i*(360.0/slices))*(PI/180.0);
    sum =  powf(fabsf(cosf((m/4.0)*phi)/A), n2);
    sum += powf(fabsf(sinf((m/4.0)*phi)/B), n3);
    r = powf(sum, -1.0/n1);
    z = r*cosf(phi);
    y = 0.0;
    x = r*sinf(phi);
    glVertex3f(x, y, z);
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
    //w0 = 1.0 - (float)i / (float)(slices/2);
    //ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
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
    //w0 = (float)(i - slices/2) / (float)(slices/2);
    //ang0 = bend_ang0*(1.0-w0) + bend_ang1*w0;
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
      //Vertex (x,y,z)
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
      //Vertex (x,y,z)
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




void find_player(struct GameData *GD,
		 int *px, int *py, int *pz){
  *px = GD->px;
  *py = GD->py;
  *pz = GD->pz;
}


/*
void find_player(struct GameData *GD,
		 int *px, int *py, int *pz){
  int i,j,k;
  for(k = 0; k < MAX_Z; k++)
    for(i = 0; i < MAX_Y; i++)
      for(j = 0; j < MAX_X; j++)
	if(Level[k][i][j] == 2){
	  *pz = k;
	  *py = i;
	  *px = j;
	}
}
*/


void triangle(float x1, float y1, float z1,
	      float x2, float y2, float z2,
	      float x3, float y3, float z3){
  glBegin(GL_TRIANGLES);
  glVertex3f(x1, y1, z1);
  glVertex3f(x2, y2, z2);
  glVertex3f(x3, y3, z3);
  glEnd();
}


void SetInitialView(){
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
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

  /*
  sphere_z(20, 20,
	   0.0, 360.0,
	   -90.0, 90.0,
	   d);
  */
  a = 0.8*d;
  b = d;
  c = 0.8*d;
  draw_hairy_ellipsoid(a, d, c, 40);
  //draw_ellipsoid(a, b, c, 40);

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
  //draw_ellipsoid_lune(a*0.9, c*0.9, b*0.9, 60, 120, 40);
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
  //glTranslatef(0.0, 0.0, -10.0);
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
  //glTranslatef(0.0, 0.0, -10.0);
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
  /*
  glPushMatrix();
  glTranslatef(a*0.20, 0.0, -c*1.20);
  draw_ellipsoid(d*0.20, d*0.20, d*0.20, 10);
  glPopMatrix();
  glPushMatrix();
  glTranslatef(-a*0.20, 0.0, -c*1.20);
  draw_ellipsoid(d*0.20, d*0.20, d*0.20, 10);
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0.0, -b*0.20, -c*1.20);
  draw_ellipsoid(d*0.20, d*0.20, d*0.20, 10);
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0.0, b*0.20, -c*1.20);
  draw_ellipsoid(d*0.20, d*0.20, d*0.20, 10);
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0.0, 0.0, -c*1.40);
  draw_ellipsoid(d*0.20, d*0.20, d*0.20, 10);
  glPopMatrix();
  */
  
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
  //draw_ellipsoid(d*0.45, d*0.45, d*0.45, 10);
  //draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 180, 360, 10);
  glRotatef(90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 270-eye_opening/2, 270+eye_opening/2, 10);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  //draw_ellipsoid_lune(d*0.45, d*0.45, d*0.45, 0, 180, 10);
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

  //boca:
  /*
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glPushMatrix();
  glTranslatef(0.0, -b*0.15, c*0.45);
  glRotatef(-90, 0.0, 1.0, 0.0 );
  draw_ellipsoid_lune(d*0.45, d*0.45, d*0.50, -30, 30, 10);  
  glPopMatrix();
  */
}





void DrawFluffy_push(){
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
  static float step_forearm = 0.05;
  
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
  //glTranslatef( -a*0.95, 0.0,  c*0.75 + d*length_forearm);
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
  static float step_arm_angle = 1.0;
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

  //printf("right = L: %f, gamma: %f, displacement: %f\n",L_right,gamma_right,displacement);

  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_left/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_left = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_left = 180.0*asinf(displacement/L_left)/PI - 50;

  //printf("left = L: %f, gamma: %f, displacement: %f\n",L_left,gamma_left,displacement);
  
  
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
  //glPopMatrix();
  //left forearm:
  if(length_forearm > 0.0){
    //glPushMatrix();
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    //glTranslatef( a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glTranslatef( 8.0, 0.0,  0.0);
    //glRotatef(20.0, 0.0, 0.0, 1.0);
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_left, //d*length_forearm,
		    5);
  }
  //glPopMatrix();
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  //glPushMatrix();
  glTranslatef( 0.0, 0.0, d*L_left);
  glRotatef(50.0+gamma_left, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  //glTranslatef( a*0.95, b*0.7,  c*0.75 + d*0.3);
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
    //glPushMatrix();
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    //glTranslatef( -a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glTranslatef( -8.0, 0.0,  0.0);    
    //glRotatef(20.0, 0.0, 0.0, 1.0);
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
  //glPushMatrix();
  glTranslatef( 0.0, 0.0, d*L_right);
  glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  //glTranslatef( 0.0, c*0.10+b*0.7, a*0.25+d*0.3);
  glRotatef(-arm_angle_right, 0.0, 0.0, 1.0);
  //glTranslatef( -a*0.95, b*0.7,  c*0.75 + d*0.3);
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
  static float step_arm_angle = 1.0;
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

  //printf("right = L: %f, gamma: %f, displacement: %f\n",L_right,gamma_right,displacement);

  L_proj = length_forearm*sinf(PI*50.0/180.0);
  displacement = 4.0*L_proj*(1.0/cosf(PI*arm_angle_left/180.0) - 1.0);
  displacement += L_proj; //+ 3.0/d;
  L_left = sqrtf(SQUARE(length_forearm*cosf(PI*50.0/180.0)) + SQUARE(displacement));
  gamma_left = 180.0*asinf(displacement/L_left)/PI - 50;

  //printf("left = L: %f, gamma: %f, displacement: %f\n",L_left,gamma_left,displacement);
  
  
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
  //glPopMatrix();
  //left forearm:
  if(length_forearm > 0.0){
    //glPushMatrix();
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    //glTranslatef( a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glTranslatef( 8.0, 0.0,  0.0);
    //glRotatef(20.0, 0.0, 0.0, 1.0);
    glRotatef(-(50.0+gamma_right), 1.0, 0.0, 0.0);
    draw_cylinder_z(3.0,
		    3.0,
		    0,
		    d*L_left, //d*length_forearm,
		    5);
  }
  //glPopMatrix();
  //left hand:  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  //glPushMatrix();
  glTranslatef( 0.0, 0.0, d*L_left);
  glRotatef(50.0+gamma_left, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  //glTranslatef( a*0.95, b*0.7,  c*0.75 + d*0.3);
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
    //glPushMatrix();
    glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
    //glTranslatef( -a*0.95, -c*0.10,  c*0.75 - a*0.25);
    glTranslatef( -8.0, 0.0,  0.0);    
    //glRotatef(20.0, 0.0, 0.0, 1.0);
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
  //glPushMatrix();
  glTranslatef( 0.0, 0.0, d*L_right);
  glRotatef(50.0+gamma_right, 1.0, 0.0, 0.0);
  glTranslatef( 0.0, c*0.26, a*0.02);
  //glTranslatef( 0.0, c*0.10+b*0.7, a*0.25+d*0.3);
  glRotatef(-arm_angle_right, 0.0, 0.0, 1.0);
  //glTranslatef( -a*0.95, b*0.7,  c*0.75 + d*0.3);
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




//Red Dragon:
void DrawRedDragon(){
  static GLfloat darkgray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  static GLfloat red[]   = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  float d = 30.0;

  //glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
  
  glDisable(GL_TEXTURE_2D);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkgray);
  glPolygonMode(GL_BACK, GL_FILL);

  draw_curved_cylinder_y(6.0, //30.0,
			 0.0,
			 20.0, //100.0,
			 10.0, //50.0,
			 0.0,
			 450.0,
			 20.0, //50.0
			 40,
			 1);
  glTranslatef( -d,  0.0,  0.0 );
  draw_curved_cylinder_y(6.0, //30.0,
			 0.0,
			 20.0, //100.0,
			 10.0, //50.0,
			 0.0,
			 450.0,
			 -20.0, //50.0
			 40,
			 1);
  glTranslatef( d/2.0,  -d/2.0,  1.0*d );
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  //draw_ellipsoid(25.0, 25.0, 50.0, 40);
  draw_ellipsoid_lune(25.0, 25.0, 50.0, 0, 180, 40);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  draw_RedDragon_Eyes(25.0, 25.0, 50.0, 40);
  
  glPushMatrix();
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  //glTranslatef( 0.0,  25.0, 25.0 );
  glRotatef(20.0, 1.0, 0.0, 0.0 );
  draw_ellipsoid_lune(24.0, 25.0, 50.0, 180, 360, 40);
  glPopMatrix();
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);  
  glTranslatef( 0.0,  -200.0,  0.0 );
  draw_curved_cylinder_y(25.0,
			 15.0,
			 200.0,
			 90.0,
			 135.0,
			 40,
			 1);
  glTranslatef( 0.0,  283.0,  -2.0*200.0*0.7071 ); //0.7071 == cosf(PI/2.0)
  draw_curved_cylinder_y(0.0,
			 15.0,
			 200.0,
			 270.0,
			 315.0,
			 40,
			 1);
}



void DrawMonsterPlant(GLuint tex[]){
  static GLfloat pink[] = { 1.0f, 0.753f,  0.796f, 1.0f };
  static GLfloat pink2[] = { 1.0f, 0.0f,  0.851f, 1.0f };  
  static GLfloat green1[] = { 0.125f, 0.914f,  0.047f, 1.0f };
  static GLfloat green2[] = { 0.173f, 0.478f,  0.078f, 1.0f };
  static GLfloat white[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat red[] = { 1.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat red2[] = { 0.5f, 0.0f,  0.0f, 1.0f };
  static GLfloat blue[] = { 0.0f, 0.0f,  1.0f, 1.0f };
  static GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  static GLfloat brilho[] = { 128.0 };
  static float theta_arm = 25.0;
  static float step_theta_arm = 0.1; //0.05;
  static float radius_arm = 70.0;
  static float step_radius_arm = 0.2; //0.1;
  float length_arm = 180.0, delta, t;
  static float theta_head = 0.0;
  static float step_theta_head = 0.02; //0.01;
  float d = 30.0;
  float x1,y1,z1;
  float x2,y2,z2;
  float x3,y3,z3;
  float dx,dy,dz;
  int i,j,k;
  GLUquadric *quad1 = NULL;

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green1);
  
  quad1 = gluNewQuadric();
  /*
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
  */
  SetInitialView();
  
  glTranslatef( 0.0, 50.0-d, 0.0);
  glDisable(GL_TEXTURE_2D);
  //draw_cube(4*d, tex, 2, 2);
  //glBindTexture(GL_TEXTURE_2D, tex[3]);
  //gluQuadricTexture(quad, true);
  //glutSolidSphere(20, 20, 20);
  //gluSphere(quad1,50,100,20);
  sphere_z(20, 20,
	   0.0, 360.0, //0.0, 360.0,
	   -90.0, 90.0,
	   50.0);
  glRotatef(-100.0, 1.0, 0.0, 0.0 );
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex[4]);
  sphere_z(20, 20,
	   265.0, 275.0, //0.0, 360.0,
	   -5.0, 5.0,
	   50.0);
  glRotatef(100.0, 1.0, 0.0, 0.0 );
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green1);
  
  glDisable(GL_TEXTURE_2D);

  glTranslatef( 0.0, 55.0, -7.0);
  
  sphere_z(20, 20,
	   0.0, 360.0,
	   -90.0, 90.0,
	   20.0);
  
  //glTranslatef( 0.0, 12.0, 70.0);
  if((theta_arm > 50.0 && step_theta_arm > 0.0)||
     (theta_arm < -25.0 && step_theta_arm < 0.0))
    step_theta_arm = -step_theta_arm;
  theta_arm += step_theta_arm;

  if((radius_arm > 90.0 && step_radius_arm > 0.0)||
     (radius_arm < 60.0 && step_radius_arm < 0.0))
    step_radius_arm = -step_radius_arm;
  radius_arm += step_radius_arm;
  
  glRotatef(theta_arm, 1.0, 0.0, 0.0 );
  glTranslatef( 0.0, 0.0, radius_arm);

  draw_curved_cylinder_x(5.0,
			 radius_arm,
			 180.0,
			 length_arm,
			 20,
			 true);
  draw_curved_cylinder_x(5.0,
			 radius_arm,
			 -180.0,
			 length_arm,
			 20,
			 false);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, blue);  
  glPolygonMode(GL_BACK, GL_FILL);

  delta = (180.0/PI)*(length_arm/radius_arm);
  draw_curved_ellipse(radius_arm,
		      180.0 - delta,
		      180.0 - delta*0.5,
		       40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		      180.0 - delta,
		      180.0 - delta*0.5,
		       -40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		      -180.0 + delta*0.5,
		      -180.0 + delta,
		       40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		      -180.0 + delta*0.5,
		      -180.0 + delta,
		       -40.0,
		      20);

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, red);  
  glTranslatef( 0.0, 5.0, 0.0);
  draw_curved_jagged_pattern(radius_arm,
			     180.0 - delta*0.5,
			     180.0 - delta*0.15,
			     8.0,
			     10);
  draw_curved_jagged_pattern(radius_arm,
			     -180.0 + delta*0.15,
			     -180.0 + delta*0.5,
			     8.0,
			     10);  
  glTranslatef( 0.0, -5.0, 0.0);

  glTranslatef( 0.0, -5.0, 0.0);
  draw_curved_jagged_pattern(radius_arm,
			     180.0 - delta*0.5,
			     180.0 - delta*0.15,
			     -8.0,
			     10);
  draw_curved_jagged_pattern(radius_arm,
			     -180.0 + delta*0.15,
			     -180.0 + delta*0.5,
			     -8.0,
			     10);  
  glTranslatef( 0.0, 5.0, 0.0);
  
  /*
  draw_curved_ellipse(radius_arm,
		       30.0,
		      120.0, //170.0,
		       40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		       30.0,
		      120.0, //170.0,
		       -40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		     -120.0, //-170.0,
		      -30.0,
		       40.0,
		      20);
  draw_curved_ellipse(radius_arm,
		     -120.0, //-170.0,
		      -30.0,
		       -40.0,
		      20);
  */

  t = (PI/180.0)*(180.0 - delta);
  glTranslatef( -sinf(t)*radius_arm, 0.0, cosf(t)*radius_arm);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
  sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 8.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
  for(k = -1; k <= 1; k += 2)
    for(i = -1; i <= 1; i += 2)
      for(j = -1; j <= 1; j += 2){
	glTranslatef(j*4.0, i*4.0, k*4.0);
	sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 4.0);
	glTranslatef(-j*4.0, -i*4.0, -k*4.0);
      }
  glTranslatef( sinf(t)*radius_arm, 0.0, -cosf(t)*radius_arm);
  

  t = (PI/180.0)*(-180.0 + delta);
  glTranslatef( -sinf(t)*radius_arm, 0.0, cosf(t)*radius_arm);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
  sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 8.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
  for(k = -1; k <= 1; k += 2)
    for(i = -1; i <= 1; i += 2)
      for(j = -1; j <= 1; j += 2){
	glTranslatef(j*4.0, i*4.0, k*4.0);
	sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 4.0);
	glTranslatef(-j*4.0, -i*4.0, -k*4.0);
      }
  glTranslatef( sinf(t)*radius_arm, 0.0, -cosf(t)*radius_arm);


  t = (PI/180.0)*(180.0 - delta*0.75);
  glTranslatef( -sinf(t)*radius_arm, 0.0, cosf(t)*radius_arm);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
  sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 8.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
  glTranslatef( sinf(t)*radius_arm, 0.0, -cosf(t)*radius_arm);


  t = (PI/180.0)*(-180.0 + delta*0.75);
  glTranslatef( -sinf(t)*radius_arm, 0.0, cosf(t)*radius_arm);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
  sphere_z(20, 20, 0.0, 360.0, -90.0, 90.0, 8.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
  glTranslatef( sinf(t)*radius_arm, 0.0, -cosf(t)*radius_arm);

  
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green1);
  glDisable(GL_TEXTURE_2D);
  GLUquadric *quad3 = NULL;
  quad3 = gluNewQuadric();
  /*
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
  */
  SetInitialView();


  //glTranslatef( 0.0, 20.0, -130);
  glRotatef(-90.0, 0.0, 0.0, 1.0 );
  glTranslatef( -100.0, 0.0, -30.0);
  //gluCylinder(quad3, 0, 25, 100, 20, 20);
  draw_curved_cylinder_x(25.0,
			 0.0,
			 100.0,
			 90.0,
			 150.0,
			 20);
 
  if((theta_head > 5.0 && step_theta_head > 0.0)||
     (theta_head < -5.0 && step_theta_head < 0.0))
    step_theta_head = -step_theta_head;
  theta_head += step_theta_head;
  
  dz = cosf((70.0/180.0)*PI)*75;
  dy = sinf((70.0/180.0)*PI)*75;

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  GLUquadric *quad2 = NULL;
  quad2 = gluNewQuadric();
  /*
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
  */
  SetInitialView();

  glTranslatef( 0.0, 50.0-d+dy, dz);  
  glEnable(GL_TEXTURE_2D);
  //glDisable(GL_TEXTURE_2D);
  //draw_cube(4*d, tex, 2, 2);  
  glBindTexture(GL_TEXTURE_2D, tex[3]);
  gluQuadricTexture(quad2, true);
  //glRotatef(-100.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_head, 0.0, 1.0, 0.0 );
  //gluSphere(quad2,25,100,20);
  sphere_y(20, 20, 0.0-90.0, 360.0-90.0, -90.0, 90.0, 25.0);
  //glutSolidSphere(20, 20, 20);
  glDisable(GL_TEXTURE_2D);
  
  /*
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
  */
  SetInitialView();

  glTranslatef( 0.0, 50.0-d+dy, dz);  
  glRotatef(theta_head, 0.0, 1.0, 0.0 );
  glDisable(GL_TEXTURE_2D);

  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_FILL);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT, GL_SHININESS, brilho);
  glMaterialfv(GL_BACK , GL_SPECULAR, white);
  glMaterialfv(GL_BACK , GL_SHININESS, brilho);  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
  glMaterialfv(GL_BACK,  GL_AMBIENT_AND_DIFFUSE, pink);
  for(k = 0; k < 10; k++){
    //if(k < 6) z1 = 0.0;
    //else
    z1 = 20.0;
    x1 = cosf(((k*36.0)/180.0)*PI)*(25+20);
    y1 = sinf(((k*36.0)/180.0)*PI)*(25+20);
    x2 = cosf(((k*36.0-18.0)/180.0)*PI)*(24.5);
    y2 = sinf(((k*36.0-18.0)/180.0)*PI)*(24.5);
    x3 = cosf(((k*36.0+18.0)/180.0)*PI)*(24.5);
    y3 = sinf(((k*36.0+18.0)/180.0)*PI)*(24.5);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink);
    triangle(x1, y1, z1,
	     x2, y2, 0.0,
	     x3, y3, 0.0);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red2);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex3f(x1*0.95, y1*0.95, z1*0.95);
    glVertex3f((x2+x3)/2.0, (y2+y3)/2.0, 0.0);
    glEnd(); 
  }
  for(k = 0; k < 10; k++){
    if(k < 6 || k == 9) z1 = 0.0;
    else z1 = 20.0;
    x1 = cosf(((k*36.0+18.0)/180.0)*PI)*(25+20);
    y1 = sinf(((k*36.0+18.0)/180.0)*PI)*(25+20);
    x2 = cosf(((k*36.0)/180.0)*PI)*(24.5);
    y2 = sinf(((k*36.0)/180.0)*PI)*(24.5);
    x3 = cosf(((k*36.0+2*18.0)/180.0)*PI)*(24.5);
    y3 = sinf(((k*36.0+2*18.0)/180.0)*PI)*(24.5);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
    triangle(x1, y1, z1,
	     x2, y2, 0.0,
	     x3, y3, 0.0);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red2);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex3f(x1*0.95, y1*0.95, z1*0.95);
    glVertex3f((x2+x3)/2.0, (y2+y3)/2.0, 0.0);
    glEnd(); 
  }

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pink2);
  sphere_y(20, 20, 180.0, 360.0, -90.0, 90.0, 25.07);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green2);
  sphere_z(20, 20, 0.0, 360.0, -90.0, -44.0, 25.2);
  
  glTranslatef( 0.0, 0.0, -20.0);  
  for(k = -2; k <= 2; k++){
    x1 = cosf(((90.0+k*45.0)/180.0)*PI)*(15+30);
    y1 = sinf(((90.0+k*45.0)/180.0)*PI)*(15+30);
    x2 = cosf(((90.0+k*45.0-22.5)/180.0)*PI)*(14.5);
    y2 = sinf(((90.0+k*45.0-22.5)/180.0)*PI)*(14.5);
    x3 = cosf(((90.0+k*45.0+22.5)/180.0)*PI)*(14.5);
    y3 = sinf(((90.0+k*45.0+22.5)/180.0)*PI)*(14.5);  
    triangle(x1, y1, 15.0,
	     x2, y2, 0.0,
	     x3, y3, 0.0);
  }
  
  glTranslatef( 0.0, -5.0, -58.0); //0,-5,-58
  /*
  for(i = -2; i <= 2; i++){
    glRotatef(180.0+40.0*i, 0.0, 0.0, 1.0 ); //45
    draw_curved_cylinder_y(0.0,
			   3.0,
			   60.0, //60
			   230.0, 
			   360.0, //360
			   20);
    draw_curved_Gielis_Equation(60.0, //60
				290.0, 
				3000.0,
				300.0,
				2,
				1.0,
				1.0,
				2, //2, 4
				60);
    glRotatef(-(180.0+40.0*i), 0.0, 0.0, 1.0 );    
  }  
  */
  
  glTranslatef( 0.0, 0.0, -40.0);
  for(i = -3; i <= 3; i+=2){
    glRotatef(180.0+20.0*i, 0.0, 0.0, 1.0 ); //45
    draw_curved_cylinder_y(0.0,
			   3.0,
			   100.0,
			   320.0, //250
			   360.0, //360
			   20,
			   1);
    draw_curved_Gielis_Equation(100.0,
				340.0, //310
				750.0,
				75.0,
				2,
				1.0,
				1.0,
				2, //2, 4
				60);
    glRotatef(-(180.0+20.0*i), 0.0, 0.0, 1.0 );    
  }

  /*
  glTranslatef(0.0, -25.0, 50.0);
  for(i = -1; i <= 1; i+=2){
    glTranslatef(i*5.0, 0.0, 0.0);
    glRotatef(180.0+35.0*i, 0.0, 0.0, 1.0 );
    draw_curved_cylinder_y(0.0,
			   3.0,
			   60.0,
			   190.0,
			   330.0,
			   20);
    draw_curved_Gielis_Equation(60.0,
				250.0,
				3000.0,
				300.0,
				2,
				1.0,
				1.0,
				2, //2, 4
				60);
    glRotatef(-(180.0+35.0*i), 0.0, 0.0, 1.0 );
    glTranslatef(-i*5.0, 0.0, 0.0);
  }  
  */ 

  /*
  glLoadIdentity( );
  glTranslatef( 0.0, 0.0, -300.0);
  glRotatef(35.0, 1.0, 0.0, 0.0 );
  glRotatef(theta_y, 0.0, 1.0, 0.0 );
  */
  SetInitialView();

  glTranslatef( 0.0, 50.0-d, 0.0);

  glRotatef(180.0, 0.0, 1.0, 0.0 );
  glTranslatef( 0.0, -12.0, 160.0);
  //draw_Gielis_Equation

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green1);
  draw_curved_cylinder_y(0.0,
			 3.0,
			 60.0,
			 60.0,
			 150.0,
			 20,
			 1);

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green2);
  draw_curved_Gielis_Equation(60.0,
			      120.0,
			      3000.0,
			      300.0,
			      2,
			      1.0,
			      1.0,
			      2, //2, 4
			      60);
  /*
  draw_Gielis_Equation(3000.0,
		       300.0,
		       2,
		       1.0,
		       1.0,
		       2, //2, 4
		       60);
  */
}



void fix_player_direction(struct GameData *GD){
  if(GD->p_dirx == 0 && GD->p_dirz == 1)
    return;
  else if(GD->p_dirx == 0 && GD->p_dirz == -1)
    glRotatef(180.0, 0.0, 1.0, 0.0 );
  else if(GD->p_dirx == -1 && GD->p_dirz == 0)
    glRotatef(-90.0, 0.0, 1.0, 0.0 );
  else if(GD->p_dirx == 1 && GD->p_dirz == 0)
    glRotatef(90.0, 0.0, 1.0, 0.0 );
}



void draw_screen(SDL_Window *Window,
		 struct GameData *GD, //int Level[MAX_Z][MAX_Y][MAX_X],
		 GLuint tex[]){
  /*
  static float theta_x = 0.0;
  static float theta_y = 0.0;
  static float theta_z = 0.0;
  */
  float d = 30.0;
  int px,py,pz,i,j,k;
  float di,dj,dk;
  //find_player(GD, &px, &py, &pz);

  //Camera movement:
  if(fabs(GD->py - GD->cy) <= GD->c_vely)
    GD->cy = GD->py;
  else if(GD->py < GD->cy)
    GD->cy -= GD->c_vely;
  else
    GD->cy += GD->c_vely;
    
  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  // We don't want to modify the projection matrix.
  glMatrixMode( GL_MODELVIEW );

  for(k = 0; k < MAX_Z; k++)
    for(i = 0; i < MAX_Y; i++)
      for(j = 0; j < MAX_X; j++)
	if(GD->Level[k][i][j] == 1){
	  dj = j - GD->cx;
	  di = i - GD->cy;
	  dk = k - GD->cz;
	  /*
	  glLoadIdentity( );
	  glTranslatef( 0.0, 0.0, -300.0);
	  glRotatef(35.0, 1.0, 0.0, 0.0 );
	  glRotatef(theta_y, 0.0, 1.0, 0.0 );
	  */
	  SetInitialView();
	  
	  glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );

	  // Rotate.
	  /*
	    theta_x += 0.025;
	    theta_y += 0.025;
	    glRotatef(theta_x, 1.0, 0.0, 0.0 );
	    glRotatef(theta_y, 0.0, 1.0, 0.0 );
	    glRotatef(theta_z, 0.0, 0.0, 1.0 );  
	  */
	  draw_cube(d, tex, 0, 1);
	}


  j = 4;
  k = 5;
  for(i = 1; i <= 6; i++){
    dj = j - GD->cx;
    di = i - GD->cy;
    dk = k - GD->cz;
    SetInitialView();
    glTranslatef( dj*d*2.0,  di*d*2.0,  dk*d*2.0 );
    draw_clock_tower(d, tex, 4+i, 11);
    SetInitialView();
    glTranslatef( dj*d*2.0,  di*d*2.0,  (dk-0.4)*d*2.0 );
    draw_clock_tower(d, tex, 11+i, 11);
  }

  //DrawMonsterPlant(tex);

  SetInitialView();
  //glTranslatef(   -2*d*2.0,   -d +d*2.0 + 70.0,      0.0);
  glTranslatef(   -4*d*2.0,   -d +d*2.0 + 70.0,      0.0);  
  DrawRedDragon();

  SetInitialView();
  glTranslatef( (GD->px-GD->cx)*d*2.0,
		(GD->py-GD->cy)*d*2.0,
		(GD->pz-GD->cz)*d*2.0);
  fix_player_direction(GD);

  if(anim == Normal)
    DrawFluffy();
  else if(anim == Pushing)
    DrawFluffy_push();
  else if(anim == Walking)
    DrawFluffy_walk();
  else if(anim == Pulling)
    DrawFluffy_pull();
  else if(anim == Hanging)
    DrawFluffy_hang();
  else if(anim == HangingRight)
    DrawFluffy_hangright();
  else if(anim == HangingLeft)
    DrawFluffy_hangleft();
  
  glEnable(GL_TEXTURE_2D);
  SDL_GL_SwapWindow(Window);
}




static void setup_opengl( int width, int height ){
  float ratio = (float) width / (float) height;
  static GLfloat light_pos[] = { 0.0f, 0.0f,  0.0f, 1.0f };
  static GLfloat light_amb[] = { 0.7f, 0.7f,  0.7f, 1.0f };
  static GLfloat light_dif[] = { 0.8f, 0.8f,  0.8f, 1.0f };
  static GLfloat light_spe[] = { 1.0f, 1.0f,  1.0f, 1.0f };
  static GLfloat light_att[] = { 0.00002f };
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  /* Our shading model--Gouraud (smooth). */
  glShadeModel( GL_SMOOTH );

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos );
  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_spe);
  glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light_att);

  /* Culling. */
  //CullFace( GL_BACK );
  //FrontFace( GL_CCW );
  //Enable( GL_CULL_FACE );
  
  /* Set the clear color. */
  glClearColor( 0, 0, 0, 0 );
  
  /* Setup our viewport. */
  glViewport( 0, 0, width, height );
  
  /*
   * Change to the projection matrix and set
   * our viewing volume.
   */
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );
  
  gluPerspective( 60.0, ratio, 1.0, 1024.0 );
  //glFrustum(-width/2, width/2, -height/2, height/2, 200.0, 1024.0);
}


int main( int argc, char* argv[] ){
  //int Level[MAX_Z][MAX_Y][MAX_X];
  struct GameData GD;
  
  /* Dimensions of our window. */
  int width =  1366; //1067; //512; //640;
  int height = 768; //600; //288; //480;
  /* Flags we will pass into SDL_SetVideoMode. */
  int flags = 0;
  SDL_Window *Window;
  GLuint tex[18];
  SDL_Surface *img = NULL;
  char filename[512];
  int i;
  
  /* First, initialize SDL's video subsystem. */
  /*
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n",
    SDL_GetError( ) );
    quit_game( 1 );
    }
  */
  
  Window = SDL_CreateWindow("OpenGL Test",
			    0, 0,
			    width, height,
			    SDL_WINDOW_OPENGL);
  assert(Window);
  SDL_GLContext Context = SDL_GL_CreateContext(Window);
  //SDL_SetWindowFullscreen(Window, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
  
  /*
   * Now, we want to setup our requested
   * window attributes for our OpenGL window.
   * We want *at least* 5 bits of red, green
   * and blue. We also want at least a 16-bit
   * depth buffer.
   *
   * The last thing we do is request a double
   * buffered window. '1' turns on double
   * buffering, '0' turns it off.
   *
   * Note that we do not use SDL_DOUBLEBUF in
   * the flags to SDL_SetVideoMode. That does
   * not affect the GL attribute state, only
   * the standard 2D blitting setup.
     */
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  /*
   * At this point, we should have a properly setup
   * double-buffered window for use with OpenGL.
   */
  setup_opengl( width, height );

  

  glEnable(GL_TEXTURE_2D);
  glGenTextures(5+7+6, tex);

  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"wall_texture2.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 512, 512, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);

  glBindTexture(GL_TEXTURE_2D, tex[1]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"wall_texture_sup2.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 512, 512, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);
  
  /*
  img = load_image((char *)"spiderplant_c.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 512, 512, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);
  */  
  glBindTexture(GL_TEXTURE_2D, tex[2]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"monsterplant_c.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 2048, 2048, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);

  
  glBindTexture(GL_TEXTURE_2D, tex[3]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"face_monsterplant_2.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 1024, 1024, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);

  glBindTexture(GL_TEXTURE_2D, tex[4]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"umbigo.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 64, 64, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);


  for(i = 1; i <= 6; i++){
    glBindTexture(GL_TEXTURE_2D, tex[4+i]);
    glTexParameteri(GL_TEXTURE_2D,
		    GL_TEXTURE_MAG_FILTER,
		    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
		    GL_TEXTURE_MIN_FILTER,
		    GL_LINEAR);
    sprintf(filename, "torre_A%d.png", i);
    img = load_image(filename);
    glTexImage2D(GL_TEXTURE_2D, 0,
		 GL_RGB, 512, 512, 0,
		 GL_RGB, GL_UNSIGNED_BYTE,
		 img->pixels);
  }


  glBindTexture(GL_TEXTURE_2D, tex[11]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"torre_L.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 512, 512, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);  
  

  for(i = 1; i <= 6; i++){
    glBindTexture(GL_TEXTURE_2D, tex[11+i]);
    glTexParameteri(GL_TEXTURE_2D,
		    GL_TEXTURE_MAG_FILTER,
		    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
		    GL_TEXTURE_MIN_FILTER,
		    GL_LINEAR);
    sprintf(filename, "torre_B%d.png", i);
    img = load_image(filename);
    glTexImage2D(GL_TEXTURE_2D, 0,
		 GL_RGB, 512, 512, 0,
		 GL_RGB, GL_UNSIGNED_BYTE,
		 img->pixels);
  }

  
  LoadLevel01(&GD);

  /*
   * Now we want to begin our normal app process--
   * an event loop with a lot of redrawing.
   */
  while( 1 ) {
    /* Process incoming events. */
    process_events(&GD);
    /* Draw the screen. */
    draw_screen(Window, &GD, tex);
  }
  
  /*
   * EXERCISE:
   * Record timings using SDL_GetTicks() and
   * and print out frames per second at program
   * end.
   */
  
  /* Never reached. */
  return 0;
}

