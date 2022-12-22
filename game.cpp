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

#define numTex 12
#define passoCam 2
#define numFases 3
#define numTelaInstrucoes 5

#define PI 3.142857

// globais
//The music that will be played
Mix_Music *gMusic = NULL;

ofstream logfile;
int numTelaInstrucao = 1;
bool bloqueiaMov = false;

SDL_Rect arrowPosition;

enum estadosJogo {
  Menu,
  Instrucoes,
  Pause,
  EmJogo,
  Vitoria,
  Derrota,
  TelaFinal
  };
estadosJogo estadoJogo = Menu;


struct GameData{
  int fase;
  torre * Torre;
  player * Player;
  float zoom;
  float theta_y;
  int cx, cz;
  LLBlocos * ListaUpdate;
  desfaz * Desfaz;
};

struct Tela{
  estadosJogo nomeTela;
  SDL_Surface * background;
  int x1,x2;
  int y1,y2;
  bool selecionaOpcao1 = true;
  bool temOpcao2 = true;
  bool showArrow = true;
};

void CriaInstanciaDesfaz(struct GameData *GD){
  logfile << "\n CriaInstanciaDesfaz >> entra\n";
  GD->Desfaz->CriaInstancia(GD->Torre,GD->Player);
}

void AplicaDesfaz(struct GameData *GD){
  logfile << "\n AplicaDesfaz >> entra\n";
  if (GD->Desfaz->DesfazAcao(GD->Torre,GD->Player))
    logfile << "AplicaDesfaz >> sucesso\n";
  else
    logfile << "AplicaDesfaz >> nao pode desfazer\n";
}

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
  return loadedImage;  
}

void SetCamera (struct GameData *GD, string s){
    string delim = ",";

    auto start = 0U;
    auto end = s.find(delim);
    GD->cx = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    GD->cz = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    GD->theta_y = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    GD->zoom = stoi (s.substr(start, end));
    start = end + delim.length();
    end = s.find(delim, start);
}

string CameraToString (struct GameData *GD){
  string s = "";
  s += "Camera (";
  s += "cx = " + to_string(GD->cx);
  s += ", cz = " + to_string(GD->cz);
  s += ", theta = " + to_string(GD->theta_y);
  s += ", zoom = " + to_string(GD->zoom);
  s += ")";
  return s;
}
string ArrowToString (){
  string s = "";
  s += "Arrow (";
  s += "x = " + to_string(arrowPosition.x);
  s += ", y = " + to_string(arrowPosition.y);
  s += ")";
  return s;
}

void LoadMap (struct GameData *GD, string filename){
  ifstream arquivo;
  string line;
  GD->Desfaz->ClearDesfaz();

  arquivo.open (filename);
  if (arquivo.is_open()){
    getline (arquivo,line);
    GD->Player->SetPlayer(line);

    getline (arquivo,line);
    SetCamera(GD, line);

    arquivo.close();

    GD->Torre->SetTorre(filename);
  }  

  else logfile << "Erro em abrir arquivo\n"; 
   
}

