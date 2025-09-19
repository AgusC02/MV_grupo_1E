#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MV.h"

int main(int argc, char *argv[]){
    // VARIABLES

    TMV MV;
    char VecFunciones[CANTFUNC][5]; //5 Es la cantidad de caracteres que tiene como maximo el nombre de la funcion.
    char VecRegistros[CANTREG][4];
    char nomarch[50];


    // INICIO
    strcpy(nomarch,argv[1]);
    LeoArch(nomarch,&MV);
    inicializoVecFunciones(VecFunciones);
    inicializoVecRegistros(VecRegistros);

    if(argc>2){
        if (!strcmp(argv[2],"-d")){
            LeoInstruccionesDissasembler(&MV,VecFunciones,VecRegistros);
        }
    }
    LeoInstruccion(&MV);

    //PARA DEBUGEAR
    //muestravaloresmv(MV);

return 0;
}
