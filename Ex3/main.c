/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

/*Todo:
check iterations finish

destroy barrier

*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"
#include <math.h>

/*--------------------------------------------------------------------
| Estruturas
---------------------------------------------------------------------*/
typedef struct{
  int counterMax;
  int count[2];//para uso no esperar por todos
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} Barrier;

typedef struct {
  DoubleMatrix2D *matrix, *matrix_aux;
  Barrier* bar;
  int id,N;
  double maxD;
} SimulArg;

/*--------------------------------------------------------------------
| Global Vars
---------------------------------------------------------------------*/
DoubleMatrix2D *matrix, *matrix_aux;
int tar; //variavel global com o nr de tarefas
// int iter;//current iteration
int max_iter;//max iteration provided by user
//1 when is over,(1 because only when we dont set is over)
int is_finished =1;
int last_iter;


/*--------------------------------------------------------------------
| Function: init_barrier
/ initializes the barrier
---------------------------------------------------------------------*/

Barrier* new_barrier(int counterMax){
  //create a Barreira object
  Barrier* bar = (Barrier*)malloc(sizeof(Barrier));


  //initialize counters
  bar->count[0]=counterMax;
  bar->count[1]=counterMax;

  //initialize mutex
  if(pthread_mutex_init(&bar->mutex, NULL) != 0) {
    fprintf(stderr, "\nErro ao inicializar mutex\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_cond_init(&bar->cond, NULL) != 0) {
    fprintf(stderr, "\nErro ao inicializar variável de condição\n");
    exit(EXIT_FAILURE);
  }
  return bar;
}

/*--------------------------------------------------------------------
| Function: destroy_barrier
/ destroyes the barrier
---------------------------------------------------------------------*/

void destroy_barrier(Barrier* bar){

  if(pthread_mutex_destroy(&bar->mutex) != 0) {
    fprintf(stderr, "\nErro ao destruir mutex\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_cond_destroy(&bar->cond) != 0) {
    fprintf(stderr, "\nErro ao destruir variável de condição\n");
    exit(EXIT_FAILURE);
  }

  free(bar);

}
/*--------------------------------------------------------------------
| Function: esperar_por_todos
---------------------------------------------------------------------*/

void esperar_por_todos(Barrier* bar){
  if(pthread_mutex_lock(&bar->mutex) != 0) {
    fprintf(stderr, "\nErro ao bloquear mutex\n");
    exit(EXIT_FAILURE);
  }
  //will change the barrier
  static int current = 0;

  bar->count[current]--;
  if (bar->count[current]==0)
  {
    //fazer reset a atual para ser usada na var cond
    bar->count[current]=tar;
    current = (current+1)%2;//mudar de contador

    if(pthread_cond_broadcast(&bar->cond) != 0) {
      fprintf(stderr, "\nErro ao desbloquear variável de condição\n");
      exit(EXIT_FAILURE);
    }
  }else{
    int old = current;//quando fizermos broadcast iremos mudar de contador
    while (current==old) //pelo q quando sair do wait usara a var old antiga
    {
      //so necessitamos de 1 contador visto q o current so permitira sair
      //quando fizermos o broadcast do wait(vindo do uso do contador antes do broadcast)
      if(pthread_cond_wait(&bar->cond,&bar->mutex) != 0) {
        fprintf(stderr, "\nErro ao esperar pela variável de condição\n");
        exit(EXIT_FAILURE);
      }
    }
  }


  if(pthread_mutex_unlock(&bar->mutex) != 0) {
    fprintf(stderr, "\nErro ao bloquear mutex\n");
    exit(EXIT_FAILURE);
  }
}


/*--------------------------------------------------------------------
| Function: simul
| Description: Função executada por cada tarefa trabalhadora.
|              Recebe como argumento uma estrutura do tipo SimulArg.
---------------------------------------------------------------------*/

void *simul(void* args) {
  SimulArg* arg = (SimulArg *)args;
  int i,j;
  int tam_fatia = arg->N /tar;
  int atual,prox;//for keep changing matrix
  int iter;

  DoubleMatrix2D *matrix_iter[2];
  matrix_iter[0]=arg->matrix;
  matrix_iter[1]=arg->matrix_aux;

  for(iter = 0;iter < max_iter;iter++) {
    //change matrix for calculation
    atual = iter % 2;
    prox = 1 - iter % 2;
    
    /* Calcular Pontos Internos */
    for (i = tam_fatia * arg->id; i < tam_fatia * (arg->id + 1); i++) {
      for (j = 0; j < arg->N; j++) {
        double val = (dm2dGetEntry(matrix_iter[atual], i, j+1) +
                      dm2dGetEntry(matrix_iter[atual], i+2, j+1) +
                      dm2dGetEntry(matrix_iter[atual], i+1, j) +
                      dm2dGetEntry(matrix_iter[atual], i+1, j+2))/4;
        
        //verificamos se ainda existe algum calculo acima do maxD
        //nao usamos mutexes pk a verificacao so e feita apos de se esperar q todos
        //o q implica q a alteracao e feita concorrentemente para o mesmo valor
        // e a verificacao so e verificada apos todas terem alterado
        if(fabs(dm2dGetEntry(matrix_iter[prox], i+1, j+1) - val) >= arg->maxD){
          is_finished = 0;
        }
        dm2dSetEntry(matrix_iter[prox], i+1, j+1, val);



        }
    }

    esperar_por_todos(arg->bar);
    //if is over  
    if (is_finished)
      break;//get of the calculation
    //espera q todos facam a verificacao ao mesmo tempo
    esperar_por_todos(arg->bar);

    is_finished=1;//reset flag
    esperar_por_todos(arg->bar);
  
  }
  //como todos sao iguais e a verificacao so e feita dps de todos
  // terminarem nao precisamos de mutex
  last_iter=iter;
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
  //Ur barrier for simul
  Barrier* bar = new_barrier(tar);

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
  SimulArg* simul_args = (SimulArg *)malloc(tar * sizeof(SimulArg));
  pthread_t* tid = (pthread_t *)malloc(tar * sizeof(pthread_t));

  if (simul_args == NULL || tid == NULL) {
    fprintf(stderr, "\nErro ao alocar memória para trabalhadoras.\n");    
    return -1;
  }

  /* Criar Trabalhadoras */
  for (i = 0; i < tar; i++) {
    simul_args[i].matrix=matrix;
    simul_args[i].matrix_aux = matrix_aux;
    simul_args[i].id = i;
    simul_args[i].N = N;
    simul_args[i].maxD=maxD;
    simul_args[i].bar=bar;
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

  /* Imprimir resultado */
  if(last_iter%2)
    dm2dPrint(matrix);
  else
    dm2dPrint(matrix_aux);
  
  /* Libertar Memória */
  destroy_barrier(bar);
  dm2dFree(matrix);
  dm2dFree(matrix_aux);
  free(simul_args);
  free(tid);

  return 0;
}
