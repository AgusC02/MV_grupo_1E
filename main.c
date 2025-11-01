#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MV.h"

int main(int argc, char *argv[]){
    // VARIABLES

    TMV MV;
    char VecFunciones[CANTFUNC][5]; //5 Es la cantidad de caracteres que tiene como maximo el nombre de la funcion.
    char VecRegistros[CANTREG][4];

    // INICIO
    inicializoVecFunciones(VecFunciones);
    inicializoVecRegistros(VecRegistros);
    dep_arg(argc,argv,&MV);


    if(MV.disassembler){
        if (!strcmp(argv[2],"-d")){
            LeoInstruccionesDissasembler(&MV,VecFunciones,VecRegistros);
        }
    }
    LeoInstruccion(&MV);
    free(MV.archivovmi);

    //PARA DEBUGEAR
    //muestravaloresmv(MV);

return 0;
}
