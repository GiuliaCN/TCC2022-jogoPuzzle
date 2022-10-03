// Entidades.h

#ifndef ENTIDADES_H // include guard
#define ENTIDADES_H

#include <iostream>
using namespace std;

#define NUM_DESFAZ 9

class block
{
    // private:
    //     int x,y,z;
    public:
        block (int _x, int _z);
        ~block();
        int x;
        int z;
        block * prox;
        bool estaEmCoordenada(int _x, int _z);
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

        block * Lista;
        int id;
        bool coordenadaOcupada (int x, int z);
        void AdicionaBloco(int x, int z);
        void RemoveBloco(int x, int z);
        void Copia (andar * a);
        void Reset();
        bool temSuporte (block * b);
        block * RetornaBloco(int x, int z);
};

class torre
{
    private:
        void deletaAndarRec(andar * a);
        int nAndares;
        
        void adicionaAndar(string s);
    public:
        andar * retornaAndarN (int n);
        torre ();
        andar * primeiroAndar;
        void SetTorre (string filename);
        andar * andarAtual;
        void vaiParaAndar (int n);
        void sobeAndar ();
        void desceAndar ();
        void updateAndar (int n);
        void Copia (torre * t);
        void Reset();
};

class player
{
    private:
    public:
        int rotacoes[4][2] = {{0,1},{-1,0},{0,-1},{1,0}};
        int iRotacao;
        int x;
        int z;
        int andarAtual;
        int rotacao[2];
        bool estaPendurado;
        bool estaCaindo;
        //enum estado; //juntar booleanas, podeControlar, caindo, animações, morto
        void Rotaciona(bool clockwise);
        void Mexe(bool estaComBloco);
        void SetPlayer(string s);
        void Copia(player * p);
};

class desfaz
{
    private:
        torre * ListaTorreInstancias[NUM_DESFAZ];
        player * ListaPlayerInstancias[NUM_DESFAZ];
        int indexAtual;

    public:
        desfaz();
        void CriaInstancia(torre * T, player * P);
        bool DesfazAcao(torre * T, player * P); // false se não conseguir (numero maximo de desfaz)

};

#endif /* ENTIDADES_H */