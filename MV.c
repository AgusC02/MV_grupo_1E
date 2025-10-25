#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MV.h"

void iniciasubrutina(TMV *MV){
    int posicionfisicaSP,i,aux;
    posicionfisicaSP=direccionamiento_logtofis(MV,(*MV).R[SP],0);

    aux=MV->punteroargv;
    for (i=3;i>=0;i--){
        //aux=MV->punteroargv;
        MV->MEM[posicionfisicaSP--]=aux & 0xFF;
        aux=aux >> (8*i);
    }

    aux=MV->argc;
    for (i=3;i>=0;i--){

        MV->MEM[posicionfisicaSP--]=aux & 0xFF;
        aux=aux >> (8*i);
    }

    for (i=3;i>=0;i--){ //AGREGA EL RET -1 (-1 siempre)
        MV->MEM[posicionfisicaSP--]=0xFF;
    }

    MV->R[SP]-=11;

}

void mododebug(TMV *MV){
    char comando;

    comando=getchar();
    if (comando == 'q') {
        exit(0);
    } else if (comando == 'g') {
        MV->flagdebug = 0;  //  SE DESACTIVA EL MODO DEBUG
    }
    // Si es enter, se sigue paso a paso sin tocar el flag
}

void generarImagen(TMV MV){
    int i=0;
    FILE *f;
    unsigned char mem_kib;
    unsigned int reg;
    f=fopen(MV.archivovmi,"wb");

    if(!f){
        printf("ERROR AL ABRIR ARCHIVO DE IMAGEN \n");
        generaerror(99);
    }

    fwrite("VMI25",1,5,f);
    fputc(1,f);
    mem_kib=MV.mem_size/1024;
    fputc((mem_kib>>8)&0xFF,f);
    fputc(mem_kib & 0xFF,f);

     // Registros (64 bytes)
    for(i=0;i<CANTREG;i++){
        reg=MV.R[i];
        fputc((reg>>24)&0xFF, f);
        fputc((reg>>16)&0xFF, f);
        fputc((reg>>8)&0xFF, f);
        fputc((reg)&0xFF, f);
    }

    // TDS (32 bytes)
    for (i=0;i<CANTMAXSEGMENTOS;i++){
        reg=MV.TDS[i];
        fputc((reg>>24)&0xFF, f);
        fputc((reg>>16)&0xFF, f);
        fputc((reg>>8)&0xFF, f);
        fputc((reg)&0xFF, f);
    }
    // MEMORIA (variable)
    fwrite(MV.MEM,1,MV.mem_size,f);
    fclose(f);
}

void init_mem0(TMV *MV){
    int i;
    for (i=0;i<TMEM;i++){
        MV->MEM[i]=0;
    }
}

void init_reg0(TMV *MV){
    int i;
    for (i=0;i<CANTREG;i++){
        MV->R[i]=0;
    }
}

void init_tds0(TMV *MV){
    int i;
    for (i=0;i<CANTMAXSEGMENTOS;i++){
        MV->TDS[i]=0;
    }
}

void inicializoMVen0(TMV *MV){
    init_mem0(MV);
    init_reg0(MV);
    init_tds0(MV);

}

void initparametrosMV(TMV *MV){
    /*
    ESTA FUNCION SOLO DEBE SER LLAMADA POR DEP_ARG,
    */
    MV->size_paramsegment=0;
    MV->disassembler=0;
    MV->argc=0;
    MV->punteroargv=-1;
    MV->archivovmi=NULL;
    MV->flagdebug=0;
}

void armaParamSegment(TMV *MV,int argc, char *argv[],int *paramsize){
/*
    ESTA FUNCION TIENE QUE ARMAR EL PARAM SEGMENT Y
    CALCULAR SU TAMAÑO EN BYTES Y DEVOLVERLO EN *paramsize para setear la MV
    [EN PROCESO]

    El segmento esta compuesto por los strings parametros y al final
    un arreglo argv de tamaño argc de punteros de 4 bytes

    Precondicion: argc>0 por lo tanto argv!=NULL
    Postcondicion: Deja armado el paramsegment de la MV,
                     y devuelve el tamaño en bytes del segmento en *paramsize
*/
    int i,j,memindx,sizestr=0,tam=0;
    int vectorindices[100];
    char *auxiliar;
    unsigned char aux_uchar;

    memindx=0;

    for (i=0;i<argc;i++){
        sizestr=0;
        sizestr+=(strlen(argv[i])+1);
        auxiliar=malloc(strlen(argv[i]+1));
        strcpy(auxiliar,argv[i]);

        vectorindices[i]=memindx;

        for(j=0;j<sizestr;j++){
            MV->MEM[memindx]=auxiliar[j];
            memindx++;
        }

        tam+=sizestr;
        free(auxiliar);
    }
    //Despues de este for ya tenemos los strings copiados
    //Defino el puntero a argv que le voy a pasar a la maquina virtual para armar la subrutina principal.
    //Este es el indice porque la posicion del paramsegment es 0 en el tds
    // y el memindx es el offset. Para integrarlo mejor en otro momento habria que buscar otra forma.
    // por ahora va a quedar asi.
    MV->punteroargv=memindx;
    //-----------------------

    for(i=0;i<argc;i++){

        for(j=3;j>=0;j--){
            aux_uchar = vectorindices[i]>>(8*j);
            MV->MEM[memindx]= aux_uchar;
            memindx++;
        }
    }

    //Despues de este doble for ya tenemos el arreglo de argumentos cargado.
    MV->argc=argc;
    tam+= argc*4;
    *paramsize=tam;
}

void dep_arg(int argc, char *argv[], TMV *MV){
    /*
    * Funcion para depurar los argumentos e inicializar la maquina virtual
    */
   int tammem=16;
   int cant_params=0;
   char vmx,vmi,disassembler=0;
   int argindx;
   char *archivo_vmx = NULL;
   char *archivo_vmi = NULL;
   char archivo[50];
   char **vectorparams = NULL;
   int paramsize=0;

   initparametrosMV(MV);

    for (argindx=1; argindx<argc;argindx++){

        if(strncmp(argv[argindx],"m=",2)==0){ // Checkea m=M
            tammem=atoi(argv[argindx]+2);
        }
        else if (strcmp(argv[argindx],"-d")==0){ // Checkea disassembler
            disassembler=1;
        }
        else if (archivo_vmx == NULL && strstr(argv[argindx],".vmx")){ //Checkea .vmx
            vmx=1;
            archivo_vmx= malloc(strlen(argv[argindx])+1);
            strcpy(archivo_vmx,argv[argindx]);
        }
        else if (archivo_vmi == NULL && strstr(argv[argindx],".vmi")){ //Checkea .vmi
            vmi=1;
            archivo_vmi= malloc(strlen(argv[argindx])+1);
            strcpy(archivo_vmi,argv[argindx]);
        }
        else if(vmx==1 && strcmp(argv[argindx],"-p")==0){ //Checkea -p
            // lo que viene despues son los parametros
            cant_params= argc-argindx-1;
            vectorparams=&argv[argindx+1];

            //vectorparams apunta al primer parametro y cant_params tiene la cantidad.
            if(cant_params>0)
                armaParamSegment(MV,cant_params,vectorparams,&paramsize);
        }
    }
    MV->size_paramsegment=paramsize;
    MV->mem_size=tammem*1024;
    MV->disassembler=disassembler;

    if(vmx){
        if(archivo_vmx!=NULL){
            strcpy(archivo,archivo_vmx);
        }
        if(vmi){
            MV->archivovmi=malloc(sizeof(strlen(archivo_vmi)+1));
            strcpy(MV->archivovmi,archivo_vmi);
        }
    }
    else if (vmi && !vmx){
        if(archivo_vmi!=NULL){
            strcpy(archivo,archivo_vmi);
            MV->archivovmi=malloc(sizeof(strlen(archivo_vmi)+1));
            strcpy(MV->archivovmi,archivo_vmi);
        }
    }
    LeoArch(archivo,MV);
    free(archivo_vmi);
    free(archivo_vmx);
}

void initregsegmentos(TMV *MV){
    int i;
    for(i=1;i<IP;i++){
        MV->R[i]=-1;
    }
}

void agregasegmentos(unsigned short int tam, int reg_indx,TMV *MV, int *tds_indx, int sizeac){
    if(tam>0){
        //------------GUARDA TDS--------------------------
        MV->TDS[*tds_indx]=sizeac;
        MV->TDS[*tds_indx]<<= 16; // quedo la base, posición donde inicia el segmento
        MV->TDS[*tds_indx] |= (tam & 0xFFFF);

        //------------GUARDA REGISTROS--------------------
        MV->R[reg_indx]= (*tds_indx);
        MV->R[reg_indx] <<= 16;
        if(reg_indx == SS){//CUANDO INICIALIZO SS INICIALIZO SP TAMBIEN.
            MV->R[SP]= (*tds_indx);
            MV->R[SP] <<= 16;
            MV->R[SP] |= (tam & 0xFFFF);
        }
        (*tds_indx)++;
    }
    else
        MV->R[reg_indx]=-1;

}

