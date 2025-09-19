#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MV.h"

void inicializoTDS(TMV* MV,short int TamCS){
  MV->TDS[0]=TamCS;
  MV->TDS[1]=TamCS << 16;
  MV->TDS[1]+=(TMEM-TamCS);
}

void inicializoRegistros(TMV *MV){
  MV->R[CS]=0X00000000;
  MV->R[DS]=0X00010000;  //DS
  MV->R[IP]=MV->R[CS]; //IP
}

void inicializoErrores(TMV *MV){
  MV->Errores[0]=0;
  MV->Errores[1]=0;
  MV->Errores[2]=0;
}

void generaerror(int tipo){
    if(tipo==0)
        printf("ERROR DIVISION POR 0");
    if(tipo==1)
        printf("ERROR INSTRUCCION INVALIDA");
    if(tipo==2)
        printf("ERROR FALLO DE SEGMENTO");
    if(tipo==3)
        printf("MEMORIA INSUFICIENTE");
    if(tipo==4)
        printf("STACK OVERFLOW");
    if(tipo==5)
        printf("STACK UNDERFLOW");
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

    //0 Operandos
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
    strcpy(VecRegistros[7], "");
    strcpy(VecRegistros[8], "");
    strcpy(VecRegistros[9], "");
    strcpy(VecRegistros[EAX], "AX");    // Parte 2 es sin la X
    strcpy(VecRegistros[EBX], "BX");
    strcpy(VecRegistros[ECX], "CX");
    strcpy(VecRegistros[EDX], "DX");
    strcpy(VecRegistros[EEX], "EX");
    strcpy(VecRegistros[EFX], "FX");
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
    strcpy(VecRegistros[28], "");
    strcpy(VecRegistros[29], "");
    strcpy(VecRegistros[30], "");
    strcpy(VecRegistros[31], "");
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

//0 OPERANDOS
  Funciones[15]=STOP;
}


void LeoArch(char nomarch[],TMV *MV){
  FILE *arch;
  unsigned char leo;
  theader header;
  int i=0;
  //DEBO PREPARAR ARCHIVO PARA LECTURA
  arch = fopen(nomarch,"rb");
  fread(&header.c1,sizeof(char),1,arch);
  fread(&header.c2,sizeof(char),1,arch);
  fread(&header.c3,sizeof(char),1,arch);
  fread(&header.c4,sizeof(char),1,arch);
  fread(&header.c5,sizeof(char),1,arch);
  fread(&header.version,sizeof(char),1,arch);

  fread(&leo,sizeof(char),1,arch);
  header.tam=leo;
  header.tam=header.tam<<8;
  fread(&leo,sizeof(char),1,arch);
  header.tam+=leo;

  if(header.c1=='V' && header.c2 =='M' && header.c3=='X' && header.c4=='2' && header.c5=='5'){
    if (header.version == 1){
        inicializoTDS(MV,header.tam);
        inicializoRegistros(MV);
        inicializoErrores(MV);
        //CARGAR EL CODIGO EN LA MEMORIA DE LA MV
        while(!feof(arch)){
            fread(&(MV->MEM[i]),1,1,arch);
            i++;
        }
    }
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
        generaerror(2);
        return -1;        // Aca nunca va a llegar si llama a generaerror, porque la ultima instruccion de la funcion es abort().
    }
    else
        return DirBase+Offset;
}

