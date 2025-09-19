#ifndef TMV_H_INCLUDED
#define TMV_H_INCLUDED
#define TMEM 16384
#define CANTFUNC 32
#define CANTREG 32
#define CANTERRORES 3
#define TDDSSIZE 8
//DEFINES PARA EL SYS
#define BITS_32 32
#define BUF_SIZE (BITS_32 + 2)

// Define de registros
#define LAR 0
#define MAR 1
#define MBR 2
#define IP 3
#define OPC 4
#define OP1 5
#define OP2 6

#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15

#define AC 16
#define CC 17

#define CS 26
#define DS 27

//Estan todas las funciones que solicitaba el enunciado del TP de MV pero no llegu  a corregir los errores de compilacion, estar n corregidos para la siguiente entrega de la parte 1
typedef struct theader{
    unsigned char c1,c2,c3,c4,c5;
    char version;
    unsigned short int tam;
}theader;

typedef struct TMV{
  unsigned char MEM[TMEM]; //Memoria, unsigned ya que la memoria no puede usar complemento a 2, nosotros tenemos que darle la interpretaci n de si es negativo o positivo.
  int R[CANTREG]; //Registros
  int TDS[TDDSSIZE]; //Segmentos
  int Errores[CANTERRORES];
}TMV;



typedef void (*TFunc[CANTFUNC])(TMV *mv); //Array de punteros a funciones

void inicializoTDS(TMV* MV,short int TamCS);
void inicializoRegistros(TMV *MV);
void inicializoErrores(TMV *MV);
void generaerror(int tipo);
void inicializoVecFunciones(char VecFunciones[CANTFUNC][5]); //PARA DISASSEMBLER
void inicializoVecRegistros(char VecRegistros[CANTREG][4]);  //PARA DISASSEMBLER
void declaroFunciones(TFunc Funciones);
void LeoArch(char nomarch[],TMV *MV);
int posmaxCODESEGMENT(TMV *MV);
int direccionamiento_logtofis(TMV *MV, int puntero, int bytes);
void LeoInstruccion(TMV* MV);
void ComponentesInstruccion(TMV *MV,int DirFisica, int *CantOp);
void SeteoValorOp(TMV *MV,int DirFisicaActual);
//void DefinoRegistro(unsigned char *Sec , unsigned char *CodReg, int Op);
//void DefinoAuxRegistro(int *AuxR,TMV MV,unsigned char Sec,int CodReg);
int LeoEnMemoria(TMV *MV,int Op);
int GuardoValorMemoria(TMV *MV,int Op);
void EscriboEnMemoria(TMV *MV,int Op, int Valor);
void modificoCC(TMV *MV,int Resultado);
void guardoOpB(TMV *MV, int *auxOpB);
char sobrepasaCS(TMV *MV,int asignable);
int devuelveN(TMV *MV);
int devuelveZ(TMV *MV);
int leer_binario_c2_32(void);
char *int_to_c2bin(int numero);

//DEBUG
void muestramemoria(unsigned char memoria[]);
void muestraregistros(int reg[]);
void muestratds(int tds[]);
void muestravaloresmv(TMV *mv);
char obtienetipooperacion(unsigned char operacion);

//FUNCIONES

//2 OPERANDOS
void MOV(TMV * MV);

void ADD(TMV * MV);

void SUB(TMV * MV);

void SWAP(TMV * MV);

void MUL(TMV * MV);

void DIV(TMV * MV);

void CMP(TMV * MV);

void SHL(TMV * MV);

void SHR(TMV * MV);

void SAR(TMV * MV);

void AND(TMV * MV);

void OR(TMV * MV);

void XOR(TMV * MV);

void LDL(TMV * MV);

void LDH(TMV * MV);

void RND(TMV * MV);

//1 OPERANDO

void SYS(TMV * MV);

void JMP(TMV * MV);

void JZ(TMV * MV);

void JP(TMV * MV);

void JN(TMV * MV);

void JNZ(TMV * MV);

void JNP(TMV * MV);

void JNN(TMV * MV);

void NOT(TMV * MV);

//0 OPERANDOS

void STOP(TMV * MV);

// DISSASSEMBLER
void LeoInstruccionesDissasembler(TMV *MV,char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4]);
void EscriboDissasembler(TMV MV, char VecFunciones[CANTFUNC][5],char VecRegistros[CANTREG][4], unsigned char CodOp,unsigned short int PosInicial,unsigned short int PosMemoria);
void GuardoSector(char Segmento[4],unsigned char Sec);

#endif // MV_H_INCLUDED
