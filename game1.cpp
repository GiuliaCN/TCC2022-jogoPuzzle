//#include <SDL/SDL.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

//audio

#include "drawFunctions.h"
#include "Entidades.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// file
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#define MAX_Z 6 //4
#define MAX_Y 30
#define MAX_X 9 //7

#define PI 3.142857

float theta_y = -30.0;
ofstream logfile;

struct GameData{
  //Level
  int  lx; //dim
  int  ly;
  int  lz;
  int Level[MAX_Z][MAX_Y][MAX_X];
  //Player position:
  int  px;
  int  py;
  int  pz;
  int  p_dirx;
  int  p_dirz;
  bool p_hanging;
  //Camera position:
  int cx;
  int cy;
  int cz;
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


int pegaValor(string s, string separador = "=")
{
    int start = s.find(separador) + separador.size();
    int end = -1;
    string val = s.substr(start, end - start);
    return stoi(val);
}

//printa em logfile Level
void PrintMap (GameData *GD) {
  logfile << "----- Imprimindo Instancia -----\n"; 
  logfile << "Player x=" << GD->px << " y="<< GD->py << " z="<< GD->pz << "\n";
  logfile << "\nMapa:\n";
  for(int y = 0; y < MAX_Y; y++) {
    logfile << "y = " << y << "\n";
    logfile << "Xv Z>\n";
    for(int x = 0; x < MAX_X; x++) {
      for(int z = 0; z < MAX_Z; z++)
        logfile << GD->Level[z][y][x] <<" ";
      logfile << "\n";
    }
  }
}

/*
GameData SaveInst (GameData *GD) {
  GameData novo = GameData();
  novo.cx = GD->cx;
  novo.cy = GD->cy;
  novo.cz = GD->cz;

  novo.p_dirx = GD->p_dirx;
  novo.p_dirz = GD->p_dirz;
  novo.p_hanging = GD->p_hanging;
  novo.px = GD->px;
  novo.py = GD->py;
  novo.pz = GD->pz;

  //novo.Level = GD->Level; // tem que copiar um a um


}*/

/*
int Y, GameData *GD
*/
void UpdateLevel (){
  // /*
  // teste matrix
  // */
  // int count = 0;
  // int Teste [3][4][5];
  // for (int i=0; i<3; i++)
  //   for (int j=0; j<4; j++)
  //     for (int k=0; k<5; k++){
  //       Teste[i][j][k]=count;
  //       count++;
  //     }
  //     /*
  // logfile << "Print Teste\n" << Teste;
  // for (int i=0; i<3; i++){
  //   for (int j=0; j<4; j++){
  //     for (int k=0; k<5; k++)
  //       logfile << Teste[i][j][k] << " ";
  //     logfile<<"\n";
  //   }
  //   logfile <<"\n";
  // }*/

  // logfile << "Print Teste[1][1]\n" << Teste[1][1];
  // logfile << "Print Teste[1]\n" << Teste[1];

}

void LoadMap (struct GameData *GD, string filename){
  ifstream arquivo;
  string line;
  arquivo.open (filename);
  //int reading_player=0;
  //int reading_mapa_set=0;
  bool reading_mapa=false;
  // player pos
  int pz,px,py;
  //mapa dim
  int mx,my,mz;

  int x,y,z; // preencher mapa
  // zerar mapa
  for(z = 0; z < MAX_Z; z++)
    for(y = 0; y < MAX_Y; y++)
      for(x = 0; x < MAX_X; x++)
	      GD->Level[z][y][x] = 0;

  logfile << "--- load map - inicio\n";

  if (arquivo.is_open())
  {
    while ( getline (arquivo,line) )
    {
      if (line == "Player:") {
        getline (arquivo,line);
        px = pegaValor(line);
        getline (arquivo,line);
        py = pegaValor(line);
        getline (arquivo,line);
        pz = pegaValor(line);
        logfile << "px= "<<px << " py= "<<py<< " pz= "<<pz<<"\n";;
      }
      else if (line == "Mapa:"){
        getline (arquivo,line);
        mx = pegaValor(line);
        getline (arquivo,line);
        my = pegaValor(line);
        getline (arquivo,line);
        mz = pegaValor(line);
        logfile << "mx= "<<mx << " my= "<<my<< " mz= "<<mz<<"\n";
        reading_mapa = true;
        y = -1;
      }
      else if (reading_mapa){
        if (line == "") {
          y++;
          x=0;
        }
        else {
          for (z=0;z<line.size();z++){
            GD->Level[z][y][x] = line[z] - '0';
            logfile << line[z]; 
          }
          x++;
          logfile << "\n";
        }

      }
    }
    arquivo.close();
  }

  else logfile << "Erro em abrir arquivo\n"; 
  
  
  GD->pz = pz;
  GD->py = py;
  GD->px = px;
  GD->cz = GD->pz;
  GD->cy = GD->py;
  GD->cx = GD->px;
  GD->p_dirx = 0;
  GD->p_dirz = 1;

  logfile << "--- load map - fim\n";
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
  logfile << "Player x=" << GD->px << " y="<< GD->py << " z="<< GD->pz << "\n";
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

void find_player(struct GameData *GD,
		 int *px, int *py, int *pz){
  *px = GD->px;
  *py = GD->py;
  *pz = GD->pz;
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
  int px,py,pz,i,j,k,di,dj,dk;
  //find_player(GD, &px, &py, &pz);

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
	  SetInitialView(theta_y);
	  
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

 
  //DrawMonsterPlant(tex);

  SetInitialView(theta_y);
  //glTranslatef(   -2*d*2.0,   -d +d*2.0 + 70.0,      0.0);
  glTranslatef(   -4*d*2.0,   -d +d*2.0 + 70.0,      0.0);  
  DrawRedDragon();

  SetInitialView(theta_y);
  glTranslatef( (GD->px-GD->cx)*d*2.0,
		(GD->py-GD->cy)*d*2.0,
		(GD->pz-GD->cz)*d*2.0);
  fix_player_direction(GD);
  DrawFluffy();
  
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

  logfile.open ("logfile.txt");
  logfile << "Log\n";
  
  /* Dimensions of our window. */
  int width =  1366; //1067; //512; //640;
  int height = 768; //600; //288; //480;
  /* Flags we will pass into SDL_SetVideoMode. */
  int flags = 0;
  SDL_Window *Window;
  GLuint tex[5];
  SDL_Surface *img = NULL;



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
  glGenTextures(5, tex);

  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);
  img = load_image((char *)"texture/wall_texture2.png");
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
  img = load_image((char *)"texture/wall_texture_sup2.png");
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
  img = load_image((char *)"texture/monsterplant_c.png");
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
  img = load_image((char *)"texture/face_monsterplant_2.png");
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
  img = load_image((char *)"texture/umbigo.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGB, 64, 64, 0,
	       GL_RGB, GL_UNSIGNED_BYTE,
	       img->pixels);
  
  LoadMap(&GD, "mapa/01.txt");
  //LoadLevel01(&GD);
  //PrintMap (&GD);
  UpdateLevel();

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
  logfile.close();
  /*
   * EXERCISE:
   * Record timings using SDL_GetTicks() and
   * and print out frames per second at program
   * end.
   */
  
  /* Never reached. */
  return 0;
}

