// Entidades.h

#ifndef ENTIDADES_H // include guard
#define ENTIDADES_H

#include <iostream>
using namespace std;

#define NUM_DESFAZ 9
#define eps 0.05

#define velQueda 0.1
#define velEnt 0.1
#define velBlock 0.1

/*
 * ColisaoDeApoio = bloco diretamente abaixo e em cruz ()
 * ColisaoLateral = bloco em frente do player
 * ColisaoAgressiva = entidades no mesmo lugar
 * ColisaoMista = colisao lateral + colisao apoio
*/
enum tipoColisao { SemColisao, ColisaoDeApoio, ColisaoLateral, ColisaoAgressiva, ColisaoMista, BlocoVitoria };

enum estadosPlayer { Morto, Pendurado, Normal, TentaPendurar};

enum animacaoPlayer {AnimNormal, AnimPendurado, AnimPenduradoEsq, AnimPenduradoDir,
    AnimEmpurra, AnimPuxa, AnimAnda};

enum tipoBloco { Movel, Fixo, FinalFase};
/*
 * Parado = bloco diretamente abaixo
 * Caindo = só para com colisao
 * Movimento = para caso colisao ou atingindo posicao limite
*/
enum estadosEntidade { Parado, Caindo, Movimento};

class posicao {
    public:
        posicao();
        posicao(posicao * p);
        posicao(double _x, double _y, double _z){
            x = _x;
            y = _y;
            z = _z;
        };
        double x;
        double y;
        double z;

        posicao operator+ (posicao const &obj){
            posicao p;
            p.x = x + obj.x;
            p.y = y + obj.y;
            p.z = z + obj.z;
            return p;
        }

        posicao operator- (posicao const &obj){
            posicao p;
            p.x = x - obj.x;
            p.y = y - obj.y;
            p.z = z - obj.z;
            return p;
        };
        posicao operator= (posicao const &obj){ return obj; };

        bool igualXZ (posicao p);
        posicao Aproximado();
        string PosicaoToString();
};

class velocidade {
    public:
        velocidade();
        velocidade(velocidade * v);
        velocidade(int _vx, int _vy, int _vz){
            vx = _vx;
            vy = _vy;
            vz = _vz;
        }
        int vx;
        int vy;
        int vz;

        velocidade& operator+ (velocidade const &obj){
            vx = vx + obj.vx;
            vy = vy + obj.vy;
            vz = vz + obj.vz;
            return *this;
        }
        velocidade operator- (velocidade const &obj){
            velocidade res;
            res.vx = vx - obj.vx;
            res.vy = vy - obj.vy;
            res.vz = vz - obj.vz;
            return res;
        };
        velocidade operator= (posicao const &obj);
        string VelocidadeToString();
};

bool operator== (posicao const &obj, posicao const &obj1);
bool operator<= (posicao const &obj, posicao const &obj1);


// relacionar velocidade, magnitude e posicao
posicao operator* (velocidade const &obj, double const &obj1);

velocidade operator* (velocidade const &obj, int const &obj1);

inline posicao operator+ (posicao const &obj1, velocidade const &obj){
    posicao p;
    p.x = obj1.x + obj.vx;
    p.y = obj1.y + obj.vy;
    p.z = obj1.z + obj.vz;
    return p;
};

inline posicao operator- (posicao const &obj1, velocidade const &obj){
    posicao p;
    p.x = obj1.x - obj.vx;
    p.y = obj1.y - obj.vy;
    p.z = obj1.z - obj.vz;
    return p;
};

class entidade
{
    public:
        posicao * pos;
        posicao * posLim;
        velocidade * vel;
        double vMag;
        velocidade rotacoes[4] = {velocidade(0,0,1),velocidade(-1,0,0),velocidade(0,0,-1),velocidade(1,0,0)};

        entidade();
        entidade(posicao p, velocidade v, double _vMag){ pos = new posicao(p); vel = new velocidade(v); vMag = _vMag; }
        ~entidade();

        estadosEntidade estado;

        //virtual void mexe() = 0;
        void atualizaPos();
        void mexe(posicao _posLim, velocidade _vel);
        bool emMovimento();
        bool emQueda() { return vel->vy != 0; }
        tipoColisao colisao(entidade * e);
        void entraEmMovimento (posicao pLim, velocidade v, double vM);
        void para();
        void cai();
        void setVel(velocidade v);
        void setPos(posicao p);
        void setPosLim(posicao p);
        string EstadoToString();

};

