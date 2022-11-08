#include "Entidades.h"
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

// compara posicoes considerando margem de erro eps
bool operator== (posicao const &obj, posicao const &obj1){
    if (obj.x >= obj1.x + eps || obj.x <= obj1.x - eps)
        return false;
    if (obj.y >= obj1.y + eps || obj.y <= obj1.y - eps)
        return false;
    if (obj.z >= obj1.z + eps || obj.z <= obj1.z - eps)
        return false;
    return true;
}

// compara posicoes considerando margem de erro eps, true se "caiu" mais que y
bool operator<= (posicao const &obj, posicao const &obj1){
    if (obj.x >= obj1.x + eps || obj.x <= obj1.x - eps)
        return false;
    if (obj.y >= obj1.y + eps)
        return false;
    if (obj.z >= obj1.z + eps || obj.z <= obj1.z - eps)
        return false;
    return true;
}

// relacionar velocidade, magnitude e posicao
posicao operator* (velocidade const &obj, double const &obj1){
    posicao res;
    res.x = obj.vx * obj1;
    res.y = obj.vy * obj1;
    res.z = obj.vz * obj1;
    return res;
};

bool posicao::igualXZ (posicao p){
    p.y = y;
    return *this == p;
}

// aproxima posicao para valores inteiros
posicao posicao::Aproximado(){
    return posicao(round(x), round(y), round(z));
}

velocidade velocidade::operator= (posicao const &obj){
    velocidade res;
    res.vx = (int) round(obj.x);
    res.vy = (int) round(obj.y);
    res.vz = (int) round(obj.z);
    return res;
}

posicao::posicao(){}
posicao::posicao(posicao * p){
    x = p->x;
    y = p->y;
    z = p->z;
}
velocidade::velocidade(){}
velocidade::velocidade(velocidade * v){
    vx = v->vx;
    vy = v->vy;
    vz = v->vz;
}

string posicao::PosicaoToString(){
    string s = "";
    s += "(" + to_string(x) + ", ";
    s += to_string(y) + ", ";
    s += to_string(z) + ")";
    return s;
}

string velocidade::VelocidadeToString(){
    string s = "";
    s += "(" + to_string(vx) + ", ";
    s += to_string(vy) + ", ";
    s += to_string(vz) + ")";
    return s;
}

// ------------------------------------------ ENTIDADE -----------------------------------

entidade::entidade(){}
entidade::~entidade(){}

void entidade::setVel(velocidade v){
    vel->vx = v.vx;
    vel->vy = v.vy;
    vel->vz = v.vz;
}
void entidade::setPos(posicao p){
    pos->x = p.x;
    pos->y = p.y;
    pos->z = p.z;
}
void entidade::setPosLim(posicao p){
    posLim->x = p.x;
    posLim->y = p.y;
    posLim->z = p.z;
}
// true se |vel| != 0
bool entidade::emMovimento(){
    if (vel->vx != 0 || vel->vy != 0 || vel->vz != 0) return true;
    return false;
}

// modifica pos com vel e vMag, checa se chega em destino
void entidade::atualizaPos(){
    setPos(posicao(pos) + velocidade(vel)*vMag);    
    if (posicao(pos) <= posicao(posLim)) cai();
}

void entidade::para() { 
    setPos(pos->Aproximado()); 
    setVel(velocidade(0,0,0)); 
    vMag = 0.0; 
    estado = Parado; 
}
void entidade::cai() { 
    setVel(velocidade(0,-1,0)); 
    vMag = velQueda; 
    estado = Caindo; 
}

string entidade::EstadoToString(){
    switch (estado)
    {
    case Parado:
        return "Parado";
        break;
    case Caindo:
        return "Caindo";
        break;
    case Movimento:
        return "Movimento";
        break;
    
    default:
        return "";
        break;
    }
}

// checa o tipo de colisao entre entidades
tipoColisao entidade::colisao(entidade * e){

    // aproximacao de posicoes
    posicao posAprox = pos->Aproximado();
    posicao posAproxE = e->pos->Aproximado();

    // colisaoAgressiva - mesma posicao aproximada (bloco cae em player)
    if (posAprox == posAproxE) return ColisaoAgressiva;

    // colisaoCentral - bloco logo abaixo
    if (posAprox - posicao(0,1,0) == posAproxE) return ColisaoCentral;

    // colisaoLateral - mesmo y
    for (int i = 0; i < 4; i++)
        if (posAprox + rotacoes[i] == posAproxE) return ColisaoLateral;

    // colisaoDeApoio - X centrado no chao da entidade, "e" não pode estar em movimento
    if (! e->emMovimento()) {
        posAprox = posAprox - posicao(0,1,0);
        if (posAprox == posAproxE) return ColisaoDeApoio;
        for (int i = 0; i < 4; i++)
            if (posAprox + rotacoes[i] == posAproxE) return ColisaoDeApoio;
    }

    return SemColisao;
}


