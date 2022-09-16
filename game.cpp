//#include <SDL/SDL.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

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
  torre * Torre;
  player * Player;
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


// int pegaValor(string s, string separador = "=")
// {
//     int start = s.find(separador) + separador.size();
//     int end = -1;
//     string val = s.substr(start, end - start);
//     return stoi(val);
// }

void printaBloco(block b){
    //cout << "\n------Print Bloco------\n";
    logfile << "\nBloco x=" <<b.x << ", z=" << b.z << endl;
    if (b.prox==nullptr)
        logfile << "Bloco sem prox" << endl;
    else
        logfile << "Bloco com prox" << endl;
    
    //cout << "------Fim Print Bloco------\n\n";
}

void printaAndar(andar *a){
    logfile << "\n------Print andar------\n";
    logfile << "Andar id=" << a->id << endl;

    for (block * prox = a->Lista; prox != nullptr; prox = prox->prox){
        printaBloco(*prox);
    }    
    logfile << "------Fim Print andar------\n\n";
}

//printa em logfile Level
void PrintMap (GameData *GD) {
  logfile << "----- Imprimindo Instancia -----\n"; 
  logfile << "Player x=" << GD->Player->x << " y="<< GD->Player->andarAtual << " z="<< GD->Player->z << "\n";
  logfile << "\nMapa:\n";
  while (GD->Torre->andarAtual != nullptr){
    printaAndar(GD->Torre->andarAtual);
    GD->Torre->sobeAndar();
  }
}

void LoadMap (struct GameData *GD, string filename){
  ifstream arquivo;
  string line;

  arquivo.open (filename);
  if (arquivo.is_open()){
    getline (arquivo,line);
    GD->Player->SetPlayer(line);
    arquivo.close();

    GD->Torre->SetTorre(filename);

    GD->cz = GD->Player->z;
    GD->cy = GD->Player->andarAtual;
    GD->cx = GD->Player->x;
  }  

  else logfile << "Erro em abrir arquivo\n"; 
   
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
  nz = GD->Player->z + GD->Player->rotacao[1];
  nx = GD->Player->x + GD->Player->rotacao[0];
  // if(nz >= MAX_Z || nz < 0 || nx >= MAX_X || nx < 0)
  //   return;

  // com bloco na frente
  if (GD->Torre->andarAtual->coordenadaOcupada(nx,nz)){
    // se esta empurrando
    if (push){
      logfile << "empurra bloco\n";
      GD->Torre->andarAtual->RemoveBloco(nx,nz);
      GD->Torre->andarAtual->AdicionaBloco(nx+GD->Player->rotacao[0],nz+GD->Player->rotacao[1]);
    }
    else{
      // sobe se nao esta ocupado
      if (! GD->Torre->andarAtual->prox->coordenadaOcupada(nx,nz)){
        // falta tratar ultimo andar
        GD->Torre->sobeAndar();
        GD->Player->andarAtual=GD->Torre->andarAtual->id;
        GD->Player->x = nx;
        GD->Player->z = nz;
      }
    }
  }
  // sem bloco na frente
  else{
    GD->Player->x = nx;
    GD->Player->z = nz;
    // se nao tem chao
    if (! GD->Torre->andarAtual->ant->coordenadaOcupada(nx,nz)){
      GD->Player->estaCaindo=true;
      GD->Torre->desceAndar();
      if (GD->Torre->andarAtual != nullptr)
        GD->Player->andarAtual = GD->Torre->andarAtual->id;
      else
        logfile << "\n>>>GAMEOVER<<<\n";
    }
  }
}

void player_fall (struct GameData *GD){
  logfile << "caindo\n";

  if (GD->Torre->andarAtual == nullptr){
    // game over
    logfile << "\n>>>GAMEOVER<<<\n";
  }
  else if (GD->Torre->andarAtual->ant == nullptr){
    // game over
    logfile << "\n>>>GAMEOVER<<<\n";
  }
  else {
    GD->Player->andarAtual = GD->Torre->andarAtual->id;
    if (GD->Torre->andarAtual->ant->coordenadaOcupada(GD->Player->x,GD->Player->z))
      GD->Player->estaCaindo=false;
  }
  GD->Torre->desceAndar();
}

void move_player_back(struct GameData *GD, bool pull){

  int nx,nz;
  nz = GD->Player->z + GD->Player->rotacao[1];
  nx = GD->Player->x + GD->Player->rotacao[0];

  // checa se pode ir para tras
  if (! GD->Torre->andarAtual->coordenadaOcupada(GD->Player->x - GD->Player->rotacao[0],GD->Player->z - GD->Player->rotacao[1])) {
    
    if(GD->Torre->andarAtual->coordenadaOcupada(nx,nz) && pull){
      logfile << "puxa bloco\n";
      GD->Torre->andarAtual->RemoveBloco(nx,nz);
      GD->Torre->andarAtual->AdicionaBloco(GD->Player->x,GD->Player->z);
    }

    //anda para tras
    GD->Player->x = GD->Player->x - GD->Player->rotacao[0];
    GD->Player->z = GD->Player->z - GD->Player->rotacao[1];

    // se nao tem chao, fica pendurado
    if (! GD->Torre->andarAtual->ant->coordenadaOcupada(GD->Player->x, GD->Player->z)){
      GD->Torre->desceAndar();
      GD->Player->andarAtual = GD->Torre->andarAtual->id;
      GD->Player->estaPendurado=true;
    }
  }
  
}

