#include "../Entidades.h"
#include <iostream>

using namespace std;

void printaBloco(block b){
    //cout << "\n------Print Bloco------\n";
    cout << "\nBloco x=" <<b.x << ", z=" << b.z << endl;
    if (b.prox==nullptr)
        cout << "Bloco sem prox" << endl;
    else
        cout << "Bloco com prox" << endl;
    
    //cout << "------Fim Print Bloco------\n\n";
}

void printaAndar(andar *a){
    cout << "\n------Print andar------\n";
    cout << "Andar id=" << a->id << endl;

    for (block * prox = a->Lista; prox != nullptr; prox = prox->prox){
        printaBloco(*prox);
    }    
    cout << "------Fim Print andar------\n\n";
}

void testeBloco (){
    cout << "Cria bloco em x=1 z=3" << endl;
    block b (1,3);
    cout << "Bloco criado"<< endl;
    printaBloco(b);

    cout << "Teste - funcao estaEmCoordenada" << b.estaEmCoordenada(1,3) << endl;

    cout << "Teste - linka em outro bloco" << endl;
    block c (2,2);
    b.prox = &c;
    printaBloco(b);

}

void testeAndar () {
    cout << "Cria lista '/011101/110001'" << endl;
    andar a("/011101/110001",1);
    cout << "Lista criada" << endl;
    //printaAndar(&a);

    cout << "Teste - tem bloco em (0,0) : " << a.coordenadaOcupada(0,0) << " / false" << endl;
    cout << "Teste - tem bloco em (1,0) : " << a.coordenadaOcupada(1,0) << " / true" << endl;
    cout << "Teste deleta bloco em (1,0)" << endl;
    a.RemoveBloco(1,0);
    cout << "Teste - tem bloco em (1,0) : " << a.coordenadaOcupada(1,0) << " / false" << endl;
}

void testeTorre () {
    cout << "Cria torre\n\n" << endl;
    torre * Torre = new torre;
    Torre->SetTorre("mapa/01.txt");
    // while (Torre->andarAtual != nullptr){
    //     printaAndar(Torre->andarAtual);
    //     Torre->sobeAndar();
    // }

    cout << "Teste - tem bloco em (3,3) : " << Torre->andarAtual->coordenadaOcupada(3,3) << " / true" << endl;
    Torre->andarAtual->RemoveBloco(3,3);
    cout << "Teste - tem bloco em (3,3) : " << Torre->andarAtual->coordenadaOcupada(3,3) << " / false" << endl;
    
}

int main () {
    //testeBloco();
    testeTorre();
    return 0;
}