// ------------------------------------------ PLAYER -----------------------------------

player::player(){}

void player::setRotacao(velocidade r){
    rotacao->vx = r.vx;
    rotacao->vy = r.vy;
    rotacao->vz = r.vz;
}

string player::EstadoPlayerToString(){
    switch (estado2)
    {
    case Morto:
        return "Morto";
        break;
    
    case Pendurado:
        return "Pendurado";
        break;
    
    case Normal:
        return "Normal";
        break;
    
    default:
        return "";
        break;
    }
}

string player::PlayerToString(){
    string s = "";
    s += "Player (";
    //(estado ="+ to_string(estado + ", estado2 =" + estado2;
    s += "Rotacao =" + rotacao->VelocidadeToString();
    s += ", Pos = " + pos->PosicaoToString();
    s += ", Vel = " + vel->VelocidadeToString();
    s += ", Estado = " + EstadoToString();
    s += ", Estado2 = " + EstadoPlayerToString();
    s += ")";
    return s;
}

void player::SetPlayer(string s){
    pos = new posicao;
    posLim = new posicao;
    vel = new velocidade;
    rotacao = new velocidade;
    estado = Parado;
    estado2 = Normal;

    posicao p;
    iRotacao = 0;
    string delim = ",";

    auto start = 0U;
    auto end = s.find(delim);
    p.x = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    p.y = stoi (s.substr(start, end - start));
    start = end + delim.length();
    end = s.find(delim, start);

    p.z = stoi (s.substr(start, end));
    start = end + delim.length();
    end = s.find(delim, start);

    setPos(p);

    setRotacao(rotacoes[iRotacao]);
}

void player::Rotaciona(bool clockwise){
    if (clockwise)
        iRotacao = (iRotacao+1)%4;
    else
        iRotacao = (iRotacao+3)%4;

    setRotacao(rotacoes[iRotacao]);
}

// void player::Copia(player * p){
//     iRotacao = p->iRotacao;
//     x=p->x;
//     z=p->z;
//     andarAtual = p->andarAtual;
//     rotacao[0]= p->rotacao[0];
//     rotacao[1]= p->rotacao[1];
//     estaCaindo = p->estaCaindo;
//     estaPendurado = p->estaPendurado;
// }

void player::mexe(posicao _posLim){
    estado = Movimento;
    setVel(velocidade(rotacao));
    vMag = velPlayer;
    posLim->x = _posLim.x;
    posLim->y = _posLim.y;
    posLim->z = _posLim.z;
}

// ------------------------------------------ BLOCO -----------------------------------
block::block(){}

block::block(posicao p) {
    pos= new posicao(p);
    vel = new velocidade();
    posLim = new posicao();
    prox = nullptr; 
    ant = nullptr; 
    estado = Parado;
}

void block::mexe(posicao _posLim, velocidade _vel){
    estado = Movimento;
    setVel(_vel);
    vMag = velBlock;
    setPosLim(_posLim);
}

bool block::estaEmCoordenada(posicao p){
    return *pos == p;
    // return pos.igualXZ(p);
}

block::~block(){
    //printf("\n --- Bloco [x=%d, z=%d] apagado\n",x,z);
}


// ------------------------------------------ LLBlocos -----------------------------------
LLBlocos::~LLBlocos(){
    deletaBlocoRec(lista);
}

void LLBlocos::AdicionaBloco(block * b){
    if (lista == nullptr) lista = b;
    else{
        b->ant = nullptr;
        lista->ant = b;
        b->prox = lista;
        lista = b;
    }
}

void LLBlocos::AdicionaBloco(posicao p){
    block * Bloco = new block(p);
    AdicionaBloco(Bloco);
}

void LLBlocos::RemoveBloco(block * b){
    // se é o primeiro da lista
    if (b->ant == nullptr){
        lista = b->prox;
    }

    // se é o ultimo
    else if (b->prox == nullptr)
        b->ant->prox = nullptr;

    // se esta no meio
    else {
        b->ant->prox = b->prox;
        b->prox->ant = b->ant;
    }

    delete b;
}