void inicializoTDS(TMV* MV,theader header){
short int TamCS;
  int indicetds=0,sizeac=0;

  TamCS=header.tamCS;

  if (header.version==1){
        MV->mem_size=TMEM;
        MV->TDS[0]=TamCS;
        MV->TDS[1]=TamCS << 16;
        MV->TDS[1]+=(MV->mem_size-TamCS);

  } else if (header.version==2){
        // Agregue dos parametros flag en MV (mem_size y param).
        initregsegmentos(MV);

        agregasegmentos(MV->size_paramsegment,-1,MV,&indicetds,sizeac);
        sizeac+=MV->size_paramsegment;  //esto tendria que ser el tamaño del paramsegm

        agregasegmentos(header.tamKS,KS,MV,&indicetds,sizeac);
        sizeac+=header.tamKS;

        agregasegmentos(header.tamCS,CS,MV,&indicetds,sizeac);
        sizeac+=header.tamCS;

        agregasegmentos(header.tamDS,DS,MV,&indicetds,sizeac);
        sizeac+=header.tamDS;

        agregasegmentos(header.tamES,ES,MV,&indicetds,sizeac);
        sizeac+=header.tamES;

        agregasegmentos(header.tamSS,SS,MV,&indicetds,sizeac);
        sizeac+=header.tamSS;

        if (sizeac>MV->mem_size)
            generaerror(ERRMEM);

        MV->R[IP]=(MV->R[CS] & 0xFFFF0000) | header.entrypointoffset;
        if(MV->R[SS]!=-1)
            MV->R[SP]=MV->R[SS] + header.tamSS -1; // Habria que checkear si existe SS primero
        iniciasubrutina(MV); // Puede estar dentro del if de arriba creo.
  }
}

void inicializoRegistros(TMV *MV,theader header){

    if (header.version==1){
        MV->R[CS]=0X00000000;
        MV->R[DS]=0X00010000;  //DS               //PARA INICIALIZAR EL DS TENDRIA QUE USAR LA TABLA DE SEGMENTOS EN UN FUTURO PORQUE NO SIEMPRE VA A ESTAR EN TDS[1]
        MV->R[IP]=MV->R[CS]; //IP
    }
}

void initheadervmx(theader *head){
    (*head).tamCS=0;
    (*head).tamDS=0;
    (*head).tamES=0;
    (*head).tamSS=0;
    (*head).tamKS=0;
    (*head).entrypointoffset=-1;
}

void agregoalconstantsegment(TMV *MV,int offset, unsigned char c_agregable){
    int direccion;

    direccion=direccionamiento_logtofis(MV,MV->R[KS]+offset,0);
    MV->MEM[direccion]=c_agregable;

}

void generaerror(int tipo){
    if(tipo==ERRDIV0)
        printf("ERROR DIVISION POR 0");
    if(tipo==ERRINVINST)
        printf("ERROR INSTRUCCION INVALIDA");
    if(tipo==ERRSEGMF)
        printf("ERROR FALLO DE SEGMENTO");
    if(tipo==ERRMEM)
        printf("MEMORIA INSUFICIENTE");
    if(tipo==ERRSTOVF)
        printf("STACK OVERFLOW");
    if(tipo==ERRSTUNF)
        printf("STACK UNDERFLOW");
    if(tipo==99)
        printf("ERROR APERTURA DE ARCHIVO");
    if(tipo ==0xF)
        printf("Error MALLOC");
    abort();
}

void inicializoVecFunciones(char VecFunciones[CANTFUNC][5]){
    //2 Operandos
    strcpy(VecFunciones[16], "MOV");
    strcpy(VecFunciones[17], "ADD");
    strcpy(VecFunciones[18], "SUB");
    strcpy(VecFunciones[19], "MUL");
    strcpy(VecFunciones[20], "DIV");
    strcpy(VecFunciones[21], "CMP");
    strcpy(VecFunciones[22], "SHL");
    strcpy(VecFunciones[23], "SHR");
    strcpy(VecFunciones[24], "SAR");
    strcpy(VecFunciones[25], "AND");
    strcpy(VecFunciones[26], "OR");
    strcpy(VecFunciones[27], "XOR");
    strcpy(VecFunciones[28], "SWAP");
    strcpy(VecFunciones[29], "LDL");
    strcpy(VecFunciones[30], "LDH");
    strcpy(VecFunciones[31], "RND");

    //1 Operando
    strcpy(VecFunciones[0], "SYS");
    strcpy(VecFunciones[1], "JMP");
    strcpy(VecFunciones[2], "JZ");
    strcpy(VecFunciones[3], "JP");
    strcpy(VecFunciones[4], "JN");
    strcpy(VecFunciones[5], "JNZ");
    strcpy(VecFunciones[6], "JNP");
    strcpy(VecFunciones[7], "JNN");
    strcpy(VecFunciones[8], "NOT");
    strcpy(VecFunciones[11], "PUSH");
    strcpy(VecFunciones[12], "POP");
    strcpy(VecFunciones[13], "CALL");
    //0 Operandos
    strcpy(VecFunciones[14], "RET");
    strcpy(VecFunciones[15], "STOP");


}

void inicializoVecRegistros(char VecRegistros[CANTREG][4]){
    strcpy(VecRegistros[LAR], "LAR");
    strcpy(VecRegistros[MAR], "MAR");
    strcpy(VecRegistros[MBR], "MBR");
    strcpy(VecRegistros[IP], "IP");
    strcpy(VecRegistros[OPC], "OPC");
    strcpy(VecRegistros[OP1], "OP1");
    strcpy(VecRegistros[OP2], "OP2");
    strcpy(VecRegistros[SP], "SP");
    strcpy(VecRegistros[BP], "BP");
    strcpy(VecRegistros[9], "");
    strcpy(VecRegistros[EAX], "A");    // Parte 2 es sin la X
    strcpy(VecRegistros[EBX], "B");
    strcpy(VecRegistros[ECX], "C");
    strcpy(VecRegistros[EDX], "D");
    strcpy(VecRegistros[EEX], "E");
    strcpy(VecRegistros[EFX], "F");
    strcpy(VecRegistros[AC], "AC");
    strcpy(VecRegistros[CC], "CC");
    strcpy(VecRegistros[18], "");
    strcpy(VecRegistros[19], "");
    strcpy(VecRegistros[20], "");
    strcpy(VecRegistros[21], "");
    strcpy(VecRegistros[22], "");
    strcpy(VecRegistros[23], "");
    strcpy(VecRegistros[24], "");
    strcpy(VecRegistros[25], "");
    strcpy(VecRegistros[CS], "CS");
    strcpy(VecRegistros[DS], "DS");
    strcpy(VecRegistros[ES], "ES");
    strcpy(VecRegistros[SS], "SS");
    strcpy(VecRegistros[KS], "KS");
    strcpy(VecRegistros[PS], "PS");
}

void declaroFunciones(TFunc Funciones){
//2 OPERANDOS
  Funciones[16]=MOV;
  Funciones[17]=ADD;
  Funciones[18]=SUB;
  Funciones[19]=MUL;
  Funciones[20]=DIV;
  Funciones[21]=CMP;
  Funciones[22]=SHL;
  Funciones[23]=SHR;
  Funciones[24]=SAR;
  Funciones[25]=AND;
  Funciones[26]=OR;
  Funciones[27]=XOR;
  Funciones[28]=SWAP;
  Funciones[29]=LDL;
  Funciones[30]=LDH;
  Funciones[31]=RND;

//1 OPERANDO
  Funciones[0]=SYS;
  Funciones[1]=JMP;
  Funciones[2]=JZ;
  Funciones[3]=JP;
  Funciones[4]=JN;
  Funciones[5]=JNZ;
  Funciones[6]=JNP;
  Funciones[7]=JNN;
  Funciones[8]=NOT;
  Funciones[11]=PUSH;
  Funciones[12]=POP;
  Funciones[13]=CALL;

//0 OPERANDOS
  Funciones[14]=RET;
  Funciones[15]=STOP;
}