static void quit_game( int code ){
  /*
   * Quit SDL so we can release the fullscreen
   * mode and restore the previous video settings,
   * etc.
   */
  // free music
  Mix_FreeMusic( gMusic );
  gMusic = NULL;

  Mix_Quit();
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
    b = atual->Lista->lista;
    while (b != nullptr)
    {
      aux = b;
      b = b->prox;
      if ((aux ->tipo==Movel || aux ->tipo==FinalMovel || aux ->tipo==FinalMovelCompleto)
         && ! base->temSuporte(aux)){
        logfile << "\n cai bloco \n ";
        houveMudanca = true;
        aux->setPosLim(posicao(aux->pos));
        aux->setVel(velocidade(0,0,0));
        atual->EjetaBloco(aux);
        ListaUpdate->AdicionaBloco(aux);
      }
    }
    if (houveMudanca) {
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
    Player->animacao = AnimAnda;
  }
  // com bloco na frente
  else {
    logfile << "ComBlocoNaFrente \n";
    // esta empurrando
    logfile << "bloco tipo:" << b->tipo <<"\n";
    if (push && (b->tipo == Movel || b->tipo == FinalMovel || b->tipo == FinalMovelCompleto)){
      // checa se pode
      logfile << "ChecaEmpurrar \n";
      posicao posBlocoFrente = posDesejada + velocidade(Player->rotacao);
      if (Torre->retornaBloco(posBlocoFrente) == nullptr){
        logfile << "Empurra \n";
        CriaInstanciaDesfaz(GD);
        b->mexe(posBlocoFrente, velocidade(Player->rotacao));
        Player->animacao = AnimEmpurra;
        Torre->EjetaBloco(b);
        GD->ListaUpdate->AdicionaBloco(b);
      }
    }
    else {
      // checa se pode subir
      logfile << "\n ChecaSePodeSubir \n";
      posicao posSubir = posDesejada + velocidade(0,1,0);
      posicao posAcima = posicao(Player->pos) + velocidade(0,1,0);
      if (Torre->retornaBloco(posSubir) == nullptr && Torre->retornaBloco(posAcima) == nullptr){
        logfile << "\n Sobe \n";
        Player->estado2 = Normal;
        Player->mexe(posSubir,velocidade(Player->rotacao) + velocidade(0,1,0));
        Player->animacao = AnimAnda;//seria anim pula
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
    logfile << "Sem bloco atras do player\n";
    // puxa se tiver bloco
    if (pull && blocoNaFrente != nullptr && (blocoNaFrente->tipo == Movel 
    || blocoNaFrente->tipo == FinalMovel || blocoNaFrente->tipo == FinalMovelCompleto)){
      logfile << "move_player_back >> puxa\n";
      CriaInstanciaDesfaz(GD);
      blocoNaFrente->mexe(posicao(Player->pos), velocidade(Player->rotacao)*(-1));
      Torre->EjetaBloco(blocoNaFrente);
      GD->ListaUpdate->AdicionaBloco(blocoNaFrente);
      Player->animacao = AnimPuxa;
    }
    else Player->animacao = AnimAnda;

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
    Player->animacao = AnimPenduradoDir;
  }
  else {
    dir = (dir+3)%4;
    Player->animacao = AnimPenduradoEsq;
  }
    
  
  rotacaoLado = Player->rotacoes[dir];

  posicao posDesejada = posicao(Player->pos) + velocidade(rotacaoLado);

  posicao apoioDesejado = posicao(Player->pos) + velocidade(rotacaoLado) + velocidade(Player->rotacao);

  bool temApoio = Torre->retornaBloco(apoioDesejado) != nullptr;
  bool posLivre = Torre->retornaBloco(posDesejada) == nullptr;

  if (temApoio && posLivre){
    Player->mexe(posDesejada,velocidade(rotacaoLado));
  }
  // se for canto
  else {
    if (posLivre){
      Player->mexe(posDesejada + velocidade(Player->rotacao),velocidade(rotacaoLado) + velocidade(Player->rotacao));
      if (right) Player->Rotaciona(1);
      else Player->Rotaciona(0);
    }
    else {
      Player->mexe(posicao(Player->pos),velocidade(0,0,0));
      if (right) Player->Rotaciona(0);
      else Player->Rotaciona(1);
    }
  }

}

void ProximaAcao (struct GameData *GD, struct Tela *tela){

  logfile << "\nProximaAcao << entrou \n";
  switch (tela->nomeTela)
  {
  case Menu:
    // inicia fase 1
    if (tela->selecionaOpcao1) {
      GD->fase = 1;
      LoadMap(GD, "mapa/fase0" + to_string(GD->fase) +".txt");
      estadoJogo = EmJogo;
    }

    // instrucoes
    else {
      estadoJogo = Instrucoes;
    }
    break;

  case Instrucoes:
    if (numTelaInstrucao<numTelaInstrucoes){
      numTelaInstrucao += 1;
    }
    else{
      numTelaInstrucao = 1;
      estadoJogo = Menu;
    }    
    break;

  case Vitoria:
    GD->fase +=1;
    if (GD->fase > numFases) estadoJogo = TelaFinal;
    else {
      LoadMap(GD, "mapa/fase0" + to_string(GD->fase) +".txt");
      estadoJogo = EmJogo;
    }
    break;

  case Derrota:
    logfile << "\nProximaAcao << GameOver \n";
    estadoJogo = Menu;
    break;

  case Pause:
    if (tela->selecionaOpcao1) {
      estadoJogo = EmJogo;
    }
    // instrucoes
    else {
      estadoJogo = Instrucoes;
    }
    break;

  case TelaFinal:
    estadoJogo = Menu;
    break;
  
  default:
    break;
  }
}

void PassaTelaInstrucao(struct Tela * tela){
  string dirArquivo = "texture/TelasInstrucoes2/" + to_string(numTelaInstrucao) +".png";
  static int numTela_atual = -1;
  if (numTela_atual != numTelaInstrucao){
    SDL_FreeSurface (tela->background);
    tela->background = load_image((char*)dirArquivo.c_str());
    numTela_atual = numTelaInstrucao;
  }
}

static void handle_key( SDL_KeyboardEvent *key, struct GameData *GD, bool down, 
  struct Tela *tela = NULL)
  {
  static bool hold_ctrl = false;
  player * Player = GD->Player;
  bool podeMover = Player->estado == Parado && estadoJogo==EmJogo;
  bool emTela = estadoJogo!=EmJogo && tela!=NULL;

  switch( key->keysym.sym ) {

    case SDLK_ESCAPE:
      if(down) quit_game( 0 );
      break;

    case SDLK_SPACE:      
      if(down && podeMover) AplicaDesfaz(GD);
      break;


    case SDLK_LEFT:
      if(down) {   
        if (podeMover){
          if (Player->estado2 == Pendurado){
            move_player_sideways(GD,1);
          }
          else{
            GD->Player->Rotaciona(0);
          }
        }
      }
      break;
      
    case SDLK_RIGHT:
      if(down) {
        if (podeMover){
          if (Player->estado2 == Pendurado){
            move_player_sideways(GD,0);
          }
          else{
            GD->Player->Rotaciona(1);
          }
        }
      }
      break;

    case SDLK_DOWN:
      if(down) {
        if (podeMover){
          if (Player->estado2 == Pendurado){
            Player->estado2 = Normal;
            Player->cai();
            logfile << "\n handle_key >> solta da beirada \n";
            logfile << Player->PlayerToString();
          }
          else
            move_player_back(GD, hold_ctrl);
        }
        if (emTela){
          logfile << "\n handle_key >> entrou \n";
          if (tela->temOpcao2) tela->selecionaOpcao1 = !tela->selecionaOpcao1;
        }
      }
      break;

    case SDLK_UP:
      if(down) {
        if (podeMover)
          move_player_front(GD, hold_ctrl);
        if (emTela){
          if (tela->temOpcao2) tela->selecionaOpcao1 = !tela->selecionaOpcao1;
        }
      }
      break;
    
    case SDLK_RETURN:
      if(down && emTela) {

        logfile << "\nHandle Key << proxima \n";
        ProximaAcao (GD,tela);
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
      if(down) GD->theta_y += 1.0;
      break;

    case SDLK_x:
      if(down) GD->theta_y -= 1.0;
      break; 
      
    case SDLK_a: // controle no plano
      if(down) {
        GD->zoom += passoCam;
      }
      break;

    case SDLK_s:
      if(down) {
        GD->zoom -= passoCam;
      }
      break; 

    case SDLK_o: // pause/play music
      if(down){
        logfile << "music" << "\n";
        //If there is no music playing
        if( Mix_PlayingMusic() == 0 )
        {
            //Play the music
            Mix_PlayMusic( gMusic, -1 );
        }
        //If music is being played
        else
        {
            //If the music is paused
            if( Mix_PausedMusic() == 1 )
            {
                //Resume the music
                Mix_ResumeMusic();
            }
            //If the music is playing
            else
            {
                //Pause the music
                Mix_PauseMusic();
            }
        }
      }
      break;

    case SDLK_p:
      if(down) {
        logfile << GD->Torre->TorreToString() << "\n";
        }
      break; 

    default:
      break;
    }
  
}

// update player
static void UpdatePlayer(player * Player, torre * Torre){
  tipoColisao colisaoTorre;
  colisaoTorre = Torre->ChecaColisaoPlayer(Player);
  if (Player->pos->y < -2){
    logfile << "\n Player caiu do mapa \n";
    estadoJogo = Derrota;
  }

  if (colisaoTorre == ColisaoAgressiva){
    logfile << "\n Player esmagado \n";
    estadoJogo = Derrota;
  }
  switch (Player->estado)
  {
  case Caindo:
    switch (colisaoTorre)
    {
    case ColisaoDeApoio:
      Player->para();
      Player->animacao = AnimNormal;
      break;

    case ColisaoLateral:
      Player->estado2 = Pendurado;
      Player->para();
      Player->animacao = AnimPendurado;
      break;

    case BlocoVitoria:
      estadoJogo = Vitoria;
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
static void UpdateLista(LLBlocos * ListaUpdate, torre * Torre, player * Player){  
  bool emMovimento = false;
  bool mudaEstado = false;
  int n = 0;
  // update lista
  if (! ListaUpdate->estaVazia()){
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
        logfile << "\nUpdateLista >> tipo de colisao: " << colisao << " \n";
        if (colisao == ColisaoDeApoio){
          aux->para();
          ListaUpdate->EjetaBloco(aux);
          Torre->adicionaBloco(aux);
          logfile << "\n ------------\n";

          if (Player->animacao == AnimEmpurra) Player->animacao = AnimNormal;
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
}

static void process_events(struct GameData *GD, struct Tela *tela = NULL){
    /* Our SDL event placeholder. */

    SDL_Event event;

    if (estadoJogo == EmJogo){
      UpdateLista(GD->ListaUpdate, GD->Torre, GD->Player);

      UpdatePlayer(GD->Player, GD->Torre);
    }

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {

      switch( event.type ) {
        case SDL_KEYDOWN:
          /* Handle key presses. */
          handle_key( &event.key, GD, true, tela);
          
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
		 struct GameData *GD,
		 GLuint tex[]){

  float d = 30.0;

  float di,dj,dk;

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  // We don't want to modify the projection matrix.
  glMatrixMode( GL_MODELVIEW );

  // desenha Torre
  for (andar * a = GD->Torre->primeiroAndar; a != nullptr; a = a->prox)
    for (block * b = a->Lista->lista; b != nullptr; b = b->prox){

      dj = b->pos->z - GD->cz;
      di = b->pos->y - GD->Player->pos->y;
      dk = b->pos->x - GD->cx;
      SetInitialView(GD->theta_y, GD->zoom);      
      glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
      if (b->tipo == Fixo) draw_cube(d, tex, 2, 3);
      else if (b->tipo == FinalFixo) draw_cube(d, tex, 4, 5);
      else if (b->tipo == FinalFixoCompleto) draw_cube(d, tex, 6, 7);
      else if (b->tipo == FinalMovel) draw_cube(d, tex, 8, 9);
      else if (b->tipo == FinalMovelCompleto) draw_cube(d, tex, 10, 11);
      else draw_cube(d, tex, 0, 1);
    }
  
  // desenha lista updates
  for (block * b = GD->ListaUpdate->lista; b != nullptr; b = b->prox){
      dj = b->pos->z - GD->cz;
      di = b->pos->y - GD->Player->pos->y;      
      dk = b->pos->x - GD->cx;
      SetInitialView(GD->theta_y, GD->zoom);       
      glTranslatef( dj*d*2.0, di*d*2.0, dk*d*2.0 );
      if (b->tipo == Fixo) draw_cube(d, tex, 2, 3);
      else if (b->tipo == FinalFixo) draw_cube(d, tex, 4, 5);
      else if (b->tipo == FinalFixoCompleto) draw_cube(d, tex, 6, 7);
      else if (b->tipo == FinalMovel) draw_cube(d, tex, 8, 9);
      else if (b->tipo == FinalMovelCompleto) draw_cube(d, tex, 10, 11);
      else draw_cube(d, tex, 0, 1);
  }

  // desenha player
  SetInitialView(GD->theta_y, GD->zoom);     
  glTranslatef( 
    (GD->Player->pos->z - GD->cz)*d*2.0,
		(GD->Player->pos->y - GD->Player->pos->y)*d*2.0,
		(GD->Player->pos->x - GD->cx)*d*2.0);
  fix_player_direction(GD->Player);
  // animacao player
  switch (GD->Player->animacao)
  {
  case AnimNormal:
    DrawFluffy();
    break;
  
  case AnimAnda:
    DrawFluffy_walk();
    break;
    
  case AnimPuxa:
    DrawFluffy_pull();
    break;
    
  case AnimEmpurra:
    if (GD->Player->animacaoAnterior != AnimEmpurra)
      DrawFluffy_push(true);
    else DrawFluffy_push(false);
    break;
    
  case AnimPendurado:
    DrawFluffy_hang();
    break;
    
  case AnimPenduradoDir:
    DrawFluffy_hangright();
    break;
    
  case AnimPenduradoEsq:
    DrawFluffy_hangleft();
    break;
  
  default:
    break;
  }
  GD->Player->animacaoAnterior = GD->Player->animacao;
  
  glEnable(GL_TEXTURE_2D);
  SDL_GL_SwapWindow(Window);
}


void draw_menu(SDL_Window *Window,
		 struct Tela *tela, SDL_Surface *arrow){
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // We don't want to modify the projection matrix.
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity( );
  
  if(tela->selecionaOpcao1){
    arrowPosition.x=tela->x1;
    arrowPosition.y=tela->y1;
  }
  else {
    arrowPosition.x=tela->x2;
    arrowPosition.y=tela->y2;
  }

  float fovy = 60.0*(3.14159/180.0);
  float height = 600;
  float focallength = (height/2.0)/tan(fovy/2.0);
  glRasterPos3f(-tela->background->w/2,
		-tela->background->h/2,
		-(focallength+0.1));
  glDrawPixels(tela->background->w,
	       tela->background->h,
	       GL_RGBA, GL_UNSIGNED_BYTE,
	       tela->background->pixels);

  if (tela->showArrow){
    glRasterPos3f(-tela->background->w/2 + arrowPosition.x,
      tela->background->h/2 - arrowPosition.y - arrow->h,
      -(focallength+0.001));
    glDrawPixels(arrow->w,
          arrow->h,
          GL_RGBA, GL_UNSIGNED_BYTE,
          arrow->pixels);
  }
  //Apply the arrow
  
  SDL_GL_SwapWindow(Window);
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
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
}

int main( int argc, char* argv[] ){
  struct GameData GD;
  GD.Player = new player;
  GD.Torre = new torre;
  GD.ListaUpdate = new LLBlocos;
  GD.Desfaz = new desfaz;

  GD.zoom = -300;
  GD.theta_y = -30.0;
  GD.cx = 3;
  GD.cz = 3;

  struct Tela TelaMenu;
  struct Tela TelaPause;
  struct Tela TelaGameOver;
  struct Tela TelaNext;
  struct Tela TelaInstrucoes;
  struct Tela TelaTelaFinal;

  logfile.open ("logfile.txt");
  logfile << "Log\n";

  /* Dimensions of our window. */
  int width =  1067; //1366; //512; //640;
  int height = 600; //768; //288; //480;
  /* Flags we will pass into SDL_SetVideoMode. */
  int flags = 0;
  SDL_Window *Window;
  GLuint tex[numTex];
  SDL_Surface *img = NULL;

  SDL_Surface *imgArrow = load_image((char*)"texture/arrow.png");

  TelaMenu.background = load_image((char*)"texture/TelasMenu/menu.png");
  TelaMenu.nomeTela = Menu;
  TelaMenu.x1=654;
  TelaMenu.y1=290;
  TelaMenu.x2=680;
  TelaMenu.y2=352;

  TelaPause.background = load_image((char*)"texture/TelasMenu/2_.png");
  TelaPause.nomeTela = Pause;
  TelaPause.x1=635;
  TelaPause.y1=288;
  TelaPause.x2=680;
  TelaPause.y2=349;

  TelaGameOver.background = load_image((char*)"texture/TelasMenu/3_.png");
  TelaGameOver.nomeTela = Derrota;
  TelaGameOver.x1=601;
  TelaGameOver.y1=288;
  TelaGameOver.temOpcao2 = false;

  TelaNext.background = load_image((char*)"texture/TelasMenu/4_.png");
  TelaNext.nomeTela = Vitoria;
  TelaNext.x1=702;
  TelaNext.y1=352;
  TelaNext.temOpcao2 = false;

  TelaInstrucoes.background = load_image((char*)"texture/TelasInstrucoes2/1.png");
  TelaInstrucoes.nomeTela = Instrucoes;
  TelaInstrucoes.temOpcao2 = false;
  TelaInstrucoes.showArrow = false;
  
  TelaTelaFinal.background = load_image((char*)"texture/TelasMenu/TelaFinal.png");
  TelaTelaFinal.nomeTela = Instrucoes;
  TelaTelaFinal.temOpcao2 = false;
  TelaTelaFinal.showArrow = false;
  
/* First, initialize SDL's video subsystem. */

  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) {
  fprintf( stderr, "Video initialization failed: %s\n",
  SDL_GetError( ) );
  quit_game( 1 );
  }
  //Initialize SDL_mixer
  if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
  {
      printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
  }
  

 //Load music
 //https://www.looperman.com/loops/detail/314853/subspace-club-type-sample-free-115bpm-disco-pad-loop
  gMusic = Mix_LoadMUS( "music/looperman-l-5041336-0314853-subspace-club-type-sample.wav" );
  
  Window = SDL_CreateWindow("O Pesadelo de Fluffy",
			    0, 0,
			    width, height,
			    SDL_WINDOW_OPENGL);
  assert(Window);
  SDL_GLContext Context = SDL_GL_CreateContext(Window);
  
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
    //set texturas etc
    if (1){
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
      glGenTextures(numTex, tex);

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

      glBindTexture(GL_TEXTURE_2D, tex[2]);
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

      glBindTexture(GL_TEXTURE_2D, tex[3]);
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
      glBindTexture(GL_TEXTURE_2D, tex[4]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
      img = load_image((char *)"texture/block_final_fixo/1.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);

    // ---
      glBindTexture(GL_TEXTURE_2D, tex[5]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
      img = load_image((char *)"texture/block_final_fixo/2.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);  
      
            
      // ---
glBindTexture(GL_TEXTURE_2D, tex[6]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);  
      img = load_image((char *)"texture/block_final_fixo/3.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);         
      // ---
      glBindTexture(GL_TEXTURE_2D, tex[7]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);  
      img = load_image((char *)"texture/block_final_fixo/4.png");
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
      img = load_image((char *)"texture/block_final_movel/1.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);  
      // ---
      glBindTexture(GL_TEXTURE_2D, tex[9]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);  
      img = load_image((char *)"texture/block_final_movel/2.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);  
      // ---
      glBindTexture(GL_TEXTURE_2D, tex[10]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);  
      img = load_image((char *)"texture/block_final_movel/3.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);  
      // ---
      glBindTexture(GL_TEXTURE_2D, tex[11]);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);  
      img = load_image((char *)"texture/block_final_movel/4.png");
      glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, 512, 512, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);
      
    }
  
  LoadMap(&GD, "mapa/fase01.txt");

  /*
   * Now we want to begin our normal app process--
   * an event loop with a lot of redrawing.
   */

  //Play the music
  Mix_PlayMusic( gMusic, -1 );

  while( 1 ) {

    if (estadoJogo == Menu){
      process_events(&GD,&TelaMenu);
      draw_menu(Window, &TelaMenu, imgArrow);
    }

    if (estadoJogo == Instrucoes){
      PassaTelaInstrucao(&TelaInstrucoes);
      process_events(&GD,&TelaInstrucoes);
      draw_menu(Window, &TelaInstrucoes, imgArrow);

    }

    if (estadoJogo == Pause){
      process_events(&GD,&TelaPause);
      draw_menu(Window, &TelaPause, imgArrow);
    }

    if (estadoJogo == EmJogo){
      process_events(&GD);
      draw_screen(Window, &GD, tex);
    }

    if (estadoJogo == Derrota){
      process_events(&GD,&TelaGameOver);
      draw_menu(Window, &TelaGameOver, imgArrow);
    }

    if (estadoJogo == Vitoria){
      process_events(&GD,&TelaNext);
      draw_menu(Window, &TelaNext, imgArrow);
    }

    if (estadoJogo == TelaFinal){
      process_events(&GD,&TelaTelaFinal);
      draw_menu(Window, &TelaTelaFinal, imgArrow);
    }
  }
  
  logfile.close();

  return 0;
}