class camera
{
    public:
    camera();
    ~camera();

    float zoom;
    float theta_y;
    int cx, cz;

    void SetCamera(string s);
    void ZoomIn();
    void ZoomOut();

    string CameraToString();

};

class player: public entidade
{
    private:

    public:
        int iRotacao;
        player();
        ~player();

        velocidade * rotacao;
        estadosPlayer estado2;
        animacaoPlayer animacao;
        animacaoPlayer animacaoAnterior;
        posicao * posAgarrar;

        void setAgarrar (posicao p);

        void Rotaciona(bool clockwise);
        //void mexe(posicao _posLim, velocidade _vel);
        void SetPlayer(string s);
        void setRotacao(velocidade r);
        string PlayerToString();
        string EstadoPlayerToString();
        string AnimacaoPlayerToString();
        
};

class block: public entidade
{
    // private:
    //     int x,y,z;
    public:
        block();
        block(posicao p, int t);
        ~block();

        block * prox;
        block * ant;
        tipoBloco tipo;

        string BlocoToString();
        //void mexe(posicao _posLim, velocidade _vel);
        bool estaEmCoordenada(posicao p);
};

// lista ligada de blocos
class LLBlocos
{
    private:
        void deletaBlocoRec(block * b);
    public:
        LLBlocos(){ lista = nullptr; };
        ~LLBlocos();

        // primeiro da lista
        block * lista;

        // LLBlocos& operator+(LLBlocos const &obj)

        block * RetornaBloco(posicao p);
        void EjetaBloco(block * b);
        void AdicionaBloco(posicao p, int t);
        void AdicionaBloco(block * b);
        void RemoveBloco(posicao p);
        void RemoveBloco(block * b);
        void UneListas(LLBlocos * LLB);

        bool contem (posicao p){ return RetornaBloco(p) == nullptr? false : true; }
        bool contem (block * b){ return contem( posicao(b->pos)); }
        bool estaVazia() { return lista == nullptr; }
        string ListaToString();

};

class andar
{
    private:
        void deletaBlocoRec(block * b);
    public:
        andar ();
        andar(string s, int n);
        ~andar();

        // lista de andares
        andar * prox;
        andar * ant;

        LLBlocos * Lista;
        int id;

        bool coordenadaOcupada (posicao p);
        void AdicionaBloco(posicao p, int t);
        void AdicionaBloco(block * b);
        void RemoveBloco(posicao p);
        bool temSuporte (block * b);
        block * RetornaBloco(posicao p);
        void EjetaBloco(block * b);
        string AndarToString();
        tipoColisao ColisaoAndar(entidade * e);
};

class torre
{
    private:
        void deletaAndarRec(andar * a);
        int nAndares;       
        void adicionaAndar(string s);
    public:
        torre();
        ~torre();

        andar * primeiroAndar;
        andar * andarAtual;

        // bool posicaoLivre (posicao p);
        block * retornaBloco (posicao p);
        void EjetaBloco (block * b);
        void DeletaBloco (posicao p);
        andar * retornaAndarN (int n);
        void SetTorre (string filename);
        void vaiParaAndar (int n);
        void sobeAndar ();
        void desceAndar ();
        LLBlocos * updateAndar (andar * a);
        //void Copia (torre * t);
        //void Reset();
        void adicionaBloco (block * b);
        LLBlocos * EjetaBlocosSemSuporte(andar * a);
        tipoColisao ChecaColisao(block * b);
        tipoColisao ChecaColisaoPlayer(player * pl);

        string TorreToString();

        bool coordenadaOcupada (posicao p);
};

// class desfaz
// {
//     private:
//         torre * ListaTorreInstancias[NUM_DESFAZ];
//         player * ListaPlayerInstancias[NUM_DESFAZ];
//         int indexAtual;
//     public:
//         desfaz();
//         void CriaInstancia(torre * T, player * P);
//         bool DesfazAcao(torre * T, player * P); // false se não conseguir (numero maximo de desfaz)
// };

#endif /* ENTIDADES_H */