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

#include "matrix2d.h"
#include "mplib3.h"

//constants
#define MAIN_ID -1


//Heap vars

typedef struct {
  int id, n_linhas, N, iter, numIteracoes,trab;
  int tEsq, tSup, tDir, tInf;
}Info;


/*--------------------------------------------------------------------
| Function: simul_thread 
---------------------------------------------------------------------*/

DoubleMatrix2D *simul_thread(int id, int n_linhas, int N, int iter, int numIteracoes, int tSup, int tInf, int tEsq, int tDir,int trab)
{
  //initializacao
  printf("Init\n");
  DoubleMatrix2D *matrix, *matrix_aux, *temp;
  matrix = dm2dNew(n_linhas + 2, N+2);
  matrix_aux = dm2dNew(n_linhas + 2,N+2);



  if(id ==0){
    dm2dSetLineTo (matrix, 0, tSup);
    
  }
  if(id == trab - 1){
    dm2dSetLineTo (matrix, n_linhas+1, tInf);
  }

  dm2dSetColumnTo (matrix, 0, tEsq);
  dm2dSetColumnTo (matrix, N+1, tDir);
  
  //finish in the same 2 matrix
  dm2dCopy (matrix_aux, matrix);
  printf("Init finished\n");

  ////////////////////////////////////////
  //processar
  int colunas = N+2;
  int linhas = n_linhas + 2;
  double *buffer;
  buffer = (double *) malloc(sizeof(double) * colunas);
  for (int n = 0; n < numIteracoes; ++n)
  {
    if(n!=0){
      if(id !=0){
        receberMensagem(id -1, id, buffer,sizeof(double) * colunas);
        dm2dSetLine(matrix,0,buffer);
      }
      if (id != trab -1)
      {
        receberMensagem(id +1, id, buffer,sizeof(double) * colunas);
        dm2dSetLine(matrix,n_linhas + 1,buffer);
      }

    }


    //calculation
    for (int i = 1; i < linhas-1; ++i)
      for (int j = 1; j < colunas-1; ++j)
        dm2dSetEntry(matrix_aux,i,j,(dm2dGetEntry(matrix,i-1,j) + dm2dGetEntry(matrix,i+1,j) + dm2dGetEntry(matrix,i,j-1) + dm2dGetEntry(matrix,i,j+1))/4);
    
    //change matrix
    temp = matrix;
    matrix = matrix_aux;
    matrix_aux = temp;


    if(id !=0){
      enviarMensagem(id, id -1, dm2dGetLine(matrix, 1), sizeof(double) * colunas);
    }
    if(id != trab -1){
      enviarMensagem(id, id +1, dm2dGetLine(matrix, n_linhas), sizeof(double) * colunas);
    }

  }
  printf("retunr\n");
  //fazer free


    return matrix;
}
/*--------------------------------------------------------------------
| Function: fnThread
| Descricao: funcao aux que cumpre certa Interface entre
| o simul_thread e a funcao pthread_create
---------------------------------------------------------------------*/
void *fnThread(void *arg) {
  Info *x;
  DoubleMatrix2D *r;
  x = (Info*)arg;
  //result
  //can the output be a pointer?
  // r = (double*)malloc(sizeof(int));

  r = simul_thread(x->id,x->n_linhas,x->N,x->iter,x->numIteracoes,x->tSup,x->tInf,x->tEsq,x->tDir,x->trab);
  return r;
}
// /*--------------------------------------------------------------------
// | Function Aux: init_simul_thread, Inicializa os threads da simul
// ---------------------------------------------------------------------*/
// void init_simul_thread(int id,int N,int n_linhas,pthread_t* tid,int n_tarefas,DoubleMatrix2D *matrix){

//   for (int i = id * n_linhas; i < n_linhas; i++)
//   {
    
//   }