void LeoArch(char nomarch[],TMV *MV){
  FILE *arch;
  unsigned char leo;
  theader header;
  theadervmi headervmi;
  int offsetks=0;
  int poscs,offsetcs=0;

  int j,i=0;
  //Inicializa header para lectura de datos (los tamaños de segmentos en -1)
  initheadervmx(&header);
  //DEBO PREPARAR ARCHIVO PARA LECTURA
  arch = fopen(nomarch,"rb");
  if(!arch)
    generaerror(99);
  fread(&header.c1,sizeof(char),1,arch);
  fread(&header.c2,sizeof(char),1,arch);
  fread(&header.c3,sizeof(char),1,arch);
  fread(&header.c4,sizeof(char),1,arch);
  fread(&header.c5,sizeof(char),1,arch);
  fread(&header.version,sizeof(char),1,arch);
  fread(&leo,sizeof(char),1,arch);
  header.tamCS=leo;
  header.tamCS=header.tamCS<<8;
  fread(&leo,sizeof(char),1,arch);
  header.tamCS+=leo;
  if(header.c1=='V' && header.c2 =='M' && header.c3=='X' && header.c4=='2' && header.c5=='5'){
    if (header.version==1){
    MV->version=1;
    inicializoTDS(MV,header);
    inicializoRegistros(MV,header);
    while(!feof(arch)){
        fread(&(MV->MEM[i]),1,1,arch);
        i++;
    }
    }
    else if (header.version==2){
        MV->version=2;
        fread(&leo,sizeof(char),1,arch);
        header.tamDS=leo;
        header.tamDS=header.tamDS<<8;
        fread(&leo,sizeof(char),1,arch);
        header.tamDS+=leo;

        fread(&leo,sizeof(char),1,arch);
        header.tamES=leo;
        header.tamES=header.tamES<<8;
        fread(&leo,sizeof(char),1,arch);
        header.tamES+=leo;

        fread(&leo,sizeof(char),1,arch);
        header.tamSS=leo;
        header.tamSS=header.tamSS<<8;
        fread(&leo,sizeof(char),1,arch);
        header.tamSS+=leo;

        fread(&leo,sizeof(char),1,arch);
        header.tamKS=leo;
        header.tamKS=header.tamKS<<8;
        fread(&leo,sizeof(char),1,arch);
        header.tamKS+=leo;

        fread(&leo,sizeof(char),1,arch);
        header.entrypointoffset=leo;
        header.entrypointoffset=header.entrypointoffset<<8;
        fread(&leo,sizeof(char),1,arch);
        header.entrypointoffset+=leo;


        inicializoTDS(MV,header);

        for(i=0;i<header.tamCS;i++){
            //Leo el codigo maquina del programa
            fread(&leo,sizeof(char),1,arch);
            poscs=direccionamiento_logtofis(MV,MV->R[CS]+offsetcs,0);
            MV->MEM[poscs]=leo;
            offsetcs++;
        }

        //Agrego el contenido del CONST SEGMENT
        if(header.tamKS){
            for(i=0;i<header.tamKS;i++){
                fread(&leo,sizeof(char),1,arch);
                agregoalconstantsegment(MV,offsetks,leo);
                offsetks++;
            }
        }

    }
    }else if (header.c1=='V' && header.c2 =='M' && header.c3=='I' && header.c4=='2' && header.c5=='5'){
        // ENTRA CON UN ARCHIVO DE IMAGEN.
        headervmi.carV=header.c1;
        headervmi.carM=header.c2;
        headervmi.carI=header.c3;
        headervmi.car2=header.c4;
        headervmi.car5=header.c5;
        headervmi.mem_size=header.tamCS;
        //SI ENTRO ACA ES PORQUE TENGO Q BASARME EN LA IMAGEN
        inicializoMVen0(MV);
        MV->mem_size=headervmi.mem_size * 1024;
        for(i=0;i<CANTREG;i++){ //LEO LOS REGISTROS
            for(j=0;j<4;j++){
                fread(&leo,sizeof(char),1,arch);
                MV->R[i]<<=8;
                MV->R[i]|=leo;
            }
        }

        for(i=0;i<CANTMAXSEGMENTOS;i++){
            for(j=0;j<4;j++){
                fread(&leo,sizeof(char),1,arch);
                MV->TDS[i] <<= 8;
                MV->TDS[i] |= leo;
            }
        }
        for(i=0;i<MV->mem_size;i++){
            fread(&leo,sizeof(char),1,arch);
            MV->MEM[i]=leo;
        }
        //MAQUINA VIRTUAL SETEADA COMO LA IMAGEN.
    }
   fclose(arch);
}

  //MODIFICADO
int direccionamiento_logtofis(TMV *MV, int puntero, int bytes){
    int DirBase,Offset,DirFisica,TamSeg,LimiteSup;

    DirBase = ((MV->TDS[(puntero & 0XFFFF0000) >> 16] ) & 0XFFFF0000) >> 16;
    Offset = puntero & 0X0000FFFF;

    DirFisica = DirBase + Offset;

    TamSeg = ((MV->TDS[(puntero & 0XFFFF0000) >> 16] ) & 0XFFFF);
    LimiteSup = DirBase + TamSeg;
    //printf("%d %d %d \n",DirFisica,DirBase,LimiteSup);
    //printf("%d %d \n",DirBase,TamSeg);

    if (bytes!=0){ //Seteo bytes en cero cuando recorro el CS para leer las instrucciones, para la lectura y escritura en memoria bytes es la cantidad de bytes a escribir o leer
        MV->R[LAR] = puntero; // dir logica
        MV->R[MAR] = (bytes << 16) + DirFisica; // bytes a cargar, siempre es 4? // Dir fisica

    }
    // En leo o escribo memoria modifico MBR

    if (!( (DirBase <= DirFisica ) && (DirFisica+bytes <= LimiteSup  ) )){ // FALTA EL +4 EN DIR FISICA SI ES MEMORIA
        generaerror(ERRSEGMF);
        return -1;        // Aca nunca va a llegar si llama a generaerror, porque la ultima instruccion de la funcion es abort().
    }
    else
        return DirBase+Offset;
}

int posmaxCODESEGMENT(TMV *MV){
    int finCS,baseCS,tamCS;

    baseCS = direccionamiento_logtofis(MV,MV->R[CS],0);
    tamCS = (MV->TDS[MV->R[CS] >> 16]) & 0XFFFF;
    finCS = baseCS + tamCS;
    return finCS;
}

void LeoInstruccion(TMV* MV){
    int finCS;
    int CantOp;
    //TInstruc instruc;
    TFunc Funciones;
    int DirFisicaActual;

    declaroFunciones(Funciones);

    finCS=posmaxCODESEGMENT(MV);

    //printf("%d \n",MV->R[IP]);
    while(direccionamiento_logtofis(MV,MV->R[IP],0)<finCS){ //MIENTRAS HAYA INSTRUCCIONES PARA LEER (BYTE A BYTE).

        DirFisicaActual = direccionamiento_logtofis(MV,MV->R[IP],0);
        ComponentesInstruccion(MV,DirFisicaActual,&CantOp); //TIPO INSTRUCCION, identifico los tipos y cantidad de operadores y el codigo de operacion

        if ((MV->R[OPC] >= 0) && ((MV->R[OPC] <= 8) || ((MV->R[OPC]<=31) && (MV->R[OPC]>=11))) ){ // Si el codigo de operacion es valido
            if (CantOp != 0) //Guardo los operandos que actuan en un auxiliar, y tambien guardo el tamanio del operando
               SeteoValorOp(MV, DirFisicaActual); // Distingue entre uno o dos operandos a setear

           //Avanzo a la proxima instruccion. FIX: Mueve el puntero de IP antes de llamar a la funcion, asi funcionan los SALTOS.
            MV->R[IP]=MV->R[IP] + ( (MV->R[OP1] >> 24 ) &  0XFF ) + ( (MV->R[OP2] >> 24 ) & 0XFF) + 1 ;
            Funciones[MV->R[OPC]](MV);
        }else{
            generaerror(ERRINVINST);
        }

        if (MV->flagdebug && (MV->archivovmi != NULL)) {
            generarImagen(*MV);
            mododebug(MV);
        }

    }
}

void ComponentesInstruccion(TMV *MV,int DirFisica, int *CantOp){
  //A priori no se cual es el opA y opB, suponemos que son 2 operandos, mas abajo, verifico.
  unsigned char Instruccion = MV->MEM[DirFisica];

  MV->R[OP2] = ((Instruccion >> 6) & 0x3) << 24;
  MV->R[OP1] = ((Instruccion >> 4) & 0x3) << 24;
  MV->R[OPC] = Instruccion & 0x1F;

  *CantOp=2;

  //Si no pasa por ningun if significa que tiene dos operandos.

  if (MV->R[OP1] == 0){ //No existe op1 -> ???0
      if (MV->R[OP2] == 0){ //No existe op2
        MV->R[OP1]=0;
        MV->R[OP2]=0;
        *CantOp=0;
      }
      else{ //Existe solo un operando
          MV->R[OP1]=MV->R[OP2]; //Cuando hay un solo operando se llama opA y es en la posicion que antes tenia opB
          MV->R[OP2]=0;
          *CantOp=1;
      }
  }
}

void SeteoValorOp(TMV *MV,int DirFisicaActual){
    int Tam1 = (MV->R[OP1] >> 24) & 0X3;
    int Tam2 = (MV->R[OP2] >> 24) & 0X3;

    for (int i=0;i<Tam2;i++){
        MV->R[OP2]+=MV->MEM[++DirFisicaActual];
        if ((Tam2-i) > 1)
            MV->R[OP2] = MV->R[OP2] << 8;
    }

    for (int i=0;i<Tam1;i++){
        MV->R[OP1]+=MV->MEM[++DirFisicaActual];
        if ((Tam1-i) > 1)
            MV->R[OP1] = MV->R[OP1] << 8;
    }

    // Para recuperar los tamaños perdidos:

    MV->R[OP1] |= (Tam1 << 24);
    MV->R[OP2] |= (Tam2 << 24);



    // == 0 nada
    // == 1 registro 8 bits
    // == 2 inmediato 16 bits
    // == 3 memoria 24 bits

}

void DefinoRegistro(unsigned char *Sec , unsigned char *CodReg, int Op){  //Defino el sector del registro en el que operare y el tipo de registro
  *Sec = (Op >> 6) & 0x03;
  *CodReg = Op & 0x1F;

}// Devuelve Sector y Codigo de Registro.

