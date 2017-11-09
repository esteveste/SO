/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

/*Todo:
barreira
check iterations finish
alternancia matrizes, seja melhor calcular por iteracao par




*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"

/*--------------------------------------------------------------------
| Estruturas
---------------------------------------------------------------------*/
typedef struct {
  DoubleMatrix2D *matrix, *matrix_aux;
} *SimulArg;


typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond1,cond2;
} Barreira;

/*--------------------------------------------------------------------
| Global Vars
---------------------------------------------------------------------*/
DoubleMatrix2D *matrix, *matrix_aux;
int tar; //variavel global com o nr de tarefas
int count;//para uso no esperar por todos
int iter;//current iteration
int max_iter;//max iteration provided by user
Barreira barreira;

/*--------------------------------------------------------------------
| Function: wait
---------------------------------------------------------------------*/

void esperar_por_todos(){
  pthread_mutex_lock(&barreira.mutex);
  count--;
  if (count==0)
  {
    // pthread_cond_broadcast();
  }else{
    // pthread_cond_wait();
  }





  pthread_mutex_unlock(&barreira.mutex);
}


/*--------------------------------------------------------------------
| Function: simul
| Description: Função executada por cada tarefa trabalhadora.
|              Recebe como argumento uma estrutura do tipo SimulArg.
---------------------------------------------------------------------*/

void *simul(void* args) {


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
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf max_iter tarefas maxD\n\n");
    return 1;
  }

  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  max_iter = parse_integer_or_exit(argv[6], "max_iter");
  tar = parse_integer_or_exit(argv[7], "tarefas"); //var global
  double maxD = parse_double_or_exit(argv[8], "maxD");


  /* Verificacao argumentos*/
  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f max_iter=%d tarefas=%d maxD=%.1f\n",
	N, tEsq, tSup, tDir, tInf, max_iter,tar,maxD);

  if(N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || max_iter < 1 || tar < 1 || maxD<0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
	" Lembrar que N >= 1, temperaturas >= 0, max_iter >= 1, tarefas >= 1 e maxD >= 0\n\n");
    return 1;
  }  
  ///////////////////////

  count = tar;//definir o count para o espera por todos


  matrix = dm2dNew(N+2, N+2);
  matrix_aux = dm2dNew(N+2, N+2);

  if (matrix == NULL || matrix_aux == NULL) {
    fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
    return -1;
  }

  int i;
  int res;//used for error handle

  dm2dSetLineTo (matrix, 0, tSup);
  dm2dSetLineTo (matrix, N+1, tInf);
  dm2dSetColumnTo (matrix, 0, tEsq);
  dm2dSetColumnTo (matrix, N+1, tDir);

  dm2dCopy (matrix_aux, matrix);

  // result = simul(matrix, matrix_aux, N+2, N+2, max_iter);

  // if (result == NULL) {
  //   printf("\nErro na simulacao.\n\n");
  //   return -1;
  // }

  // dm2dPrint(result);

  /* Reservar Memória para Trabalhadoras */
  SimulArg simul_args = (SimulArg)malloc(tar * sizeof(SimulArg));
  pthread_t* tid = (pthread_t *)malloc(tar * sizeof(pthread_t));

  if (simul_args == NULL || tid == NULL) {
    fprintf(stderr, "\nErro ao alocar memória para trabalhadoras.\n");    
    return -1;
  }

  /* Criar Trabalhadoras */
  for (i = 0; i < tar; i++) {
    simul_args[i].matrix=matrix;
    simul_args[i].matrix_aux = matrix_aux;
    res = pthread_create(&tid[i], NULL, simul, &simul_args[i]);

    if(res != 0) {
      fprintf(stderr, "\nErro ao criar uma tarefa trabalhadora.\n");
      return -1;
    }
  }




  /* Esperar que as Trabalhadoras Terminem */
  for (i = 0; i < tar; i++) {
    res = pthread_join(tid[i], NULL);
    
    if (res != 0) {
      fprintf(stderr, "\nErro ao esperar por uma tarefa trabalhadora.\n");    
      return -1;
    }  
  }



  dm2dFree(matrix);
  dm2dFree(matrix_aux);

  return 0;
}