//   if(id !=0){
//     enviarMensagem(MAIN_ID, tid[id-1], dm2dGetLine(matrix, int line_nb), sizeof(double) * N);
//   }
//   //nem depois da tarefa
//   if(id != n_tarefas -1){
//     enviarMensagem(MAIN_ID, tid[id+1], dm2dGetLine(matrix, int line_nb), sizeof(double) * N);
//   }
// }

/*--------------------------------------------------------------------
| Function: simul (iterative)
---------------------------------------------------------------------*/

DoubleMatrix2D *simul(DoubleMatrix2D *matrix, DoubleMatrix2D *matrix_aux, int linhas, int colunas, int numIteracoes) {
  DoubleMatrix2D *temp;

  for (int n = 0; n < numIteracoes; ++n)
  {

    for (int i = 1; i < linhas-1; ++i)
      for (int j = 1; j < colunas-1; ++j)
        dm2dSetEntry(matrix_aux,i,j,(dm2dGetEntry(matrix,i-1,j) + dm2dGetEntry(matrix,i+1,j) + dm2dGetEntry(matrix,i,j-1) + dm2dGetEntry(matrix,i,j+1))/4);
    
    temp = matrix;
    matrix = matrix_aux;
    matrix_aux = temp;

  }
    return matrix;
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

  if(N<1 || tEsq < 0 || tSup < 0 || tDir < 0|| tInf < 0 || iteracoes < 1 || trab < 0 || csz < 0 || N % trab != 0){
    fprintf(stderr,"Contexto dos argumentos invalido\n");
    return 1;
  }


  DoubleMatrix2D *matrix,*result;


  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d ntasks=%d capacidade_de_cada_canal=%d\n",
	N, tEsq, tSup, tDir, tInf, iteracoes,trab,csz);


  matrix = dm2dNew(N+2, N+2);
  // matrix_aux = dm2dNew(N+2, N+2);

  dm2dSetLineTo (matrix, 0, tSup);
  dm2dSetLineTo (matrix, N+1, tInf);
  // dm2dSetColumnTo (matrix, 0, tEsq);
  // dm2dSetColumnTo (matrix, N+1, tDir);

  // dm2dCopy (matrix_aux, matrix);
  //Fim initializar Matrizes


  inicializarMPlib(csz,trab);

  //threading
  pthread_t tid[trab];
  Info* args = (Info *) malloc(sizeof(Info)*trab);

  //definir INFO

  // for(;i<trab;i++){

  // }
  int n_linhas = N / trab;
  int i = 0;
  for (; i<trab; i++) {
    //init args
    args[i].id = i;
    args[i].N = N;
    args[i].trab = trab;
    args[i].n_linhas = n_linhas;
    args[i].iter = 0;
    args[i].numIteracoes = iteracoes;
    args[i].tDir = tDir;
    args[i].tEsq = tEsq;
    args[i].tSup = tSup;
    args[i].tInf = tInf;


    if (pthread_create (&tid[i], NULL, fnThread, &args[i]) != 0){
      printf("Erro ao criar tarefa.\n");
      return 1;
    }

    printf("Lancou uma tarefa nr: %d\n",i);
  }

  // // initialize threads
  // for (i=0; i<trab; i++) {

  //   init_simul_thread(i,N,n_linhas,tid,trab,matrix);

  // }

  //wait for finish thread
  for (i=0; i<trab; i++) {
    if (pthread_join (tid[i], (void**)&result) != 0) {
      printf("Erro ao esperar por tarefa.\n");
      return 2;
    }

    //put lines in matrix
    // for (int j = 0; j < n_linhas; j++)
    // {
    //   dm2dSetLine(matrix, i*n_linhas + j + 1, dm2dGetLine(result, j+1));
    // }
    // printf("Tarefa retornou com resultado = %f\n", result->data[0]);
    dm2dPrint(result);
  }

  // result = simul(matrix, matrix_aux, N+2, N+2, iteracoes);

  // dm2dPrint(matrix);

  // dm2dFree(matrix);
  // dm2dFree(matrix_aux);
  // //terminar as tarefas
  libertarMPlib();

  return 0;
}
