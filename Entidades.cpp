#include "Entidades.h"
#include <iostream>
#include <fstream>

using namespace std;

// ------- PLAYER -------

void player::SetPlayer(string s){
    estaCaindo = false;
    estaPendurado=false;
    iRotacao = 0;
    //5,1,4
    //std::string s = "scott>=tiger";
    string delim = ",";

    auto start = 0U;
    auto end = s.find(delim);
    x = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    andarAtual = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    z = stoi (s.substr(start, end));
    start = end + delim.length();
    end = s.find(delim, start);

    rotacao[0] = rotacoes[iRotacao][0];
    rotacao[1] = rotacoes[iRotacao][1];

}

void player::Rotaciona(bool clockwise){
    if (clockwise)
        iRotacao = (iRotacao+1)%4;
    else
        iRotacao = (iRotacao+3)%4;

    rotacao[0] = rotacoes[iRotacao][0];
    rotacao[1] = rotacoes[iRotacao][1];

}

void player::Copia(player * p){
    iRotacao = p->iRotacao;
    x=p->x;
    z=p->z;
    andarAtual = p->andarAtual;
    rotacao[0]= p->rotacao[0];
    rotacao[1]= p->rotacao[1];
    estaCaindo = p->estaCaindo;
    estaPendurado = p->estaPendurado;
}

// ------- FIM PLAYER -------


// ------- ANDAR -------

andar::andar(){}
// cria andar apartir de string que representa matriz, linhas separadas por /
andar::andar(string s, int n){
    prox = nullptr;
    ant = nullptr;
    Lista = nullptr;
    id = n;
    //cout << "\nrecebido: "<<s<<endl;
    int x = -1; // pois s começa com '/'
    int z = 0;
    char c;

    for (int i = 0; i < s.size() ; i++){
        c = s[i];
        // inicia nova linha
        if (c == '/' || c == '-') {
            x += 1;
            z = 0;
        }
        else {
            if (c == '1'){
                AdicionaBloco(x,z);
            }
            z+=1;
        } 
    }
}

void andar::AdicionaBloco(int x, int z){
    //cria bloco
    block * Bloco = new block(x,z);

    if (Lista==nullptr) Lista = Bloco;
    else {
        //pega ultimo da lista
        block * ant = nullptr;
        for (block * p = Lista; p != nullptr; p = p->prox){
            ant=p;
        }

        //conecta bloco no fim da lista
        ant->prox = Bloco;
    }
}

void andar::RemoveBloco(int x, int z){
    block * ant = nullptr;
    block * p = nullptr;
    for (p = Lista; p != nullptr && !(p->estaEmCoordenada(x, z)); p = p->prox){
        ant=p;
    }
    //se achou
    if (p!=nullptr){

        // caso seja o primeiro bloco da lista
        if (ant == nullptr)
            Lista = p->prox;
        else 
            ant->prox=p->prox; //isola p da lista
        delete p;
        //destroi bloco
        //p->~block();
    }

}

bool andar::coordenadaOcupada (int x, int z){
    for (block * prox = Lista; prox != nullptr; prox = prox->prox){
        if (prox->estaEmCoordenada(x, z)) return true;
    }
    return false;
}

void andar::Copia(andar * a){
    id = a->id;
    prox = nullptr;
    ant = nullptr;

    for (block * p = a->Lista; p != nullptr; p = p->prox){
        this->AdicionaBloco(p->x,p->z);
    }
}

void andar::deletaBlocoRec(block * b){
    if (b->prox != nullptr) deletaBlocoRec(b->prox);
    delete b;
}

void andar::Reset(){
    id = 0;
    ant = nullptr;
    prox = nullptr;
    deletaBlocoRec(Lista);
}

bool andar::temSuporte (block * b){
    int x, z;
    x = b->x;
    z = b->z;
    if (coordenadaOcupada(x,z) || coordenadaOcupada(x+1,z) || coordenadaOcupada(x-1,z)
        || coordenadaOcupada(x,z+1) || coordenadaOcupada(x,z-1))
        return true;
    return false;
}

block * andar::RetornaBloco(int x, int z){
    for (block * prox = Lista; prox != nullptr; prox = prox->prox){
        if (prox->estaEmCoordenada(x, z)) return prox;
    }
    return nullptr;
}

andar::~andar(){
    this->Reset();
}

// ------- FIM ANDAR -------


// ------- BLOCO -------

block::block(int _x, int _z){
    x =_x;
    z=_z;
    prox = nullptr;
}

bool block::estaEmCoordenada(int _x, int _z){
    if (x != _x) return false;
    if (z != _z) return false;
    return true;
}

