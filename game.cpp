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
#include <map>

using namespace std;

#define MAX_Z 6 //4
#define MAX_Y 30
#define MAX_X 9 //7
#define literate true
#define numTex 9

#define PI 3.142857

// globais
//float theta_y = -30.0;
ofstream logfile;
map<int, string> teclado;
bool bloqueiaMov = false;
int estadoJogo = 1; // 1 = em jogo, 2 = vitoria, 3 = derrota

struct GameData{
  torre * Torre;
  player * Player;
  camera * Camera;
  int cx = 3;
  int cy = 2;
  int cz = 3;
  LLBlocos * ListaUpdate;
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

// void ConfiguraMapTeclado(){
//   teclado[1073741906] = "UP";
//   teclado[1073741904] = "LEFT";
//   teclado[1073741905] = "DOWN";
//   teclado[1073741903] = "RIGHT";
//   teclado[1073742048] = "CTRL";
//   teclado[27] = "ESC";
  
// }


void LoadMap (struct GameData *GD, string filename){
  ifstream arquivo;
  string line;

  arquivo.open (filename);
  if (arquivo.is_open()){
    //if (literate) printf("Arquivo Mapa aberto\n");

    getline (arquivo,line);
    //if (literate) printf("Set Player\n----\n");
    GD->Player->SetPlayer(line);

    getline (arquivo,line);
    //if (literate) printf("Set Camera\n----\n");
    GD->Camera->SetCamera(line);

    arquivo.close();

    if (literate)cout << GD->Player->PlayerToString() << endl;
    //if (literate) printf("Set Torre\n");
    GD->Torre->SetTorre(filename);
    //if (literate) printf("Mapa criado com sucesso\n");
    //if (literate) cout << GD->Torre->TorreToString() << endl;
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


void UpdateAndar(LLBlocos * ListaUpdate, torre * Torre, int n){
  logfile << "\nEntrou Update do andar_id=" << n << "\n";
  bool houveMudanca = false;
  block * b;
  block * aux;
  andar * base = Torre->retornaAndarN(n-1);
  andar * atual = Torre->retornaAndarN(n);
  if (base != nullptr && atual != nullptr){
    logfile << "\n Dando Update em andar\n ";
    logfile << "\n Andar Atual antes: " << atual->AndarToString() << "\n";
    //logfile << "\n Andar Base antes: " << base->AndarToString() << "\n";
    b = atual->Lista->lista;
    while (b != nullptr)
    {
      aux = b;
      b = b->prox;
      logfile << "\n checa suporte de " << aux->BlocoToString() << "\n";
      if (aux ->tipo==Movel && ! base->temSuporte(aux)){
        logfile << "\n cai bloco \n ";
        houveMudanca = true;
        aux->setPosLim(posicao(aux->pos));
        aux->setVel(velocidade(0,0,0));
        atual->EjetaBloco(aux);
        ListaUpdate->AdicionaBloco(aux);
      }
    }
    logfile << "\n Andar atual depois: " << atual->AndarToString() << "\n";
    //logfile << "\n Andar base depois: " << base->AndarToString() << "\n";
    if (houveMudanca) {
      logfile << "\n houve mudanca \n ";
      UpdateAndar(ListaUpdate, Torre, n+1);
    }
  }  
}


void move_player_front(struct GameData *GD, bool push){
  logfile << "\nEntrou player front\n";

  player * Player = GD->Player;
  torre * Torre = GD->Torre;

  posicao posDesejada = posicao(Player->pos) + velocidade(Player->rotacao);

  block * b = Torre->retornaBloco(posDesejada);

  // sem bloco na frente
  if (b == nullptr){
    logfile << "\nSem bloco na frente do player\n";
    logfile << "player anda\n";
    Player->mexe(posDesejada, velocidade(Player->rotacao));
  }
  // com bloco na frente
  else {
    logfile << "\n ComBlocoNaFrente \n";
    // esta empurrando
    if (push && b->tipo == Movel){
      // checa se pode
      logfile << "\n ChecaEmpurrar \n";
      posicao posBlocoAtras = posDesejada + velocidade(Player->rotacao);
      if (Torre->retornaBloco(posBlocoAtras) == nullptr){
        logfile << "\n Empurra \n";
        b->mexe(posBlocoAtras, velocidade(Player->rotacao));
        Torre->EjetaBloco(b);
        GD->ListaUpdate->AdicionaBloco(b);
        //UpdateAndar(GD->ListaUpdate, Torre, posDesejada.y + 1);
      }
    }
    else {
      // checa se pode subir
      logfile << "\n ChecaSePodeSubir \n";
      posicao posSubir = posDesejada + velocidade(0,1,0);
      posicao posAcima = posicao(Player->pos) + velocidade(0,1,0);
      if (Torre->retornaBloco(posSubir) == nullptr && Torre->retornaBloco(posAcima) == nullptr){
        logfile << "\n Sobe \n";
        //Player->setVel(velocidade(0,1,0));
        Player->mexe(posSubir,velocidade(Player->rotacao) + velocidade(0,1,0));
        //logfile << Player->vel->VelocidadeToString() << "\n";
      }
    }
  }
}

void move_player_back(struct GameData *GD, bool pull){
  logfile << "\nmove_player_back >> Entrou player back\n";

  player * Player = GD->Player;
  torre * Torre = GD->Torre;

  posicao posDesejada = posicao(Player->pos) - velocidade(Player->rotacao);
  posicao posFrente = posicao(Player->pos) + velocidade(Player->rotacao);
  posicao posChaoTras = posicao(Player->pos) - velocidade(Player->rotacao) + velocidade(0,-1,0);

  bool podeIrAtras = Torre->retornaBloco(posDesejada) == nullptr;
  block * blocoNaFrente = Torre->retornaBloco(posFrente);
  bool temChaoTras = Torre->retornaBloco(posChaoTras) != nullptr;
  // checa se pode ir para tras
  if (podeIrAtras){
    logfile << "\nSem bloco atras do player\n";
    // puxa se tiver bloco
    if (pull && blocoNaFrente != nullptr && blocoNaFrente->tipo==Movel){
        blocoNaFrente->mexe(posicao(Player->pos), velocidade(Player->rotacao)*(-1));
        Torre->EjetaBloco(blocoNaFrente);
        GD->ListaUpdate->AdicionaBloco(blocoNaFrente);
        //UpdateAndar(GD->ListaUpdate, Torre, blocoNaFrente->pos->y);
    }

    // anda para tras
    Player->mexe(posDesejada,velocidade(Player->rotacao)*(-1));
    
    if (!temChaoTras) {
      Player->estado2 = TentaPendurar;
      Player->setAgarrar (posChaoTras + velocidade(Player->rotacao));
    }

  }

  else
    logfile << "\nmove_player_back >> Bloco atras do player\n";

}

void move_player_sideways (struct GameData *GD,bool right){
  logfile << "\nEntrou player sideways\n";

  player * Player = GD->Player;
  torre * Torre = GD->Torre;

  velocidade rotacaoLado;

  int dir = Player->iRotacao;
  if (right){
    // move em direcao horaria
    dir = (dir+1)%4;
  }
  else
    dir = (dir+3)%4;
    
  
  rotacaoLado = Player->rotacoes[dir];
//   cx = GD->Player->rotacoes[dir][0];
//   cz = GD->Player->rotacoes[dir][1];

  posicao posDesejada = posicao(Player->pos) + velocidade(rotacaoLado);

  posicao apoioDesejado = posicao(Player->pos) + velocidade(rotacaoLado) + velocidade(Player->rotacao);

  bool temApoio = Torre->retornaBloco(apoioDesejado) != nullptr;
  bool posLivre = Torre->retornaBloco(posDesejada) == nullptr;
  //logfile << "\nmove_player_sideways >> (temApoio, posLivre) = " << temApoio << posLivre<<"\n";
  // movimentacao lateral
  if (temApoio && posLivre){
    Player->mexe(posDesejada,velocidade(rotacaoLado));
  }
  // se for canto
  else {
    if (posLivre){
    Player->mexe(posDesejada + velocidade(Player->rotacao),velocidade(rotacaoLado) + velocidade(Player->rotacao));
    }
    if (right) Player->Rotaciona(1);
    else Player->Rotaciona(0);
  }


  // se tem bloco para ir
  // if (Torre->retornaBloco(posDesejada) == nullptr){
  //   GD->Player->x += cx;
  //   GD->Player->z += cz;
  // }

//   // se não tem, é um canto
//   else{
//     GD->Player->x += cx + GD->Player->rotacao[0];
//     GD->Player->z += cz + GD->Player->rotacao[1];
//     if (left)
//       GD->Player->Rotaciona(1);
//     else
//       GD->Player->Rotaciona(0);
//   }

}

static void handle_key( SDL_KeyboardEvent *key, struct GameData *GD, bool down){
  static bool hold_ctrl = false;

  //if (literate) logfile << "\nhandle key = " << teclado[key->keysym.sym] << "\n";
  //if (literate) logfile << GD->Player->PlayerToString() + "\n";
  player * Player = GD->Player;
  camera * Camera = GD->Camera;
  bool podeMover = Player->estado == Parado;
    //if (literate) logfile << "Pode Mover";

  switch( key->keysym.sym ) {

    case SDLK_ESCAPE:
      if(down) quit_game( 0 );
      break;

    case SDLK_SPACE:
      break;

    case SDLK_LEFT:
      if(down && podeMover) {     
        if (Player->estado2 == Pendurado){
          move_player_sideways(GD,1);
        }
        else{
          GD->Player->Rotaciona(0);
        }
      }
      break;
      
    case SDLK_RIGHT:
      if(down && podeMover) {
        if (Player->estado2 == Pendurado){
          move_player_sideways(GD,0);
        }
        else{
          GD->Player->Rotaciona(1);
        }
      }
      break;

    case SDLK_DOWN:
      if(down && podeMover) {
        if (Player->estado2 == Pendurado){
          Player->estado2 = Normal;
          Player->cai();
          logfile << "\n handle_key >> solta da beirada \n";
          logfile << Player->PlayerToString();
          //Player->mexe(posicao(Player->pos) + velocidade(0,-1,0),velocidade(0,-1,0));
        }
        else
          move_player_back(GD, hold_ctrl);
      }
      break;

    case SDLK_UP:
      if(down && podeMover) {
        if (Player->estado2 == Pendurado)
            Player->estado2 = Normal;

        move_player_front(GD, hold_ctrl);
      }
      break;

    case SDLK_LCTRL: //Left Ctrl
      if(down) hold_ctrl = true;
      else     hold_ctrl = false;
      break;

    case SDLK_RALT: //Right Alt
      break;

    // camera
    case SDLK_z: // controle de rotacao
      if(down) Camera->theta_y += 1.0;
      break;

    case SDLK_x:
      if(down) Camera->theta_y -= 1.0;
      break; 
      
    case SDLK_a: // controle no plano
      if(down) {
        Camera->setPos(posicao(Camera->pos)+posicao(1,0,0));
      }
      break;

    case SDLK_s:
      if(down) {
        Camera->setPos(posicao(Camera->pos)+posicao(-1,0,0));
      }
      break; 
      
    case SDLK_d: // controle de altura
      if(down) {
        Camera->setPos(posicao(Camera->pos)+posicao(0,1,0));
      }
      break;

    case SDLK_f:
      if(down) {
        Camera->setPos(posicao(Camera->pos)+posicao(0,-1,0));
      }
      break; 

    case SDLK_g: //
      if(down) Camera->setPos(posicao(Camera->pos)+posicao(0,0,1));
      break;

    case SDLK_h:
      if(down) Camera->setPos(posicao(Camera->pos)+posicao(0,0,-1));
      break; 
    case SDLK_p:
      if(down) {
        logfile <<"\n" << Camera->CameraToString() << "\n";
        }
      break; 

    default:
      break;
    }
  
}

// update camera
static void UpdateCamera(camera * Camera){
  Camera->atualizaPos();
}

// update player
static void UpdatePlayer(player * Player, torre * Torre){
  // update player
  //logfile << Player->PlayerToString() << "\n";
  tipoColisao colisaoTorre;
  colisaoTorre = Torre->ChecaColisaoPlayer(Player);
  if (Player->pos->y < -2){
    logfile << "\n Player caiu do mapa \n";
    estadoJogo = 3;
  }

  if (colisaoTorre == ColisaoAgressiva){
    logfile << "\n Player esmagado \n";
    estadoJogo = 3;
  }
  //logfile << "\n UpdatePlayer >> colisao com torre = " << colisaoTorre <<"\n";
  switch (Player->estado)
  {
  case Caindo:
    switch (colisaoTorre)
    {
    case ColisaoDeApoio:
      Player->para();
      //Player->estado2 = Normal;
      break;

    case ColisaoLateral:
      Player->estado2 = Pendurado;
      Player->para();
      break;

    case Vitoria:
      estadoJogo = 2;
      break;
    
    default:
      Player->atualizaPos();
      break;
    }


    break;

  case Movimento:
    Player->atualizaPos();
    break;
  
  default:
    
    break;
  }
}

// update lista
static void UpdateLista(LLBlocos * ListaUpdate, torre * Torre){  
  bool emMovimento = false;
  bool mudaEstado = false;
  int n = 0;
  // update lista
  if (! ListaUpdate->estaVazia()){
    //logfile << "\n Lista nao vazia \n";
    block * aux;
    block * b = ListaUpdate->lista; 
    while(b != nullptr){
      if (b->estado == Movimento) emMovimento = true;
      b->atualizaPos();
      if (b->estado == Caindo && emMovimento) {
        mudaEstado = true;
        n =(int) b->pos->y;
      }
      aux = b;
      b=b->prox;
      if (aux->estado != Movimento){
        tipoColisao colisao = Torre->ChecaColisao(aux);
        logfile << "\n" << aux->BlocoToString() << " \n";
        if (colisao == ColisaoDeApoio){
          logfile << "\n Colisao de apoio \n";
          aux->para();
          ListaUpdate->EjetaBloco(aux);
          Torre->adicionaBloco(aux);
          logfile << "\n lista: " << ListaUpdate->ListaToString() << "\n";
          logfile << "\n andar: " << Torre->retornaAndarN(aux->pos->y)->AndarToString() << "\n";
          logfile << "\n ------------\n";
        }
        else if (colisao == ColisaoAgressiva){
          logfile << "\n Deleta Bloco \n";
          ListaUpdate->EjetaBloco(aux);
          delete aux;
        }
      }
      if (mudaEstado) UpdateAndar(ListaUpdate, Torre, n+1);
    }
  }
  //else logfile << "\n----------lista update vazia\n";
}

static void process_events(struct GameData *GD){
    /* Our SDL event placeholder. */
    SDL_Event event;

    //UpdateCamera(GD->Camera);

    UpdateLista(GD->ListaUpdate, GD->Torre);

    UpdatePlayer(GD->Player, GD->Torre);

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

// void find_player(struct GameData *GD,
// 		 int *px, int *py, int *pz){
//   *px = GD->Player->x;
//   *py = GD->Player->andarAtual;
//   *pz = GD->Player->x;
// }

void fix_player_direction(player * Player){
  switch (Player->iRotacao)
  {
  case 0:
    glRotatef(90.0, 0.0, 1.0, 0.0 );
    break;
  case 1:
    glRotatef(180.0, 0.0, 1.0, 0.0 );
    break;
  case 2:
    glRotatef(-90.0, 0.0, 1.0, 0.0 );
    break;
  case 3:
    
    break;
  
  default:
    break;
  }
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
  //int px,py,pz,i,j,k;
  float di,dj,dk;
  //find_player(GD, &px, &py, &pz);

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  // We don't want to modify the projection matrix.
  glMatrixMode( GL_MODELVIEW );

  // desenha Torre
  for (andar * a = GD->Torre->primeiroAndar; a != nullptr; a = a->prox)
    for (block * b = a->Lista->lista; b != nullptr; b = b->prox){
      dk = b->pos->x - GD->Camera->pos->x;
      di = b->pos->y - GD->Camera->pos->y;
      dj = b->pos->z - GD->Camera->pos->z;
      // di = a->id - GD->cy;
      // dk = b->z - GD->cz;
      SetInitialView(GD->Camera->theta_y);      
      glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
      if (b->tipo == Fixo) draw_cube(d, tex, 5, 6);
      if (b->tipo == FinalFase) draw_cube(d, tex, 7, 8);
      else draw_cube(d, tex, 0, 1);
    }
  
  // desenha lista updates
  for (block * b = GD->ListaUpdate->lista; b != nullptr; b = b->prox){
      dk = b->pos->x - GD->Camera->pos->x;
      di = b->pos->y - GD->Camera->pos->y;
      dj = b->pos->z - GD->Camera->pos->z;
      SetInitialView(GD->Camera->theta_y);      
      glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
      if (b->tipo == Fixo) draw_cube(d, tex, 5, 6);
      else draw_cube(d, tex, 0, 1);
  }

  // desenha player
  SetInitialView(GD->Camera->theta_y);
  glTranslatef( 
    (GD->Player->pos->z - GD->Camera->pos->z)*d*2.0,
		(GD->Player->pos->y - GD->Camera->pos->y)*d*2.0,
		(GD->Player->pos->x - GD->Camera->pos->x)*d*2.0);
  fix_player_direction(GD->Player);
  DrawFluffy();
 
  //DrawMonsterPlant(tex);

  // SetInitialView(theta_y);
  // //glTranslatef(   -2*d*2.0,   -d +d*2.0 + 70.0,      0.0);
  // glTranslatef(   -4*d*2.0,   -d +d*2.0 + 70.0,      0.0);  
  // DrawRedDragon();
  
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
  //if (literate) printf("Cria estruturas\n");
  struct GameData GD;
  GD.Player = new player;
  GD.Torre = new torre;
  GD.ListaUpdate = new LLBlocos;
  GD.Camera = new camera;

  logfile.open ("logfile.txt");
  logfile << "Log\n";

  // ConfiguraMapTeclado();
  //  if (literate) printf("Configura SDL\n");
  /* Dimensions of our window. */
  int width =  1366; //1067; //512; //640;
  int height = 768; //600; //288; //480;
  /* Flags we will pass into SDL_SetVideoMode. */
  int flags = 0;
  SDL_Window *Window;
  GLuint tex[numTex];
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
  glGenTextures(6, tex);

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

  glBindTexture(GL_TEXTURE_2D, tex[5]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);  
  img = load_image((char *)"texture/block-fixo/1.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGBA, 512, 512, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE,
	       img->pixels);
  glBindTexture(GL_TEXTURE_2D, tex[6]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);  
  img = load_image((char *)"texture/block-fixo/5.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGBA, 512, 512, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE,
	       img->pixels);

// bloco final
  glBindTexture(GL_TEXTURE_2D, tex[7]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);  
  img = load_image((char *)"texture/block_final/1.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGBA, 512, 512, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE,
	       img->pixels);
// ---
  glBindTexture(GL_TEXTURE_2D, tex[8]);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
		  GL_TEXTURE_MIN_FILTER,
		  GL_LINEAR);  
  img = load_image((char *)"texture/block_final/2.png");
  glTexImage2D(GL_TEXTURE_2D, 0,
	       GL_RGBA, 512, 512, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE,
	       img->pixels);
  
  
  
  if (literate) printf("Carrega Mapa\n");
  //LoadMap(&GD, "mapa/01.txt");
  LoadMap(&GD, "mapa/fase01.txt");
  //PrintMap(&GD);
  /*
   * Now we want to begin our normal app process--
   * an event loop with a lot of redrawing.
   */
  if (literate) printf("Comeca jogo\n");
  while( estadoJogo == 1 ) {
    /* Process incoming events. */
    process_events(&GD);

    /* Draw the screen. */
    draw_screen(Window, &GD, tex);
  }
  if (estadoJogo == 2){
    printf("Vitoria!\n");
    logfile << "Vitoria!";
  }
  else if (estadoJogo == 3){
    printf("Derrota!\n");
    logfile << "Derrota!";
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

// music = Mix_LoadMUS(MY_COOL_OGG);
//   if(music == NULL) {
//     printf("Unable to load Ogg file: %s\n", Mix_GetError());
//     exit(1);
//   }
  
//   if(Mix_PlayMusic(music, 0) == -1) {
//     printf("Unable to play Ogg file: %s\n", Mix_GetError());
//     exit(1);
//   }
// int audio_rate = 22050;
//   Uint16 audio_format = AUDIO_S16SYS;
//   int audio_channels = 2;
//   int audio_buffers = 4096;
 
//   if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
//     fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
//     exit(1);
//   }

// static const char *MY_COOL_OGG = "SabatonSmokingSnakes.ogg";