void DefinoAuxRegistro(int *AuxR,TMV MV,unsigned char Sec,int CodReg){ //Apago las posiciones del registro de 32 bytes en el que asignare a otro registro/memoria
  signed char charlocal=0;
  signed short int shortlocal=0;

  if (Sec == 1){
        charlocal = MV.R[CodReg] & 0XFF;
        *AuxR=charlocal;
    }
      else
        if (Sec == 2){
         charlocal=(MV.R[CodReg]>>8 & 0XFF);
         *AuxR=charlocal;
        }
          else
            if (Sec == 3){
              shortlocal=MV.R[CodReg] & 0XFFFF;
              *AuxR=shortlocal;
            }
            else
                *AuxR = MV.R[CodReg];



}

int LeoEnMemoria(TMV *MV,int Op){ // Guarda el valor de los 4 bytes de memoria en un auxiliar
    int aux=0,PosMemoria,offset,CodReg,puntero;
    unsigned short int modif;
    int TamModif,corro;

    offset= Op & 0XFFFF;
    offset<<=16;
    offset>>=16;
    CodReg=(Op>>16)&0x1F;

    puntero=MV->R[CodReg]+offset;
    modif = (Op>>22) & 0x3;
    TamModif = (~modif)&0x3;
    TamModif+=1;

    PosMemoria = direccionamiento_logtofis(MV,puntero,4);

    for (int i=0;i<TamModif;i++){
        aux+=MV->MEM[PosMemoria];
        PosMemoria++;
        if (TamModif-i > 1)
            aux=aux << 8;
    }

    corro = (4-TamModif)*8;
    aux = aux << corro;
    aux = aux >> corro;

    MV->R[MBR] = aux;

    return aux;
}

void EscriboEnMemoria(TMV *MV,int Op, int Valor){ // Guarda el valor en 4 bytes de la memoria, se usa solo para el MOV
    int offset,CodReg,puntero,PosMemoria,TamModif;
    unsigned short int modif;


    offset= Op & 0XFFFF;
    CodReg=(Op>>16)&0x1F;
    modif = (Op>>22) & 0x3;
    TamModif = (~modif)&0x3;
    TamModif+=1;

    //printf("%d offset \t %d codreg",offset,CodReg);
    puntero=(*MV).R[CodReg]+offset;
    PosMemoria = direccionamiento_logtofis(MV,puntero,4);

    for (int i=0;i<TamModif;i++){
        MV->MEM[PosMemoria] = (Valor & 0XFF000000) >> 24;
        PosMemoria++;
        if (TamModif-i > 1)
            Valor=Valor << 8;
    }
    MV->R[MBR] = Valor;
}

void modificoCC(TMV *MV,int Resultado){
  MV->R[CC] = MV->R[CC] & 0x00000000;
    if (Resultado < 0)
        MV->R[CC] = 0x80000000;
    else
      if (Resultado == 0)
         MV->R[CC] = 0x40000000;
}

void guardoOpB(TMV *MV, int *auxOpB){
    unsigned char SecB,CodOpB;
    //OPB

    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){
        DefinoRegistro(&SecB,&CodOpB,MV->R[OP2]);
        DefinoAuxRegistro(auxOpB,*MV,SecB,CodOpB);
    }
    else
      if (((MV->R[OP2] >> 24 ) &  0XFF ) == 2){  //Inmediato
         *auxOpB = MV->R[OP2] & 0XFFFF;
         *auxOpB = *auxOpB << 16;
         *auxOpB = *auxOpB >> 16;
      }
      else
        if (((MV->R[OP2] >> 24 ) &  0XFF ) == 3)
            *auxOpB = LeoEnMemoria(MV,MV->R[OP2]);

}

void guardoOpA(TMV *MV, int *auxOpA){
    unsigned char SecA,CodOpA;
    //OPB

    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        DefinoAuxRegistro(auxOpA,*MV,SecA,CodOpA);
    }
    else
      if (((MV->R[OP1] >> 24 ) &  0XFF ) == 2){  //Inmediato
         *auxOpA = MV->R[OP1] & 0XFFFF;
         *auxOpA = *auxOpA << 16;
         *auxOpA = *auxOpA >> 16;
      }
      else
        if (((MV->R[OP1] >> 24 ) &  0XFF ) == 3)
            *auxOpA = LeoEnMemoria(MV,MV->R[OP1]);

}

void MOV(TMV * MV){
    int mover;
    //OPB
    guardoOpB(MV,&mover);
    //OPA


    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1) //4 byte
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + (mover & 0XFF);
        else
            if (SecA == 2){ //3 byte
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( (mover & 0XFF) << 8);
            }
            else
                if (SecA == 3) //3 y 4 byte
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) + (mover & 0XFFFF);
                else{ //Los 4 bytes
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0X0000000000000000) + mover;
                }
    }
    else{ //Es memoria ya que no se puede guardar nada en un inmediato
        EscriboEnMemoria(MV,MV->R[OP1],mover);
    }

}

void ADD(TMV * MV){
    int sumar;

    //OPB
    guardoOpB(MV,&sumar);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      unsigned char SecA,CodOpA;
      DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) + (sumar & 0XFF) ) & 0XFF );  // MODIFICAR
        else
            if (SecA == 2){
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) + ( (sumar & 0XFF) << 8) ) & 0x0000FF00);
            }
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) + ( ( (MV->R[CodOpA] & 0x0000FFFF) +  (sumar & 0XFFFF) ) & 0x0000FFFF );
                else
                    MV->R[CodOpA] += sumar;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]); //ResultadoSeg guarda el resultado del segmento que fue modificado, con el fin de modificar condition code
    }
     else{
        int AuxSuma;
        AuxSuma = LeoEnMemoria(MV,MV->R[OP1]) + sumar;
        modificoCC(MV,AuxSuma);
        EscriboEnMemoria(MV,MV->R[OP1],AuxSuma);
    }


}

void SUB(TMV * MV){
    int resta;

    //OPB
    guardoOpB(MV,&resta);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) - (resta & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) - ( (resta & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) -  (resta & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] -= resta;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
    }
     else{
        int AuxResta;
        AuxResta = LeoEnMemoria(MV,MV->R[OP1]) - resta;
        modificoCC(MV,AuxResta);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResta);
    }


}

void SWAP(TMV *MV){
    // Intercambia los valores de los dos operandos (ambos deben ser registros y/o celdas de memoria)
    // IDEA: SACO EL PRIMER OPERANDO A UN AUXILIAR, PONGO LO DEL SEGUNDO OPERANDO EN EL PRIMERO, Y PONGO LO DEL AUXILIAR EN EL SEGUNDO OPERANDO.a
    int auxA=0,auxB=0,regA,regB;
    unsigned char secA=0,secB=0,codregA,codregB;

    //Saco primer operando a un auxiliar.
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        DefinoRegistro(&secA,&codregA,MV->R[OP1]);
        DefinoAuxRegistro(&regA,*MV,secA,codregA);
        auxA=regA;
    }
    else{ // El operando A es de memoria.
        auxA=LeoEnMemoria(MV,MV->R[OP1]);
    }


    //Saco segundo operando a un auxiliar.
    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){
        DefinoRegistro(&secB,&codregB,MV->R[OP2]);
        DefinoAuxRegistro(&regB,*MV,secB,codregB);
        auxB=regB;
    }
    else{
        auxB=LeoEnMemoria(MV,MV->R[OP2]);
    }


    // Hago "el MOV OpA,AuxB"
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        if (secA==0){
            MV->R[codregA]=auxB;
        }
        else if (secA==3){ // AX
            MV->R[codregA]=MV->R[codregA] & 0xFFFF0000;
            MV->R[codregA]=MV->R[codregA] & (auxB & 0x0000FFFF);
        }
        else if(secA==2){ // AH
            MV->R[codregA]=MV->R[codregA]& 0xFFFF00FF;
            auxB=auxB & 0xFF;
            MV->R[codregA]=MV->R[codregA] & (auxB<<8);

        }
        else{ // secA==1 (AL)
            MV->R[codregA]=MV->R[codregA] & 0xFFFFFF00;
            MV->R[codregA]=MV->R[codregA] & (auxB & 0x000000FF);
        }
    }
    else{ //OPERANDO A ES MEMORIA
        EscriboEnMemoria(MV,MV->R[OP1],auxB);
    }


    //Hago "el MOV OpB,AuxA"
    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        if (secB==0){
            MV->R[codregB]=auxA;
        }
        else if (secB==3){
            MV->R[codregB]=MV->R[codregB]& 0xFFFF0000;
            MV->R[codregB]=MV->R[codregB] & (auxA & 0x0000FFFF);
        }
        else if(secB==2){
            MV->R[codregB]=MV->R[codregB]& 0xFFFF00FF;
            auxA=auxA & 0xFF;
            MV->R[codregB]=MV->R[codregB] & (auxA<<8);
        }
        else{ // secB==1 (AL)
            MV->R[codregB]=MV->R[codregB] & 0xFFFFFF00;
            MV->R[codregB]=MV->R[codregB] & (auxA & 0x000000FF);
        }
    }
    else{ //OPERANDO B ES MEMORIA
        EscriboEnMemoria(MV,MV->R[OP2],auxA);
    }
}

void MUL(TMV * MV){
    int mult;

    //OPB
    guardoOpB(MV,&mult);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) * (mult & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) * ( (mult & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) * (mult & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] *= mult;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxMult;
        AuxMult = LeoEnMemoria(MV,MV->R[OP1]) * mult;
        modificoCC(MV,AuxMult);
        EscriboEnMemoria(MV,MV->R[OP1],AuxMult);
    }
}