block::~block(){
    //printf("\n --- Bloco [x=%d, z=%d] apagado\n",x,z);
}

// ------- FIM BLOCO -------


// ------- TORRE -------
torre::torre (){
    //settings basicos:
    primeiroAndar=nullptr;
    nAndares=0;
}

andar * torre::retornaAndarN (int n){
    for (andar * p = primeiroAndar; p != nullptr; p = p->prox){
        if (p->id == n) return p;
    }
    return nullptr;
}

void torre::SetTorre(string filename){
    //abre arquivo
    ifstream arquivo;
    string line;
    string conjunto = "";

    arquivo.open (filename);
    if (!arquivo.is_open()) perror ("Error opening file");

    // primeira linha eh info do player (pega e joga fora)
    getline (arquivo,line);
    //cout << line << "\n";

    // cria matriz andar apartir de arquivo
    while ( getline (arquivo,line) )
    {
        //cout << line << "\n";

        //marca fim de um andar
        if (line == "-"){
            //cout << "\nandar criado de: " << conjunto << "\n\n";
            adicionaAndar(conjunto);
            conjunto = "";
        }
        else {
            conjunto = conjunto + '/' + line;
        }           
    }

    andarAtual = retornaAndarN(1);
}

void torre::vaiParaAndar(int n){
    andarAtual = retornaAndarN(n);
}

void torre::sobeAndar(){
    andarAtual = andarAtual->prox;
}

void torre::desceAndar(){
    andarAtual = andarAtual->ant;
}

void torre::Copia (torre * t){
    andar * ant;
    nAndares = t->nAndares;
    for (int i=1; i <= nAndares; i++){
        andar * a = new andar();
        a->Copia(t->retornaAndarN(i));
        if (i == 1){
            primeiroAndar = a;
            ant = a;
        }
        else {
            ant->prox = a;
            a->ant = ant;
        }
    }
    andarAtual = this->retornaAndarN(t->andarAtual->id);
}

// recebe string para criar andar
void torre::adicionaAndar(string s){
    if (s.size() != 0){
        nAndares+=1;

        andar * p; // ponteiros para busca na lista
        andar * atual; 

        andar *Novo = new andar(s, nAndares);

        if (primeiroAndar==nullptr) primeiroAndar=Novo;
        else{
            for (p=primeiroAndar; p != nullptr; p=p->prox){
                atual = p;
            }
            atual->prox=Novo;
            Novo->ant = atual;
        }
    }
}

void torre::deletaAndarRec(andar * a){
    if (a->prox != nullptr) deletaAndarRec(a->prox);
    delete a;
}

void torre::Reset(){
    deletaAndarRec(primeiroAndar);
    primeiroAndar=nullptr;
    nAndares=0;
}

void torre::updateAndar(int n){
    bool houveMudanca = false;
    block * delB;
    block * b;

    andar * base = retornaAndarN(n-1);
    andar * atual = retornaAndarN(n);

    b = atual->Lista;

    while ( b != nullptr){
        if(!(base->coordenadaOcupada(b->x,b->z))||(base->coordenadaOcupada(b->x+1,b->z))||
        (base->coordenadaOcupada(b->x-1,b->z))||(base->coordenadaOcupada(b->x,b->z+1))||
        (base->coordenadaOcupada(b->x,b->z-1))){
            base->AdicionaBloco(b->x,b->z);
            delB = b;
            b=b->prox;

            atual->RemoveBloco(delB->x,delB->z);
        }
    }

    if (houveMudanca) updateAndar(n+1);
}

// ------- FIM TORRE -------


// ------- DESFAZ -------
desfaz::desfaz(){
    indexAtual = 0;
    for(int i = 0; i < NUM_DESFAZ; i++){
        ListaPlayerInstancias[i] = nullptr;
        ListaTorreInstancias[i] = nullptr;
    }
}

void desfaz::CriaInstancia(torre * T, player * P){
    indexAtual ++;
    indexAtual = indexAtual%NUM_DESFAZ;

    torre * Tnovo = new torre();
    player * Pnovo = new player;

    Tnovo->Copia(T);
    Pnovo->Copia(P);

    ListaTorreInstancias[indexAtual] = Tnovo;
    ListaPlayerInstancias[indexAtual] = Pnovo;
}

bool desfaz::DesfazAcao(torre * T, player * P){
    indexAtual --;
    if (indexAtual < 0){
        indexAtual = 0;
        return false;
    }

    T->Reset();

    T->Copia(ListaTorreInstancias[indexAtual]);
    P->Copia(ListaPlayerInstancias[indexAtual]);

    return true;
}
// ------- FIM DESFAZ -------