void LLBlocos::RemoveBloco(posicao p){
    block * b = RetornaBloco(p);
    if (b != nullptr)
        RemoveBloco(b);
}

block * LLBlocos::RetornaBloco(posicao p){
    block * b;
    for (block * b = lista; b != nullptr; b = b->prox){
        if (posicao(b->pos) == p) return b;
    }
    return nullptr;
}

void LLBlocos::deletaBlocoRec(block * b){
    if (b->prox != nullptr) deletaBlocoRec(b->prox);
    delete b;
}

void LLBlocos::EjetaBloco(block * b){
    if (b->ant != nullptr){
        b->ant->prox = b->prox;
        if (b->prox != nullptr) b->prox->ant = b->ant;
    }
    else { 
        // checa se é o primeiro da lista
        if (lista->pos == b->pos) lista = b->prox;
    }
}

// LLBlocos& LLBlocos::operator+ (LLBlocos const &obj){
//     if (obj.lista != nullptr){
//         block * b;
//         for (b = obj.lista; b->prox != nullptr; b=b->prox){}
//         b->prox = lista;
//         lista = b;
//     }
//     return *this;
// }


// ------------------------------------------ ANDAR ------------------------------------------

andar::andar(){}

bool andar::coordenadaOcupada (posicao p){ return Lista->contem(p); }
void andar::AdicionaBloco(posicao p) { Lista->AdicionaBloco(p); }
void andar::AdicionaBloco(block * b) { Lista->AdicionaBloco(b); }
void andar::RemoveBloco(posicao p){ Lista->RemoveBloco(p); }
block * andar::RetornaBloco(posicao p) { return Lista->RetornaBloco(p); }
void andar::EjetaBloco(block * b) { Lista->EjetaBloco(b); }

string andar::AndarToString(){
    string s = "";
    s+="Andar id =" + to_string(id) + " ;";
    int count = 0;
    for (block * b = Lista->lista; b!= nullptr; b=b->prox) count++;
    s+="Numero de blocos =" + to_string(count) + " ;";

    int i = 0;
    for (block * b = Lista->lista; b != nullptr; b = b->prox ){
        i++;
        s+=to_string(i) + "o Bloco = " + b->pos->PosicaoToString() + ";";    
    }
    //s+="1o Bloco = " + Lista->lista->pos.PosicaoToString();    
    return s;
}

// cria andar apartir de string que representa matriz, linhas separadas por /
andar::andar(string s, int n){
    prox = nullptr;
    ant = nullptr;
    Lista = new LLBlocos();
    id = n;
    //cout << "\nrecebido: "<<s<<endl;
    int x = -1; // pois s começa com '/'
    int z = 0;
    char c;
    posicao p;
    p.y = n;
    for (int i = 0; i < s.size() ; i++){
        c = s[i];
        // inicia nova linha
        if (c == '/' || c == '-') {
            p.x += 1;
            p.z = 0;
        }
        else {
            if (c == '1'){
                //cout << "criar bloco em "<< p.PosicaoToString() <<endl;
                AdicionaBloco(p);
            }
            p.z+=1;
        } 
    }
}

// void andar::Copia(andar * a){
//     id = a->id;
//     prox = nullptr;
//     ant = nullptr;
//     for (block * p = a->Lista; p != nullptr; p = p->prox){
//         this->AdicionaBloco(p->x,p->z);
//     }
// }

// void andar::Reset(){
//     id = 0;
//     ant = nullptr;
//     prox = nullptr;
//     deletaBlocoRec(Lista);
// }

bool andar::temSuporte (entidade * e){
    posicao p = posicao(e->pos);
    if (coordenadaOcupada(p)) return true;
    for (int i = 0; i < 4; i++)
        if (coordenadaOcupada(p + e->rotacoes[i])) return true;
    return false;
}

tipoColisao andar::ColisaoAndar(entidade * e){
    return SemColisao;
}

andar::~andar(){
    // this->Reset();
}



// ------------------------------------------ TORRE ------------------------------------------
torre::torre (){
    //settings basicos:
    primeiroAndar=nullptr;
    nAndares=0;
}

string torre::TorreToString(){
    string s = "";
    s+="Torre:\n - Numero Andares = " + to_string(nAndares) + "\n";
    s+=primeiroAndar->AndarToString();
    // for (andar * a = primeiroAndar; a!= nullptr; a = a->prox)
    //     s+= a->AndarToString() + "\n";
    return s;
}