void DIV(TMV * MV){
    int divisor;


    //OPB
    guardoOpB(MV,&divisor);

    if (divisor == 0)
        generaerror(ERRDIV0);
    else{
    //OPA
    int Dividendo;
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
          unsigned char SecA,CodOpA;
          DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
          DefinoAuxRegistro(&Dividendo,*MV,SecA,CodOpA);
          if (SecA == 1) //4 byte
              MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) / (divisor & 0XFF) ) & 0XFF ); //Apago los bits que voy a modificar de OpA
          else                                                                                         //Y lo divido con lo que da la suma de los bits que voy a modificar de OpA con los bits que corresponden al OpB.
              if (SecA == 2){ //3 byte
                  MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) / ( (divisor & 0XFF) << 8) ) & 0x0000FF00);
              }
              else
                  if (SecA == 3) //3 y 4 byte
                      MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) /  (divisor & 0XFFFF) ) & 0x0000FFFF);
                  else //Los 4 bytes
                      MV->R[CodOpA] /= divisor;

        int ResultadoSeg;
        DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
        modificoCC(MV,ResultadoSeg);
        MV->R[AC] = ( Dividendo % divisor);
    }
    else{ //Es memoria ya que no se puede guardar nada en un inmediato
        int AuxDiv, dividendo = LeoEnMemoria(MV,MV->R[OP1]);
        AuxDiv = (int) (dividendo / divisor);
        if ((dividendo * divisor) < 0 && (dividendo % divisor != 0)) { // Si la division daba -0.5 la truncaba a 0, debe truncarla a -1.
            AuxDiv--; // Capaz que debo tenerlo en cuenta para registros tambien..
        }
        modificoCC(MV,AuxDiv);
        MV->R[AC] = dividendo % divisor;
        EscriboEnMemoria(MV,MV->R[OP1],AuxDiv);
    }
    }
}

void CMP(TMV * MV){
    int resta,resultado;

    //OPB
    guardoOpB(MV,&resta);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1){
            resultado = ( ( (MV->R[CodOpA] & 0x000000FF) - (resta & 0XFF) ) & 0XFF );
            resultado<<=24;
            resultado>>=24;
        }
        else
            if (SecA == 2){
                resultado = ( ( (MV->R[CodOpA] & 0x0000FF00) - ( (resta & 0XFF) << 8) ) & 0x0000FF00) >> 8;
                resultado<<=16;
                resultado>>=16;
            }
            else
                if (SecA == 3){
                    resultado = ( ( (MV->R[CodOpA] & 0x0000FFFF) -  (resta & 0XFFFF) ) & 0x0000FFFF) ;
                    resultado<<=16;
                    resultado>>=16;
                }
                else
                    resultado = MV->R[CodOpA] - resta;
    }
     else //Memoria
        resultado = LeoEnMemoria(MV,MV->R[OP1]) - resta;


    modificoCC(MV,resultado);
}

void SHL(TMV * MV){ // Se propaga solo con ceros
    int auxSHL;

    //OPB
    guardoOpB(MV,&auxSHL);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) << (auxSHL & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) << ( (auxSHL & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) << (auxSHL & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] <<= auxSHL;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResSHL;
        AuxResSHL = LeoEnMemoria(MV,MV->R[OP1]) << auxSHL;
        modificoCC(MV,AuxResSHL);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResSHL);
    }
}

void SHR(TMV * MV){ // SHR LÓGICO -  Debo propagar con ceros a la izq
    int auxSHR;

    //OPB
    guardoOpB(MV,&auxSHR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + (unsigned int) ( ( (MV->R[CodOpA] & 0x000000FF) >> (auxSHR & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + (unsigned int) ( ( (MV->R[CodOpA] & 0x0000FF00) >> ( (auxSHR & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  (unsigned int) ( ( (MV->R[CodOpA] & 0x0000FFFF) >> (auxSHR & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] = (unsigned int) MV->R[CodOpA] >> auxSHR;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResSHR;
        AuxResSHR = (unsigned int) LeoEnMemoria(MV,MV->R[OP1]) >> auxSHR;
        modificoCC(MV,AuxResSHR);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResSHR);
    }
}

void SAR(TMV * MV){ // Se propaga solo con ceros o unos
    int auxSAR;
    //OPB
    guardoOpB(MV,&auxSAR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) >> (auxSAR & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) >> ( (auxSAR & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) >> (auxSAR & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] >>= auxSAR;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResSAR;
        AuxResSAR = LeoEnMemoria(MV,MV->R[OP1]) >> auxSAR;
        modificoCC(MV,AuxResSAR);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResSAR);
    }
}

void AND(TMV * MV){
    int auxAND;

    //OPB
    guardoOpB(MV,&auxAND);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) & (auxAND & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) & ( (auxAND & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) & (auxAND & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] &= auxAND;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResAnd;
        AuxResAnd = LeoEnMemoria(MV,MV->R[OP1]) & auxAND;
        modificoCC(MV,AuxResAnd);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResAnd);
    }
}

void OR(TMV * MV){
    int auxOR;

    //OPB
    guardoOpB(MV,&auxOR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) | (auxOR & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) | ( (auxOR & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) | (auxOR & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] |= auxOR;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResOR;
        AuxResOR = LeoEnMemoria(MV,MV->R[OP1]) | auxOR;
        modificoCC(MV,AuxResOR);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResOR);
    }
}

void XOR(TMV * MV){
    int auxXOR;

    //OPB
    guardoOpB(MV,&auxXOR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( ( (MV->R[CodOpA] & 0x000000FF) ^ (auxXOR & 0XFF) ) & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( ( (MV->R[CodOpA] & 0x0000FF00) ^ ( (auxXOR & 0XFF) << 8) ) & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( ( (MV->R[CodOpA] & 0x0000FFFF) ^ (auxXOR & 0XFFFF) ) & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] ^= auxXOR;

      int ResultadoSeg;
      DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,ResultadoSeg);
    }
     else{
        int AuxResXOR;
        AuxResXOR = LeoEnMemoria(MV,MV->R[OP1]) ^ auxXOR;
        modificoCC(MV,AuxResXOR);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResXOR);
    }
}

void LDL(TMV * MV){
    int auxLDL;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxLDL);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) + (auxLDL & 0X0000FFFF) ;
    }
     else{
        int AuxResLDL;
        AuxResLDL = (LeoEnMemoria(MV,MV->R[OP1]) & 0XFFFF0000 )+ (auxLDL & 0X0000FFFF);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResLDL);
     }
}

void LDH(TMV * MV){
    int auxLDH;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxLDH);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] = (MV->R[CodOpA] & 0X0000FFFF) + ((auxLDH & 0X0000FFFF) << 16);
    }
     else{
        int AuxResLDH;
        AuxResLDH = (LeoEnMemoria(MV,MV->R[OP1]) & 0X0000FFFF )+ ((auxLDH & 0X0000FFFF) << 16);
        EscriboEnMemoria(MV,MV->R[OP1],AuxResLDH);
     }
}

void RND(TMV * MV){
    int tope,random;

    //OPB
    guardoOpB(MV,&tope);
    random = rand() % (tope + 1);

    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        unsigned char SecA,CodOpA;
        DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
        if (SecA == 1)
            MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + ( random  & 0XFF );
        else
            if (SecA == 2)
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + (( (random & 0XFF) << 8)  & 0x0000FF00);
            else
                if (SecA == 3)
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) +  ( random & 0x0000FFFF) ;
                else
                    MV->R[CodOpA] = random;
    }
     else //Es memoria ya que no se puede guardar nada en un inmediato
        EscriboEnMemoria(MV,MV->R[OP1],random);

}
//---------------------------------------------------------------DEBUG-------------------------------------------------------------------
void muestraheader(theader h){
    printf("%c %c %c %c %c",h.c1,h.c2,h.c3,h.c4,h.c5);
    printf("\n version:%d",h.version);
    printf("\n tamCS: %d",h.tamCS);
    printf("\n tamDS: %d",h.tamDS);
    printf("\n tamES: %d",h.tamES);
    printf("\n tamSS: %d",h.tamSS);
    printf("\n tamKS: %d",h.tamKS);
    printf("\n offsetentrypoint: %d",h.entrypointoffset);
}

void muestraMVfijos(TMV MV){
    printf("\n-------FLAG MV \n");;
    //if(MV.archivovmi)
        //printf("\n archivo vmi: %s",MV.archivovmi);
    printf("\n argc: %d",MV.argc);
    printf("\n -d: %d",MV.disassembler);
    printf("\n flag debug: %d",MV.flagdebug);
    printf("\n MEM SIZE: %d",MV.mem_size);
    printf("\n posicion a la que apunta el puntero argv: %d",MV.punteroargv);
    printf("\n SIZE PARAMSEGMENT :%d \n",MV.size_paramsegment);
}

void muestramemoria(unsigned char memoria[]){
    int pos_i,pos_f;
    printf("Ingresar de que posicion a que posicion mostrar\n Pos_inicial: ");
    scanf("%d",&pos_i);
    printf("\nPos_final: ");
    scanf("%d",&pos_f);

    printf("\n------- MEMORIA -------\n");
    while(pos_i<=pos_f){
        printf("[%08X] = %02X \n",pos_i,memoria[pos_i]);
        pos_i++;
    }
}

