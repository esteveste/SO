/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
//Grupo 16
*/

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
  int id, n_linhas, N, iter, numIteracoes;
}Info;


/*--------------------------------------------------------------------
| Function: simul_thread 
---------------------------------------------------------------------*/

double *simul_thread(int id, int n_linhas, int N, int iter, int numIteracoes) {
  DoubleMatrix2D *temp;

  // for (int n = 0; n < numIteracoes; ++n)
  // {

  //   for (int i = 1; i < linhas-1; ++i)
  //     for (int j = 1; j < colunas-1; ++j)
  //       dm2dSetEntry(matrix_aux,i,j,(dm2dGetEntry(matrix,i-1,j) + dm2dGetEntry(matrix,i+1,j) + dm2dGetEntry(matrix,i,j-1) + dm2dGetEntry(matrix,i,j+1))/4);
    
  //   temp = matrix;
  //   matrix = matrix_aux;
  //   matrix_aux = temp;

  // }
    // return matrix;
}
/*--------------------------------------------------------------------
| Function: fnThread
| Descricao: funcao aux que cumpre certa Interface entre
| o simul_thread e a funcao pthread_create
---------------------------------------------------------------------*/
void *fnThread(void *arg) {
  Info *x;
  double *r;
  x = (Info*)arg;
  //result
  //NOTE FAZER ISTO DO SIZEOF
  // r = (double*)malloc(sizeof(int));


  r = simul_thread(x->id,x->n_linhas,x->N,x->iter,x->numIteracoes);
  return r;
}
/*--------------------------------------------------------------------
| Function Aux: init_simul_thread, Inicializa os threads da simul
---------------------------------------------------------------------*/
void init_simul_thread(int i,int n_linhas,pthread_t* tid,int n_tarefas){
  //n fazer antes
  if(i !=0){
    enviarMensagem(MAIN_ID, tid[i-1], void *msg, int tamanho);
  }
  //nem depois da tarefa
  if(i != n_tarefas -1){
    enviarMensagem(MAIN_ID, tid[i+1], void *msg, int tamanho);
  }
}

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

  if(N<1 || tEsq < 0 || tSup < 0 || tDir < 0|| tInf < 0 || iteracoes < 1 || trab < 0 || csz < 0 || N % trab == 0){
    fprintf(stderr,"Contexto dos argumentos invalido\n");
    return 1;
  }


  DoubleMatrix2D *matrix, *matrix_aux, *result;


  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d ntasks=%d capacidade_de_cada_canal=%d\n",
	N, tEsq, tSup, tDir, tInf, iteracoes,trab,csz);


  matrix = dm2dNew(N+2, N+2);
  matrix_aux = dm2dNew(N+2, N+2);

  dm2dSetLineTo (matrix, 0, tSup);
  dm2dSetLineTo (matrix, N+1, tInf);
  dm2dSetColumnTo (matrix, 0, tEsq);
  dm2dSetColumnTo (matrix, N+1, tDir);

  dm2dCopy (matrix_aux, matrix);
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
    args[i].n_linhas = n_linhas;
    args[i].iter = 0;
    args[i].numIteracoes = iteracoes;


    if (pthread_create (&tid[i], NULL, fnThread, &args[i]) != 0){
      printf("Erro ao criar tarefa.\n");
      return 1;
    }

    printf("Lancou uma tarefa nr: %d\n",i);
  }

  // initialize threads
  for (i=0; i<trab; i++) {

    init_simul_thread(i,n_linhas,tid,trab);

  }

  //wait for finish thread
  for (i=0; i<N; i++) {
    if (pthread_join (tid[i], (void**)&result) != 0) {
      printf("Erro ao esperar por tarefa.\n");
      return 2;
    }
    printf("Tarefa retornou com resultado = %d\n", *result);
  }

  result = simul(matrix, matrix_aux, N+2, N+2, iteracoes);

  dm2dPrint(result);

  dm2dFree(matrix);
  dm2dFree(matrix_aux);
  //terminar as tarefas
  libertarMPlib();

  return 0;
}