int posmaxCODESEGMENT(TMV *MV){
    int finCS,baseCS,tamCS;

    baseCS = ((MV->TDS[(MV->R[CS] & 0XFFFF0000) >> 16] ) & 0XFFFF0000) >> 16;
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
    while(MV->R[IP]<finCS){ //MIENTRAS HAYA INSTRUCCIONES PARA LEER (BYTE A BYTE).

        DirFisicaActual = direccionamiento_logtofis(MV,MV->R[IP],0);
        ComponentesInstruccion(MV,DirFisicaActual,&CantOp); //TIPO INSTRUCCION, identifico los tipos y cantidad de operadores y el codigo de operacion
        if ((MV->R[OPC] >= 0) && ((MV->R[OPC] <= 8) || ((MV->R[OPC]<=31) && (MV->R[OPC]>=15))) ){ // Si el codigo de operacion es valido
            if (CantOp != 0) //Guardo los operandos que actuan en un auxiliar, y tambien guardo el tamanio del operando
               SeteoValorOp(MV, DirFisicaActual); // Distingue entre uno o dos operandos a setear

           //Avanzo a la proxima instruccion. FIX: Mueve el puntero de IP antes de llamar a la funcion, asi funcionan los SALTOS.
            MV->R[IP]=MV->R[IP] + ( (MV->R[OP1] >> 24 ) &  0XFF ) + ( (MV->R[OP2] >> 24 ) & 0XFF) + 1 ;
            Funciones[MV->R[OPC]](MV);
        }else
            generaerror(1);

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

/*void DefinoRegistro(unsigned char *Sec , unsigned char *CodReg, int Op){  //Defino el sector del registro en el que operare y el tipo de registro
  *Sec = (Op >> 2) & 0x03;
  *CodReg = (Op >> 4) & 0xF;
}// Devuelve Sector y Codigo de Registro.

void DefinoAuxRegistro(int *AuxR,TMV MV,unsigned char Sec,int CodReg){ //Apago las posiciones del registro de 32 bytes en el que asignare a otro registro/memoria
  int CorroSigno=0;
  if (Sec == 1){
        *AuxR = MV.R[CodReg] & 0XFF;
        CorroSigno = 24;
    }
      else
        if (Sec == 2){
          *AuxR = (MV.R[CodReg] & 0XFF00) >> 8;
          CorroSigno = 16;
        }
          else
            if (Sec == 3){
              *AuxR = MV.R[CodReg] & 0XFFFF;
              CorroSigno = 16;
            }
            else
                *AuxR = MV.R[CodReg];

    *AuxR = *AuxR << CorroSigno;
    *AuxR = *AuxR >> CorroSigno;

}
*/
int LeoEnMemoria(TMV *MV,int Op){ // Guarda el valor de los 4 bytes de memoria en un auxiliar
    int aux=0,PosMemoria,offset,CodReg,puntero;

    offset= Op & 0XFFFF;
    CodReg=(Op>>16)&0x1F;

    puntero=MV->R[CodReg]+offset;

    PosMemoria = direccionamiento_logtofis(MV,puntero,4);

    for (int i=0;i<4;i++){
        aux+=MV->MEM[PosMemoria];
        PosMemoria++;
        if (4-i > 1)
            aux=aux << 8;
    }

    MV->R[MBR] = aux;

    return aux;
}

void EscriboEnMemoria(TMV *MV,int Op, int Valor){ // Guarda el valor en 4 bytes de la memoria, se usa solo para el MOV

    //HAY QUE CHECKEAR ESTA FUNCION, LA USAMOS MUCHO Y TENEMOS QUE CHECKEAR SI OCURRE FALLO DE SEGMENTO.
    int offset,CodReg,puntero;
    int PosMemoria;

    offset= Op & 0XFFFF;
    CodReg=(Op>>16)&0x1F;

    //printf("%d offset \t %d codreg",offset,CodReg);
    puntero=(*MV).R[CodReg]+offset;
    PosMemoria = direccionamiento_logtofis(MV,puntero,4);
    MV->R[MBR] = Valor;

    for (int i=0;i<4;i++){
        MV->MEM[PosMemoria] = (Valor & 0XFF000000) >> 24;
        PosMemoria++;
        if (4-i > 1)
            Valor=Valor << 8;
    }
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
    unsigned char CodRegB;
    //OPB

    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){
        CodRegB = MV->R[OP2] & 0x1F;
        *auxOpB = MV->R[CodRegB];
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

void MOV(TMV * MV){
    int mover;
    unsigned char CodOpA;
    //OPB
    guardoOpB(MV,&mover);
    //OPA


    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] = (MV->R[CodOpA] & 0X0000000000000000) + mover;
    }
     else{ //Es memoria ya que no se puede guardar nada en un inmediato
        EscriboEnMemoria(MV,MV->R[OP1],mover);
    }

}

void ADD(TMV * MV){
    int sumar;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&sumar);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] += sumar;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&resta);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] -= resta;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
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
    int auxA=0,auxB=0;
    unsigned char codregA,codregB;

    //Saco primer operando a un auxiliar.
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        codregA = MV->R[OP1] & 0x1F;
        auxA=MV->R[codregA];
    }
    else{ // El operando A es de memoria.
        auxA=LeoEnMemoria(MV,MV->R[OP1]);
    }


    //Saco segundo operando a un auxiliar.
    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){
        codregB = MV->R[OP2] & 0x1F;
        auxB=MV->R[codregB];
    }
    else{
        auxB=LeoEnMemoria(MV,MV->R[OP2]);
    }


    // Hago "el MOV OpA,AuxB"
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        MV->R[codregA] = (MV->R[codregA] & 0X0000000000000000) + auxB;
    }
    else{ //OPERANDO A ES MEMORIA
        EscriboEnMemoria(MV,MV->R[OP1],auxB);
    }


    //Hago "el MOV OpB,AuxA"
    if (((MV->R[OP2] >> 24 ) &  0XFF ) == 1){ //Si Op1 es de registro, debo cambiar la posicion de memoria del registro por la que me diga el Op1
        MV->R[codregB] = (MV->R[codregB] & 0X0000000000000000) + auxA;
    }
    else{ //OPERANDO B ES MEMORIA
        EscriboEnMemoria(MV,MV->R[OP2],auxA);
    }
}