void muestraDatasegment(TMV *MV,unsigned char memoria[]){
    int pos_i,ultposcs;
    int pos_f;

    ultposcs=posmaxCODESEGMENT(MV);
    pos_i=ultposcs;
    pos_f=pos_i+20;

    while(pos_i<=pos_f){
        printf("[%08X] = %02X \n",pos_i-ultposcs,memoria[pos_i]);
        pos_i++;
    }
}

void muestraregistros(int reg[]){
    int i;
    char VecRegistros[CANTREG][4];

    inicializoVecRegistros(VecRegistros);
    printf("\n------- REGISTROS -------\n");
    for(i=0;i<CANTREG;i++){
        printf("%s : %08X \n",VecRegistros[i],reg[i]);
    }
}

void muestratds(int tds[]){
    int i;
    printf("\n------- TDS -------\n");

    for(i=0;i<TDDSSIZE;i++){
        printf("pos:%d = %08X \n",i,tds[i]);
    }
}

void muestravaloresmv(TMV *mv){
    //muestratds(mv.TDS);
    muestraregistros(mv->R);
    muestramemoria(mv->MEM);
    muestraDatasegment(mv,mv->MEM);
}

char obtienetipooperacion(unsigned char operacion){
  if ((operacion&0x10)==0x10)
    return 2; //2 operandos
  else if ((operacion&0xF0)==0)
      return 0; //0 operandos
    else if ((operacion&0x30)==0)
        return 1; //1 operando
    else
        return -1; //Error, no existe la operacion.
}

char sobrepasaCS(TMV *MV,int asignable){

    if(asignable>=posmaxCODESEGMENT(MV))
        return 1;
    else
        return 0;
}

int devuelveN(TMV *MV){
    int i=0;
    i=((*MV).R[CC]>>31) & 1;
    return i;
}


int devuelveZ(TMV *MV){
    int i=0;
    i=((*MV).R[CC]>>30) & 1;
    return i;
}

int leer_binario_c2_32(void) {
    /**
 * Lee por teclado de 1 a 32 d gitos binarios,
 * los interpreta como un int en complemento a 2 de 32 bits
 * (con ceros impl citos a la izquierda) y devuelve el valor.
 */
    char buffer[BUF_SIZE];
    size_t len;
    unsigned int uvalue;
    int          svalue;

    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            fprintf(stderr, "Error al leer la entrada.\n");
            exit(EXIT_FAILURE);
        }

        // Quitar '\n' y medir longitud
        buffer[strcspn(buffer, "\n")] = '\0';
        len = strlen(buffer);

        // Validar longitud 1 32
        if (len == 0 || len > BITS_32) {
            printf("Entrada inv lida: ingresa entre 1 y %d d gitos binarios.\n\n", BITS_32);
            continue;
        }

        // Validar que todos sean '0' o '1'
        int valido = 1;
        for (size_t i = 0; i < len; i++) {
            if (buffer[i] != '0' && buffer[i] != '1') {
                valido = 0;
                break;
            }
        }
        if (!valido) {
            printf("Formato inv lido: solo d gitos '0' o '1'.\n\n");
            continue;
        }

        // Convertir a unsigned int (base 2)
        uvalue = (unsigned int)strtoul(buffer, NULL, 2);

        // Ajuste complemento a 2 (32 bits)
        if (uvalue & (1U << (BITS_32 - 1))) {
            svalue = (int)(uvalue - (1ULL << BITS_32));
        } else {
            svalue = (int)uvalue;
        }

        return svalue;
    }
}


char *int_to_c2bin(int numero) {
    /**
    * Convierte un `int` (asumiendo 32 bits) a su representaci n
    * en complemento a 2 y devuelve un puntero a una cadena reci n
    * alocada que contiene dicha representaci n sin ceros a la izquierda.
    *
    * Si el n mero es 0, la cadena ser  "0".
    * El llamador debe liberar el buffer con free().
    */

    char full[BITS_32 + 1];
    unsigned int u = (unsigned int) numero;

    // Generar la cadena completa de 32 bits
    for (int i = BITS_32 - 1; i >= 0; --i) {
        full[i] = (u & 1) ? '1' : '0';
        u >>= 1;
    }
    full[BITS_32] = '\0';

    // Encontrar el primer '1', o dejar el  ltimo '0' si es todo ceros
    char *p = full;
    while (*p == '0' && *(p + 1) != '\0') {
        ++p;
    }

    // Copiar a un buffer de tama o justo
    size_t len = strlen(p);
    char *trimmed = malloc(len + 1);
    if (!trimmed) return NULL;
    memcpy(trimmed, p, len + 1);
    return trimmed;
}

void setvaloresSYS(TMV *MV,char *mod, char *cantceldas, char *size, int *pos_i, int *pos_max){
    char modo,celdas,sizelocal;

    modo= MV->R[EAX]& 0xFF;
    *mod = modo;

    celdas = MV->R[ECX]& 0xFF;
    *cantceldas=celdas;

    sizelocal=(MV->R[ECX]>>16)& 0xFF;
    *size=sizelocal;

    *pos_i=direccionamiento_logtofis(MV,MV->R[EDX],0);
    *pos_max=direccionamiento_logtofis(MV, (MV->R[EDX]) + celdas * sizelocal,0); // Para verificar fallo de segmento

}

// -------------------------------------- FUNCIONES CON 1 OPERANDO
void SYS (TMV *MV){
/*  Ejecuta la llamada al sistema indicada por el valor del operando.
    SYS 1 (READ): permite almacenar los datos leidos desde el teclado a partir de la posicion de memoria apuntada por EDX, guardandolo en CL celdas de tama o CH.
    El modo de lectura depende de la configuracion almacenada en AL.

    SYS 2 (WRITE): muestra en pantalla los valores contenidos a partir de la posicion de memoria apuntada por EDX, recuperando CL celdas de tama o CH.
    El modo de escritura depende de la configuracion almacenada en AL.

*/
    int i,j,operando,pos_inicial_memoria,numero,pos_max_acceso,base,punteroedx;
    short int cx,nbytes;
    char modo,celdas,size,imprimible;
    char *bin;
    char *auxstr;
    unsigned char Sec,Codreg;

    int TamA = (MV->R[OP1] >> 24 ) &  0XFF ;

    if(TamA==1){
        DefinoRegistro(&Sec,&Codreg,MV->R[OP1]);
        DefinoAuxRegistro(&operando,*MV,Sec,Codreg);
    }
    else if (TamA==2){
        operando = MV->R[OP1] & 0XFFFF;
        operando = operando << 16;
        operando = operando >> 16;
    }
    else
        operando=LeoEnMemoria(MV,MV->R[OP1]);

    if(operando==1){    //READ
        setvaloresSYS(MV,&modo,&celdas,&size,&pos_inicial_memoria,&pos_max_acceso);

        for(i=0;i<celdas;i++){
            printf("[%04X] ",pos_inicial_memoria);
            if(modo==0x10){
                //lee binario como string Y LO PASA A INT.
                numero=leer_binario_c2_32();
            }
            else if (modo==0x08){
                scanf("%X",&numero);
            }
            else if (modo==0x04){
                scanf("%o",&numero);
            }
            else if (modo==0x02){
                scanf("\n %c",&numero);
            }
            else if (modo==0x01){
                scanf("%d",&numero);
            }

            if(size==1){
                (*MV).MEM[pos_inicial_memoria++]=numero;
            }
            else if (size==2){
                (*MV).MEM[pos_inicial_memoria++]=numero >> 8;
                (*MV).MEM[pos_inicial_memoria++]=numero & 0xFF;
            }
            else if (size==3){
                (*MV).MEM[pos_inicial_memoria++]=numero >> 16;
                (*MV).MEM[pos_inicial_memoria++]=(numero << 8) >> 16;
                (*MV).MEM[pos_inicial_memoria++]=numero & 0xFF;
            }
            else if (size==4){
                (*MV).MEM[pos_inicial_memoria++]=numero >> 24;
                (*MV).MEM[pos_inicial_memoria++]=(numero << 8) >> 24;
                (*MV).MEM[pos_inicial_memoria++]=(numero << 16)>> 24;
                (*MV).MEM[pos_inicial_memoria++]=numero & 0xFF;
            }
        }
    }
    else if (operando==2){ //WRITE.
        setvaloresSYS(MV,&modo,&celdas,&size,&pos_inicial_memoria,&pos_max_acceso);
        for (i=0;i<celdas;i++){
            printf("[%04X] ",pos_inicial_memoria);

            if(size==1){
                numero=(*MV).MEM[pos_inicial_memoria++];
            }
            else if(size==2){
                numero=(*MV).MEM[pos_inicial_memoria++];
                numero=numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
            }
            else if(size==3){
                numero=(*MV).MEM[pos_inicial_memoria++];
                numero=numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
                numero=numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
            }
            else if (size==4){
                numero=(*MV).MEM[pos_inicial_memoria++];
                numero=numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
                numero=numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
                numero = numero<<8;
                numero |= (*MV).MEM[pos_inicial_memoria++];
            }

            if(modo&0x10){
                //Funcion que toma el numero (entero de 32 bits) y lo transforma en un string con formato 0b numero
                bin = int_to_c2bin(numero);
                if(!bin){
                    printf("ERROR DE MEMORIA EN SYS WRITE BINARIO\n");
                    exit(0);
                }
                printf("0b%s ",bin);
                free(bin);
            }
            if(modo & 0x08)
                printf("0x%X ",numero);
            if(modo & 0x04)
                printf("0o%o ",numero);
            if(modo & 0x02){
                for (j=size-1;j>-1;j--){
                    imprimible=numero>>(4*(2*j));
                    if(imprimible<32 || imprimible>126)
                        printf(".");
                    else
                        printf("%c",imprimible);
                }
                printf(" ");
            }
            if(modo & 0x01)
                printf("%d ",numero);
            printf("\n");
        }
    }else if (operando == 3){
        /*
        almacena en un rango de celdas de memoria los datos leídos desde el teclado.
        Almacena lo que se lee en la posición de memoria apuntada por EDX. En CX (16 bits) se especifica la
        cantidad máxima de caracteres a leer. Si CX tiene -1 no se limita la cantidad de caracteres a leer.
        */

        punteroedx=MV->R[EDX];
        cx=MV->R[ECX]&0x0000FFFF;
        if(cx==0) //Validacion por si el programador assembler es limitado ;)
            generaerror(0xF);
        nbytes=(cx==-1)? 100 : cx;
        auxstr=malloc(nbytes*sizeof(char)+1);
        if(auxstr){
            fgets(auxstr,nbytes*sizeof(char)+1,stdin);
            auxstr[strcspn(auxstr, "\n")] = '\0';

            i=0;
            while(i<nbytes && auxstr[i]!='\0'){
                MV->MEM[direccionamiento_logtofis(MV,punteroedx,0)]=auxstr[i];
                punteroedx++;
                i++;
            }
            //Agrego el \0.
            MV->MEM[direccionamiento_logtofis(MV,punteroedx,0)]='\0';
            free(auxstr);
        }
        else
            generaerror(0xF);
    }
    else if(operando == 4){
        /*
        imprime por pantalla un rango de celdas donde se encuentra un string. Inicia en la
        posición de memoria apuntada por EDX, e imprime hasta encontrar un '\0' (0x00).
        */

        base=0;
        imprimible=MV->MEM[direccionamiento_logtofis(MV,MV->R[EDX]+base,0)];
        while(imprimible!='\0'){
                //printf("%c %s",imprimible,"F");
                putchar(imprimible);
                base+=1;
                imprimible=MV->MEM[direccionamiento_logtofis(MV,MV->R[EDX]+base,0)];
                //printf("\n [%04X]",direccionamiento_logtofis(*MV,MV->R[EDX]+base));

        }
    }
    else if(operando==7){
        clearscreen();
    }
    else if(operando==0xF){
        if(MV->archivovmi != NULL){
            MV->flagdebug=1;
        }
    }
    else{
        generaerror(ERRINVINST);
    } //ESTO NO SE SI SE HACE PERO BUENO.

}