void move_player_sideways (struct GameData *GD,bool left){
  int cx, cz;
  int dir = GD->Player->iRotacao;
  if (left){
    // move em direcao anti horaria
    dir = (dir+3)%4;
  }
  else
    dir = (dir+1)%4;
  
  cx = GD->Player->rotacoes[dir][0];
  cz = GD->Player->rotacoes[dir][1];

  // se tem bloco para ir
  if (GD->Torre->andarAtual->coordenadaOcupada(GD->Player->x + cx + GD->Player->rotacao[0],GD->Player->z +GD->Player->rotacao[1]+ cz)){
    GD->Player->x += cx;
    GD->Player->z += cz;
  }

  // se não tem, é um canto
  else{
    GD->Player->x += cx + GD->Player->rotacao[0];
    GD->Player->z += cz + GD->Player->rotacao[1];
    if (left)
      GD->Player->Rotaciona(1);
    else
      GD->Player->Rotaciona(0);
  }

}

static void handle_key( SDL_KeyboardEvent *key, struct GameData *GD, bool down){
  static bool hold_ctrl = false;

  logfile << "\nhandle key = " << key->keysym.sym <<"\n";
  logfile << "Player x=" << GD->Player->x << " y="<< GD->Player->andarAtual << " z="<< GD->Player->z << "\n";
  logfile << "Rotacao = [" << GD->Player->rotacao[0] << ", " << GD->Player->rotacao[1] <<"]\n";
  
  switch( key->keysym.sym ) {
  case SDLK_ESCAPE:
    if(down) quit_game( 0 );
    break;
  case SDLK_SPACE:
    break;
  case SDLK_LEFT:
    if(down) {     
      if (GD->Player->estaPendurado){
        move_player_sideways(GD,1);
      }
      else{
        GD->Player->Rotaciona(0);
      }
    }
    break;
  case SDLK_RIGHT:
    if(down) {
      if (GD->Player->estaPendurado){
        move_player_sideways(GD,0);
      }
      else{
        GD->Player->Rotaciona(1);
      }
    }
    break;
  case SDLK_DOWN:
    if(down) {
      if (GD->Player->estaPendurado){
        GD->Player->estaPendurado = false;
        if (!GD->Torre->andarAtual->ant->coordenadaOcupada(GD->Player->x,GD->Player->z))
          GD->Player->estaCaindo=true;
      }
      else
        move_player_back(GD, hold_ctrl);
    }
    break;
  case SDLK_UP:
    if(down) {
      if (GD->Player->estaPendurado)
          GD->Player->estaPendurado=false;

      move_player_front(GD, hold_ctrl);
    }
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

    while (GD->Player->estaCaindo)
      player_fall(GD);

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
  *px = GD->Player->x;
  *py = GD->Player->andarAtual;
  *pz = GD->Player->x;
}

void fix_player_direction(struct GameData *GD){
  if(GD->Player->rotacao[0] == 0 && GD->Player->rotacao[1]  == 1)
    return;
  else if(GD->Player->rotacao[0]  == 0 && GD->Player->rotacao[1] == -1)
    glRotatef(180.0, 0.0, 1.0, 0.0 );
  else if(GD->Player->rotacao[0]  == -1 && GD->Player->rotacao[1] == 0)
    glRotatef(-90.0, 0.0, 1.0, 0.0 );
  else if(GD->Player->rotacao[0]  == 1 && GD->Player->rotacao[1] == 0)
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

  for (andar * a = GD->Torre->primeiroAndar; a != nullptr; a = a->prox)
    for (block * b = a->Lista; b != nullptr; b = b->prox){
      dj = b->x - GD->cx;
      di = a->id - GD->cy;
      dk = b->z - GD->cz;
      SetInitialView(theta_y);      
      glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
      draw_cube(d, tex, 0, 1);
    }
 
  //DrawMonsterPlant(tex);

  SetInitialView(theta_y);
  //glTranslatef(   -2*d*2.0,   -d +d*2.0 + 70.0,      0.0);
  glTranslatef(   -4*d*2.0,   -d +d*2.0 + 70.0,      0.0);  
  DrawRedDragon();

  SetInitialView(theta_y);
  glTranslatef( 
    (GD->Player->x - GD->cx)*d*2.0,
		(GD->Player->andarAtual - GD->cy)*d*2.0,
		(GD->Player->z  - GD->cz)*d*2.0);
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
  GD.Player = new player;
  GD.Torre = new torre;

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
  //PrintMap(&GD);
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