void MUL(TMV * MV){
    int mult;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&mult);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] *= mult;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;
    // unsigned char SecA

    //OPB
    guardoOpB(MV,&divisor);

    if (divisor == 0)
        generaerror(0);
    else{
    //OPA
    int Dividendo;
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] /= divisor;

        //int ResultadoSeg;
        //DefinoAuxRegistro(&Dividendo,*MV,SecA,CodOpA);
        Dividendo = MV->R[CodOpA];
        modificoCC(MV,MV->R[CodOpA]);
        MV->R[AC] = ( Dividendo % divisor);
    }
    else{ //Es memoria ya que no se puede guardar nada en un inmediato
        int AuxDiv, dividendo = LeoEnMemoria(MV,MV->R[OP1]);
        AuxDiv = (int) (dividendo / divisor);
        modificoCC(MV,AuxDiv);
        MV->R[AC] = dividendo % divisor;
        EscriboEnMemoria(MV,MV->R[OP1],AuxDiv);
    }
    }
}

void CMP(TMV * MV){
    int resta,resultado;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&resta);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        resultado = MV->R[CodOpA] - resta;
    }
     else //Memoria
        resultado = LeoEnMemoria(MV,MV->R[OP1]) - resta;


    modificoCC(MV,resultado);
}

void SHL(TMV * MV){ // Se propaga solo con ceros
    int auxSHL;
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxSHL);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] <<= auxSHL;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxSHR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] = (unsigned int) MV->R[CodOpA] >> auxSHR;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxSAR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] >>= auxSAR;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxAND);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
        CodOpA = MV->R[OP1] & 0x1F;
        MV->R[CodOpA] &= auxAND;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&auxOR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] |= auxOR;


      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;


    //OPB
    guardoOpB(MV,&auxXOR);

    //OPA
    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] ^= auxXOR;

      //int ResultadoSeg;
      //DefinoAuxRegistro(&ResultadoSeg,*MV,SecA,CodOpA);
      modificoCC(MV,MV->R[CodOpA]);
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
    unsigned char CodOpA;

    //OPB
    guardoOpB(MV,&tope);
    random = rand() % (tope + 1);

    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){
      CodOpA = MV->R[OP1] & 0x1F;
      MV->R[CodOpA] = random;
    }
     else //Es memoria ya que no se puede guardar nada en un inmediato
        EscriboEnMemoria(MV,MV->R[OP1],random);

}
//---------------------------------------------------------------DEBUG-------------------------------------------------------------------
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