void JMP (TMV *MV){
    //Efectua un salto incondicional a la celda del segmento de codigo indicada en el operando.
    int asignable,auxReg;
    unsigned char auxSecReg,auxCodReg;


    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ // Operando de registro
        DefinoRegistro(&auxSecReg,&auxCodReg,MV->R[OP1]);
        DefinoAuxRegistro(&auxReg,*MV,auxSecReg,auxCodReg);
        asignable=auxReg;
    }
    else if (((MV->R[OP1] >> 24 ) &  0XFF ) == 2){ // Operando inmediato
        asignable= MV->R[OP1] & 0XFFFF;
    }else if (((MV->R[OP1] >> 24 ) &  0XFF ) == 3){ // Operando de memoria
        asignable=LeoEnMemoria(MV,MV->R[OP1]);
    }

    // Antes de asignarle a Ip el asignable tendria que checkear que no salga del CS.
    if (sobrepasaCS(MV,asignable)==1)
        generaerror(2);

    MV->R[IP]=asignable;

}

void JZ (TMV *MV){
    //int asignable,auxReg;

    if(devuelveZ(MV)==1){
        JMP(MV);
    }
}

void JP (TMV *MV){
    //int asignable,auxReg;
    //unsigned char auxSecReg,auxCodReg;

    if((devuelveN(MV)==0) && (devuelveZ(MV)==0)){
       JMP(MV);
    }
}

void JN (TMV *MV){
    //int asignable,auxReg;
    //unsigned char auxSecReg,auxCodReg;

    if (devuelveN(MV)==1){
      JMP(MV);
    }
}

void JNZ (TMV *MV){
    //int asignable,auxReg;
    //unsigned char auxSecReg,auxCodReg;

    if(devuelveZ(MV)==0){
       JMP(MV);
    }
}

void JNP (TMV *MV){
    //int asignable,auxReg;
    //unsigned char auxSecReg,auxCodReg;

    if(devuelveN(MV)==1 || devuelveZ(MV)==1){
      JMP(MV);
    }
}

void JNN (TMV *MV){
    //int asignable,auxReg;
    //unsigned char auxSecReg,auxCodReg;

    if(devuelveN(MV)==0){
      JMP(MV);
    }
}

void NOT (TMV *MV){
// Efectua la negacion bit a bit del operando y afectan al registro CC.
// El resultado se almacena en el primer operando.
    int resultado,aux;
     unsigned char auxSec,auxCodReg;

    if(((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Operando de registro
        DefinoRegistro(&auxSec,&auxCodReg,MV->R[OP1]);
        DefinoAuxRegistro(&aux,*MV,auxSec,auxCodReg);
        resultado=~aux;
        if(auxSec==0)
            (*MV).R[auxCodReg]=resultado;
        else if (auxSec==1){
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]>>8;
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]<<8;
            resultado=resultado & 0xFF;
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]&resultado;
            //propagacion de signo para mandarlo a modificaCC
            resultado=resultado<<24;
            resultado=resultado>>24;
        }
        else if(auxSec==3){
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]>>16;
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]<<16;
            resultado=resultado&0xFFFF;
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]&resultado;
            //Propagacion de signo para mandarlo a modificaCC.
            resultado=resultado<<16;
            resultado=resultado>>16;
        }
        else if(auxSec==2){
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg]&0xFFFF00FF;
            resultado=resultado & 0x000000FF;
            (*MV).R[auxCodReg]=(*MV).R[auxCodReg] & (resultado<<8);
            //Propagacion de signo para mandarlo a modificoCC
            resultado=resultado<<24; // Esto tiene en cuenta que DefinoAuxRegistro no me lo devuelve colocado en AH sino en AL lo que habia en AH.
            resultado=resultado>>24;
        }
    }
    else if (((MV->R[OP1] >> 24 ) &  0XFF ) == 3){ //Operando de memoria
        aux=LeoEnMemoria(MV,MV->R[OP1]);
        resultado=~aux;
        EscriboEnMemoria(MV,MV->R[OP1],resultado);
    }

    modificoCC(MV,resultado);
}

void PUSH(TMV *MV){
    int PosSP,i,guardo=0;


   guardoOpA(MV,&guardo);
    if ((MV->R[SP] -4 ) < MV->R[SS])
        generaerror(ERRSTOVF);
    else{
       MV->R[SP]-=4;
       PosSP=direccionamiento_logtofis(MV,MV->R[SP],0);
        for (i=3;i>=0;i--){
            MV->MEM[PosSP++] = (guardo >> (8*i)) & 0xFF;
        }
    }
}

void POP(TMV *MV) {
    int guardo=0,TamPila,PosSP;
    unsigned char SecA,CodOpA;
    //Primero levanta el dato y luego corre SP (+4)

    //LEVANTO EL DATO

    TamPila = (MV->TDS[(MV->R[SS] >> 16) & 0XFFFF]) & 0XFFFF ; // TamSS

   if ((MV->R[SP] & 0XFFFF) == TamPila)
        generaerror(ERRSTUNF);
    else{
        PosSP = direccionamiento_logtofis(MV,MV->R[SP],0);
        for (int i=0;i<4;i++){ //Recorro los 4 bytes ; arranco desde el más significativo (tope de la pila)
            guardo += MV->MEM[PosSP];
            PosSP++;
            if (4 - i > 1)
                guardo = guardo << 8;
            MV->R[SP]+=1;
        }

        //GUARDO EL DATO

        if(((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
            DefinoRegistro(&SecA,&CodOpA,MV->R[OP1]);
            if (SecA == 1) //4 byte
                MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFFFF00) + (guardo & 0XFF);
            else
                    if (SecA == 2){ //3 byte
                    MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF00FF) + ( (guardo & 0XFF) << 8);
                     }
                      else
                    if (SecA == 3) //3 y 4 byte
                        MV->R[CodOpA] = (MV->R[CodOpA] & 0XFFFF0000) + (guardo & 0XFFFF);
                    else{ //Los 4 bytes
                        MV->R[CodOpA] = (MV->R[CodOpA] & 0X0000000000000000) + guardo;
                    }
        }
        else //Memoria
            EscriboEnMemoria(MV,MV->R[OP1],guardo);
    }
}