// adiciona bloco no andar id = b->pos.y
// se bloco em mov, para o bloco, se nao tem andar em y, destroi bloco
void torre::adicionaBloco (block * b){
    if (b->emMovimento()) b->para();

    posicao aprox = b->pos->Aproximado();
    b->setPos(aprox);

    if (b->pos->y < 1 || b->pos->y > nAndares) delete b;
    else {
        andar * a = retornaAndarN((int) aprox.y);
        a->AdicionaBloco(b);
    }
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

// void torre::Copia (torre * t){
//     andar * ant;
//     nAndares = t->nAndares;
//     for (int i=1; i <= nAndares; i++){
//         andar * a = new andar;
//         a->Copia(t->retornaAndarN(i));
//         if (i == 1){
//             primeiroAndar = a;
//             ant = a;
//         }
//         else {
//             ant->prox = a;
//             a->ant = ant;
//         }
//     }
//     andarAtual = this->retornaAndarN(t->andarAtual->id);
// }

block * torre::retornaBloco (posicao p){
    posicao posAprox = p.Aproximado();
    andar * a = retornaAndarN((int) posAprox.y);
    if (a == nullptr) return nullptr;
    return a->RetornaBloco(posAprox);
}

void torre::EjetaBloco(block * b){
    posicao posAprox = b->pos->Aproximado();
    andar * a = retornaAndarN((int) posAprox.y);
    a->EjetaBloco(b);
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

// void torre::Reset(){
//     deletaAndarRec(primeiroAndar);
//     primeiroAndar=nullptr;
//     nAndares=0;
// }

LLBlocos * torre::EjetaBlocosSemSuporte(andar * a){
    LLBlocos * BlocosSemSuporte = new LLBlocos;
    if (a == nullptr) return BlocosSemSuporte;
    // percorre lista
    andar * base = a->ant;
    if (base != nullptr) {
        block * b;
        for (b = a->Lista->lista; b != nullptr; b=b->prox)
            if (! base->temSuporte(b)) {
                a->EjetaBloco(b);
                BlocosSemSuporte->AdicionaBloco(b);
            }
    }
    return BlocosSemSuporte;
}

LLBlocos * torre::updateAndar(andar * a){
    LLBlocos * BlocosParaUpdate = new LLBlocos;
    for (LLBlocos * llb = EjetaBlocosSemSuporte(a); 
            llb->lista != nullptr;
                llb = EjetaBlocosSemSuporte(a->prox)){
        // une listas ligadas
        // BlocosParaUpdate = BlocosParaUpdate + llb;
        if (llb->lista != nullptr){
            block * b;
            for (b = llb->lista; b->prox != nullptr; b=b->prox){}
            b->prox = BlocosParaUpdate->lista;
            BlocosParaUpdate->lista->ant = b;
        }
    }
    return BlocosParaUpdate;
}

// checa colisao com algum bloco da torre - apenas base
tipoColisao torre::ChecaColisao(entidade * e){
    posicao pAprox = e->pos->Aproximado();
    // proximo o bastante para poder interagir
    if (pAprox == posicao(e->pos)){
        andar * a = retornaAndarN((int) pAprox.y - 1);

        if (a == nullptr) return ColisaoAgressiva; // bloco ja saiu da torre, deve ser destruido?

        if (a->temSuporte(e)) return ColisaoDeApoio;
    }
    return SemColisao;    
}

// ------------------------------------------ DESFAZ ------------------------------------------
// desfaz::desfaz(){
//     indexAtual = 0;
//     for(int i = 0; i < NUM_DESFAZ; i++){
//         ListaPlayerInstancias[i] = nullptr;
//         ListaTorreInstancias[i] = nullptr;
//     }
// }
//
// void desfaz::CriaInstancia(torre * T, player * P){
//     indexAtual ++;
//     indexAtual = indexAtual%NUM_DESFAZ;
//
//     torre * Tnovo = new torre;
//     player * Pnovo = new player;
//
//     Tnovo->Copia(T);
//     Pnovo->Copia(P);
//
//     ListaTorreInstancias[indexAtual] = Tnovo;
//     ListaPlayerInstancias[indexAtual] = Pnovo;
// }
//
// bool desfaz::DesfazAcao(torre * T, player * P){
//     indexAtual --;
//     if (indexAtual < 0){
//         indexAtual = 0;
//         return false;
//     }
//
//     T->Reset();
//
//     T->Copia(ListaTorreInstancias[indexAtual]);
//     P->Copia(ListaPlayerInstancias[indexAtual]);
//
//     return true;
// }