// -------------------------------------- FUNCIONES CON 1 OPERANDO
void SYS (TMV *MV){
/*  Ejecuta la llamada al sistema indicada por el valor del operando.
    SYS 1 (READ): permite almacenar los datos leidos desde el teclado a partir de la posicion de memoria apuntada por EDX, guardandolo en CL celdas de tama o CH.
    El modo de lectura depende de la configuracion almacenada en AL.

    SYS 2 (WRITE): muestra en pantalla los valores contenidos a partir de la posicion de memoria apuntada por EDX, recuperando CL celdas de tama o CH.
    El modo de escritura depende de la configuracion almacenada en AL.

*/
    int i,j,operando,pos_inicial_memoria,numero;
    char modo,celdas,size,imprimible;
    char *bin;
    unsigned char CodReg;
    //unsigned char Sec;

    int TamA = (MV->R[OP1] >> 24 ) &  0XFF ;

    if(TamA==1){
        CodReg = MV->R[OP1] & 0x1F;
        operando = MV->R[CodReg];
    }
    else if (TamA==2){
        operando = MV->R[OP1] & 0XFFFF;
        operando = operando << 16;
        operando = operando >> 16;
    }
    else
        operando=LeoEnMemoria(MV,MV->R[OP1]);

    //SETEO VALORES

    modo = MV->R[EAX]& 0xFF;
    celdas = MV->R[ECX]& 0xFF; // Cant celdas en HIGH LDH
    size = (MV->R[ECX]>>16)& 0xFF; //Tamaño en LOW LDL
    pos_inicial_memoria=direccionamiento_logtofis(MV,MV->R[EDX],celdas*size);


    if(operando==1){    //READ
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
            // ESTA PARTE ESTA BIEN SI CH SOLO PUEDE TOMAR VALORES DE 1 A 4. HAY QUE PREGUNTAR Y CORREGIR CON ALGUN FOR SINO.
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
        for (i=0;i<celdas;i++){
            printf("[%04X] ",pos_inicial_memoria);
            // PASA LO MISMO CON EL WRITE. SI CH SOLO PUEDE TOMAR VALORES DE 1 A 4 ESTA BIEN, SINO HAY QUE CORREGIR CON ALGUN FOR.
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
            /*  IMPLEMENTADO CON UN FOR SERIA:
                if (size >= 1 && size <= 4) {
                // Leo el primer byte sin desplazar
                numero = (*MV).MEM[pos_inicial_memoria++];
                // Para cada byte adicional, desplazo y concateno
                for (int i = 1; i < size; ++i) {
                    numero = (numero << 8) | (*MV).MEM[pos_inicial_memoria++];
                }
            */
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
    }
    else
        generaerror(1); //ESTO NO SE SI SE HACE PERO BUENO.

}

void JMP (TMV *MV){
    //Efectua un salto incondicional a la celda del segmento de codigo indicada en el operando.
    int asignable;
    // int auxReg;
    unsigned char CodOp;


    if (((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ // Operando de registro
        CodOp = MV->R[OP1] & 0x1F;
        asignable= MV->R[CodOp];
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
    unsigned char CodOp;
    // unsigned char auxSec,auxCodReg;

    if(((MV->R[OP1] >> 24 ) &  0XFF ) == 1){ //Operando de registro
        CodOp = MV->R[OP1] & 0x1F;
        aux = MV->R[CodOp];
        resultado=~aux;
        MV->R[CodOp] = resultado;
    }
    else if (((MV->R[OP1] >> 24 ) &  0XFF ) == 3){ //Operando de memoria
        aux=LeoEnMemoria(MV,MV->R[OP1]);
        resultado=~aux;
        EscriboEnMemoria(MV,MV->R[OP1],resultado);
    }

    modificoCC(MV,resultado);
}
// -------------------------------------- FUNCIONES SIN OPERANDO
void STOP(TMV *MV){
    exit (0);
}

void LeoInstruccionesDissasembler(TMV *MV,char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4]) {

    int CantOp;
    unsigned short int PosInicial,PosMemoria,PosFinal;

    PosMemoria = direccionamiento_logtofis(MV,MV->R[CS],0);
    PosFinal = posmaxCODESEGMENT(MV);
    while (PosMemoria < PosFinal) {

        PosInicial=PosMemoria;
        ComponentesInstruccion(MV,PosMemoria,&CantOp);
        SeteoValorOp(MV,PosMemoria);

        PosMemoria +=  ( (MV->R[OP1] >> 24 ) &  0XFF ) + ( (MV->R[OP2] >> 24 ) & 0XFF) + 1; // Posicion de la Siguiente instruccion
        EscriboDissasembler(*MV,VecFunciones,VecRegistros,MV->R[OPC],PosInicial,PosMemoria);
    }
}

void EscriboDissasembler(TMV MV, char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4], unsigned char CodOp,unsigned short int PosInicial,unsigned short int PosMemoria){

    short int Offset;
    unsigned char CodReg;
    int i;
    char AuxSeg[4];
    int TamA = (MV.R[OP1] >> 24 ) &  0XFF ;
    int TamB = (MV.R[OP2] >> 24 ) &  0XFF ;

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
            CodReg=(MV.R[OP1] >>16) & 0x1F;
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
            CodReg = MV.R[OP1] & 0x1F;
            if (CodReg >= 10 && CodReg<=15){
                strcpy(AuxSeg,VecRegistros[CodReg]);
                //GuardoSector(AuxSeg,SecA);
                //if (SecA == 0)
                printf("%c%s",'E',AuxSeg);
                //else
                  //printf("%s",AuxSeg);
            }
            else
               printf("%s",VecRegistros[CodReg]);
        }

        if (TamB!=0) {
            printf(", ");
            if (TamB == 3) {  //Memoria
                Offset= MV.R[OP2] & 0XFFFF;
                CodReg=(MV.R[OP2] >>16) & 0x1F;
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
                  CodReg = MV.R[OP2] & 0x1F;
                  if (CodReg >= 10 && CodReg<=15){
                     strcpy(AuxSeg,VecRegistros[CodReg]);
                     //GuardoSector(AuxSeg,SecB);
                     //if (SecB == 0)
                     printf("%c%s",'E',AuxSeg);
                     //else
                       // printf("%s",AuxSeg);
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