void CALL(TMV * MV){
  int guardo,PosSP,PosSS,asignable;

    MV->R[SP]-=4;
    PosSP=direccionamiento_logtofis(MV,MV->R[SP],0);
    PosSS=direccionamiento_logtofis(MV,MV->R[SS],0);
    guardo=MV->R[IP];
  if (PosSP < PosSS)
        generaerror(ERRSTOVF);
    else{
        /*for (int i=0;i<4;i++){ //Recorro los 4 bytes
            MV->MEM[PosSP] = (guardo & 0XFF000000) >> 24;
            PosSP++;
            if (4-i > 1)
                guardo = guardo << 8;
        }*/
        for (int i=3;i>=0;i--){
            MV->MEM[PosSP++] = (guardo >> (8*i)) & 0xFF;
        }

//SALTO A LA MEMORIA QUE SE INDICA EN OpA
        guardoOpA(MV,&asignable);

// Antes de asignarle a Ip el asignable tendria que checkear que no salga del CS.
        if (sobrepasaCS(MV,asignable)==1)
            generaerror(ERRSEGMF);
        else
            MV->R[IP] = (MV->R[IP] & 0XFFFF0000) + asignable;
    }
}

// -------------------------------------- FUNCIONES SIN OPERANDO
void STOP(TMV *MV){
    exit (0);
}


void RET(TMV * MV){
  int guardo=0,TamPila,PosSP;
   //Primero levanta el dato y luego corre SP (+4)

 //LEVANTO EL DATO

    TamPila = (MV->TDS[(MV->R[SS] >> 16) & 0XFFFF]) & 0XFFFF ; // TamSS
    if ((MV->R[SP] & 0XFFFF) == TamPila)
        generaerror(ERRSTUNF);
    else{
        PosSP = direccionamiento_logtofis(MV,MV->R[SP],0);
        for (int i=0;i<4;i++){ //Recorro los 4 bytes ; arranco desde el más significativo (tope de la pila)
            guardo |= MV->MEM[PosSP];
            PosSP++;
            if (4 - i > 1)
                guardo = guardo << 8;
            MV->R[SP]+=1;
        }
        //GUARDO EL DATO EN IP
        MV->R[IP] = guardo;
    }
}

void LeoInstruccionesDissasembler(TMV *MV,char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4]) {

    int CantOp,InicioKS,InicioCS,TamKS;
    unsigned short int PosInicialCS,PosMemoria,PosFinal;

    // CONSTANTES
    if(MV->version==2){
        if (MV->R[KS] != -1){
            InicioKS = (MV->TDS[((MV->R[KS] >> 16) & 0XFFFF)] >> 16) & 0XFFFF;
            TamKS = MV->TDS[((MV->R[KS] >> 16) & 0XFFFF)] & 0XFFFF;
            PosMemoria = InicioKS;
            while (PosMemoria < TamKS)
                EscribeCadenaDissasembler(*MV,&PosMemoria);
        }
    }
    InicioCS = (MV->TDS[((MV->R[CS] >> 16) & 0XFFFF)] >> 16) & 0XFFFF;
    PosMemoria = InicioCS;

    PosFinal = posmaxCODESEGMENT(MV);

    while (PosMemoria < PosFinal) {

        PosInicialCS=PosMemoria;
        ComponentesInstruccion(MV,PosMemoria,&CantOp);
        SeteoValorOp(MV,PosMemoria);

        PosMemoria +=  ( (MV->R[OP1] >> 24 ) &  0XFF ) + ( (MV->R[OP2] >> 24 ) & 0XFF) + 1; // Posicion de la Siguiente instruccion
        EscriboDissasembler(*MV,VecFunciones,VecRegistros,MV->R[OPC],PosInicialCS,PosMemoria);
    }
}

void EscribeCadenaDissasembler(TMV MV,unsigned short int *PosMemoria){
   int i=0;
   printf("[%04X] ",*PosMemoria);
   char Cadena[30]="";

   while ( MV.MEM[*PosMemoria] != 0 ){ //Mientras no se termine el string (\0)
       if (i == 6)
          printf("..");
       else
          if (i < 6)
             printf("%02X ",MV.MEM[*PosMemoria]);
       if ( (MV.MEM[*PosMemoria] > 31) && (MV.MEM[*PosMemoria] != 127) ) //Caracter no imprimible, segun ASCII 127 es supr
          Cadena[i]=MV.MEM[*PosMemoria];
       else
            Cadena[i]='.';
       *PosMemoria = *PosMemoria + 1;
       i++;
   }
   *PosMemoria = *PosMemoria + 1; //Si encuentro el cero tambien me tengo que adelantar un paso, si no, ciclo infinito

   if (i < 6)
      printf("00");

   if (i>=5) //Para tabular
      i=6;

    for (int j=8-i;j>1;j=j-2)
        printf("\t");
    printf("| ");

    printf("''");
    printf("%s",Cadena);
    printf("'' \n");

}

void EscriboDissasembler(TMV MV, char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4], unsigned char CodOp,unsigned short int PosInicial,unsigned short int PosMemoria){

    short int Offset;
    unsigned char CodReg,SecA,SecB,Modif;
    int i;
    char AuxSeg[4];
    int TamA = (MV.R[OP1] >> 24 ) &  0XFF ;
    int TamB = (MV.R[OP2] >> 24 ) &  0XFF ;

    if (PosInicial == ( ( MV.TDS[(MV.R[IP] & 0XFFFF0000) >> 16] ) >> 16 ) +  (MV.R[IP] & 0XFFFF)) //Entry point
        printf(">");
    printf("[%04X] ",PosInicial); //Muestro posicion de la memoria indicada, hexadecimal de 4 partes , 4 nibbles

    //Muestro los 8 bits del tipo de instruccion, luego los valores de los operandos

    for (i=PosInicial;i<PosMemoria;i++){
        printf("%02X ",MV.MEM[i]); //Lo guardo en hexadecimal en dos partes. Primero se lee OpBOpA y luego COdOp y luego los valores de los operandos
    }

    //Tabulaciones

    if (TamA+TamB > 4)
        printf("\t| ");
    else
        if (TamA+TamB > 1)
          printf("\t\t| ");
    else
        printf("\t\t\t| ");

    printf("%s \t",VecFunciones[CodOp]); //Muestro operacion


    //Muestro ASM

    if (TamA != 0) {
        if (TamA == 3) { //Memoria
            Offset= MV.R[OP1] & 0XFFFF;
            CodReg=(MV.R[OP1] >> 16) & 0x1F;
            Modif = (MV.R[OP1] >> 22) & 0x3;

            if (Modif == 0)
                printf("l");
            else
                if (Modif == 2)
                   printf("w");
                else
                   if (Modif == 3)
                        printf("b");

            if (CodReg !=0)
                if (CodReg >= 10 && CodReg<=15)
                  printf("[%c%s%c+%d]",'E',VecRegistros[CodReg],'X',Offset);
                else
                    printf("[%s+%d]",VecRegistros[CodReg],Offset);
            else
                printf("[%d]",Offset);
        }
        else
          if (TamA == 2) { //inmediato
             printf("%hd", MV.R[OP1] & 0XFFFF);
        }
        else { //Registro
            DefinoRegistro(&SecA,&CodReg,MV.R[OP1]);
            if (CodReg >= 10 && CodReg<=15){
                strcpy(AuxSeg,VecRegistros[CodReg]);
                GuardoSector(AuxSeg,SecA);
                if (SecA == 0)
                    printf("%c%s",'E',AuxSeg);
                else
                  printf("%s",AuxSeg);
            }
            else
               printf("%s",VecRegistros[CodReg]);
        }

        if (TamB!=0) {
            printf(", ");
            if (TamB == 3) {  //Memoria
                Offset= MV.R[OP2] & 0XFFFF;
                CodReg=(MV.R[OP2] >> 16) & 0x1F;
                Modif = (MV.R[OP2] >> 22) & 0x3;

                if (Modif == 0)
                    printf("l");
                else
                    if (Modif == 2)
                        printf("w");
                    else
                        if (Modif == 3)
                            printf("b");

                if (CodReg >= 10 && CodReg<=15)
                    printf("[%c%s%c+%d]",'E',VecRegistros[CodReg],'X',Offset);
                else
                    printf("[%s+%d]",VecRegistros[CodReg],Offset);
            }
            else
                if (TamB == 2) {  //Inmediato
                  printf("%hd", MV.R[OP2] & 0XFFFF);
               }
                else { //Registro
                  DefinoRegistro(&SecB,&CodReg,MV.R[OP2]);
                  if (CodReg >= 10 && CodReg<=15){
                     strcpy(AuxSeg,VecRegistros[CodReg]);
                     GuardoSector(AuxSeg,SecB);
                     if (SecB == 0)
                        printf("%c%s",'E',AuxSeg);
                     else
                        printf("%s",AuxSeg);
                 }
                 else
                    printf("%s",VecRegistros[CodReg]);
                }
        }
    }
  printf("\n");
}

void GuardoSector(char Segmento[4],unsigned char Sec){
  if (Sec == 1)
        strcat(Segmento,"L");
  else
    if (Sec == 2)
        strcat(Segmento,"H");
    else
        strcat(Segmento,"X");
}

void clearscreen() {
    #ifdef _WIN32
        system("cls");
    #elif defined(__linux__) || defined(__APPLE__)
        system("clear");
    #else
        // No se reconoce el sistema operativo
        printf("No se puede limpiar la pantalla en este sistema.\n");
    #endif
}
