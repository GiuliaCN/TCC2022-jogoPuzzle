#include "../Entidades.h"
#include <iostream>

using namespace std;
void printaPos(posicao p){
    cout << " Pos x=" <<p.x << ", y=" <<p.y << ", z=" <<p.z;
}

void printaVel(velocidade vel){
    cout << " Vel vx=" <<vel.vx << ", vy=" <<vel.vy<< ", vz=" <<vel.vz << endl;
}

void printaBloco(block * b){
    //cout << "\n------Print Bloco------\n";
    cout << "\nBloco: estado =" << b->estado << " " ;
    // printaPos(b->pos);
    // printaVel(b->vel);
    if (b->prox==nullptr)
        cout << " Bloco sem prox" << endl;
    else
        cout << " Bloco com prox" << endl;
    
    //cout << "------Fim Print Bloco------\n\n";
}

// void printaAndar(andar *a){
//     cout << "\n------Print andar------\n";
//     cout << "Andar id=" << a->id << endl;

//     for (block * prox = a->Lista; prox != nullptr; prox = prox->prox){
//         printaBloco(*prox);
//     }    
//     cout << "------Fim Print andar------\n\n";
// }

block * criaB () {
    posicao p (0.0,1.0,2.2);
    block * b = new block(p);
    return b;
}

void testeBloco (){
    cout << "Cria bloco" << endl;
    block * B = criaB();
    cout << "Bloco criado"<< endl;
    printaBloco(B);

    // cout << "Teste - funcao estaEmCoordenada" << b.estaEmCoordenada(1,3) << endl;

    // cout << "Teste - linka em outro bloco" << endl;
    // block c (2,2);
    // b.prox = &c;
    // printaBloco(b);

}

void testeAndar () {
    cout << "Cria lista '011101/110001'" << endl;
    andar a("011101/110001",1);
    cout << "Lista criada" << endl;
    //cout << a.AndarToString() << endl;
    
    cout << "Teste - tem bloco em (0,0) : " << a.coordenadaOcupada(posicao(0,1,0)) << " / false" << endl;
    cout << "Teste - tem bloco em (1,0) : " << a.coordenadaOcupada(posicao(1,1,0)) << " / true" << endl;
    cout << "Teste deleta bloco em (1,0)" << endl;
    a.RemoveBloco(posicao(1,1,0));
    cout << "Teste - tem bloco em (1,0) : " << a.coordenadaOcupada(posicao(1,1,0)) << " / false" << endl;

    cout << a.AndarToString() << endl;
}

// void testeTorre () {
//     cout << "Cria torre\n\n" << endl;
//     torre * Torre = new torre;
//     Torre->SetTorre("mapa/01.txt");
//     // while (Torre->andarAtual != nullptr){
//     //     printaAndar(Torre->andarAtual);
//     //     Torre->sobeAndar();
//     // }

//     cout << "Teste - tem bloco em (3,3) : " << Torre->andarAtual->coordenadaOcupada(3,3) << " / true" << endl;
//     Torre->andarAtual->RemoveBloco(3,3);
//     cout << "Teste - tem bloco em (3,3) : " << Torre->andarAtual->coordenadaOcupada(3,3) << " / false" << endl;
    
// }

int main () {
    //testeBloco();
    testeAndar();
    //testeTorre();
    return 0;
}