
/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
//Grupo 16
*/

//Fazer error Handle
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <errno.h>

#include "matrix2d.h"
#include "mplib3.h"

//constants
#define MAIN_ID 0

typedef struct {
  int id, n_linhas, N, iter, numIteracoes,trab;
  int tEsq, tSup, tDir, tInf;
}Info;


/*--------------------------------------------------------------------
| Function: simul_thread 
---------------------------------------------------------------------*/

void simul_thread(int id, int n_linhas, int N, int numIteracoes, int trab)
{
  //initializacao
  //util para ter nocao do codigo
  int colunas = N+2;
  int linhas = n_linhas + 2;

  DoubleMatrix2D *fatia, *fatia_aux, *temp;
  fatia = dm2dNew(linhas, colunas);
  fatia_aux = dm2dNew(linhas,colunas);

  //receber fatia da main
  for(int n = 0;n<linhas;n++){
    if(receberMensagem(MAIN_ID, id, dm2dGetLine(fatia,n),sizeof(double) * colunas) == -1){
      perror("Error Receiving Message from Main thread:Initialization");exit(1);}
  }

  //acabar com as mesmas 2 fatias
  dm2dCopy (fatia_aux, fatia);

  //processar
  for (int n = 0; n < numIteracoes; ++n)
  {
    //calculation
    for (int i = 1; i < linhas-1; ++i)
      for (int j = 1; j < colunas-1; ++j)
        dm2dSetEntry(fatia_aux,i,j,(dm2dGetEntry(fatia,i-1,j) + dm2dGetEntry(fatia,i+1,j) + dm2dGetEntry(fatia,i,j-1) + dm2dGetEntry(fatia,i,j+1))/4);
    
    //change fatia
    temp = fatia;
    fatia = fatia_aux;
    fatia_aux = temp;

    //enviar e receber linhas necessarias para calculos de outras threads
    if(id !=1){
      if(receberMensagem(id - 1, id, dm2dGetLine(fatia,0),sizeof(double) * colunas) == -1){
        perror("Error Receiving Message from previous thread");exit(1);}
    }
    if(id !=1){

      if (enviarMensagem(id, id - 1, dm2dGetLine(fatia, 1), sizeof(double) * colunas) == -1){
        perror("Error Sending Message to previous thread");exit(1);}
      
    }
    if(id != trab){
      if(enviarMensagem(id, id + 1, dm2dGetLine(fatia, n_linhas), sizeof(double) * colunas) == -1){
        perror("Error Sending Message to next thread");exit(1);}
  }
    if (id != trab)
    {
      if(receberMensagem(id + 1, id, dm2dGetLine(fatia,n_linhas + 1),sizeof(double) * colunas) == -1){
        perror("Error Receiving Message from next thread");exit(1);}
    }

}

  // return fatia
  int j =0;
  for (; j < n_linhas; j++)
  {
    if(enviarMensagem(id, MAIN_ID, dm2dGetLine(fatia, j + 1), sizeof(double)*(N+2)) == -1){
      perror("Error Sending Message to Main");exit(1);}
  }

  //fazer free da fatias aux
  dm2dFree(fatia_aux);
  dm2dFree(fatia);
}

/*--------------------------------------------------------------------
| Function: fnThread
| Descricao: funcao aux que cumpre certa Interface entre
| o simul_thread e a funcao pthread_create
---------------------------------------------------------------------*/
void *fnThread(void *arg) {
  Info *x;
  x = (Info*)arg;

  simul_thread(x->id,x->n_linhas,x->N,x->numIteracoes,x->trab);
  return NULL;
}

/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
  int value;
 
  if(sscanf(str, "%d", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name)
{
  double value;

  if(sscanf(str, "%lf", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {

  if(argc != 9) {
    fprintf(stderr, "\nNumero invalido de argumentos.\n");
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trab csz\n\n");
    return 1;
  }

  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  int iteracoes = parse_integer_or_exit(argv[6], "iteracoes");
  int trab = parse_integer_or_exit(argv[7], "trab"); //nr tarefas
  int csz = parse_integer_or_exit(argv[8], "csz"); //nr mensagens por canal

  if(N<1 || tEsq < 0 || tSup < 0 || tDir < 0|| tInf < 0 || iteracoes < 1 || trab < 1 || csz < 0 || N % trab != 0){
    fprintf(stderr,"Contexto dos argumentos invalido\n");
    return 1;
  }

  DoubleMatrix2D *matrix;

  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d ntasks=%d capacidade_de_cada_canal=%d\n",
	N, tEsq, tSup, tDir, tInf, iteracoes,trab,csz);


  matrix = dm2dNew(N+2, N+2);

  //dm2dNew Error Handling
  if (matrix == NULL) {
    fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
    return -1;
  }

  dm2dSetLineTo (matrix, 0, tSup);
  dm2dSetLineTo (matrix, N+1, tInf);
  dm2dSetColumnTo (matrix, 0, tEsq);
  dm2dSetColumnTo (matrix, N+1, tDir);

  //Fim initializar Matrizes

  // + 1 because of Main
  if (inicializarMPlib(csz,trab + 1)==-1){//error handling
    perror("Erro ao inicializar MPLib.");exit(1);}

  //threading
  pthread_t tid[trab];
  Info* args = (Info *) malloc(sizeof(Info)*trab);

  //definir INFO

  int n_linhas = N / trab;
  int i = 0;
  for (; i<trab; i++) {
    //init args
    args[i].id = i+1;
    args[i].N = N;
    args[i].trab = trab;
    args[i].n_linhas = n_linhas;
    args[i].numIteracoes = iteracoes;

    if (pthread_create (&tid[i], NULL, fnThread, &args[i]) != 0){
      perror("Erro ao criar tarefa.");
      return 1;
    }

    printf("Lancou uma tarefa nr: %d\n",i);
  }

  //send matrix to threads
  int j =0;
  int last = 0;
  for (i=0; i<trab; i++) {
    last += n_linhas +2;
    for (; j < last; j++){
      if(enviarMensagem(MAIN_ID, i+1, dm2dGetLine(matrix, j), sizeof(double) * (N+2))==-1){
        perror("Error sending Message from Main to threads");exit(1);}
      }
    j-=2;
    last-=2;
  }

  //receive output from threads
  for (i=0; i<trab; i++) {
    int j =0;
    for (; j < n_linhas; j++)
    {
      //i+1 visto q o id e index 1
      //dm2GetLine visto q esta funcao manda o ponteiro do 1 elemento da linha 
      //e o receber preenche esse ponteiro
      if(receberMensagem(i+1, MAIN_ID, dm2dGetLine(matrix, i*n_linhas + j + 1),sizeof(double)*(N+2))==-1){
        perror("Error Receiving Message in Main");exit(1);}
    }
  }

  //wait for finish thread
  for (i=0; i<trab; i++) {
    if (pthread_join (tid[i], NULL) != 0) {
      perror("Erro ao esperar por tarefa terminar.\n");
      return 2;
    }
  }

  //fazer print do result
  dm2dPrint(matrix);

  //free of vars
  free(args);
  dm2dFree(matrix);
  // terminar o servico de troca de mensagens
  libertarMPlib();

  return 0;
}