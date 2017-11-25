/*
// 3 Projeto SO 
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"
#include <math.h>


#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
/*--------------------------------------------------------------------
| Estruturas
---------------------------------------------------------------------*/
typedef struct{
  int reset_counter;
  int count[2];//para uso no esperar por todos
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} Barrier;

typedef struct {
  Barrier* bar;
  int id,N,max_iter,tam_fatia;
  double maxD;
} SimulArg;

/*--------------------------------------------------------------------
| Global Vars
---------------------------------------------------------------------*/
//ambas podiam estar em estrutura para evitar variaveis globais mas so
//complicava o codigo, (static para limited scope, so para este ficheiro)
static int is_finished =1;//1 when is over,(1 because only when we dont set is over)
static int flag_exit_thread=0;//alterado para exercicio
char* file_name;
int periodoS;
DoubleMatrix2D *matrix, *matrix_aux;

/*--------------------------------------------------------------------
| Function: auto_save_handler
/ Function that handles the new process created for auto saving the matrix
---------------------------------------------------------------------*/
void auto_save_handler(){
  FILE *f=fopen(file_name,"w");//write over the file
  if (f==NULL)
  {
    fprintf(stderr, "\nErro abrir ficheiro para escrita\n");
    exit(EXIT_FAILURE);
  }

  saveMatrix2dToFile(f,matrix);
  fclose(f);
  exit(EXIT_SUCCESS);
}

/*--------------------------------------------------------------------
| Function: init_barrier
/ initializes the barrier,to simplefy code also contains error handle
---------------------------------------------------------------------*/

Barrier* new_barrier(int counterMax){
  //create a Barreira object
  Barrier* bar = (Barrier*)malloc(sizeof(Barrier));

  //initialize counters
  bar->count[0]=counterMax;
  bar->count[1]=counterMax;
  bar->reset_counter=counterMax;
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
/ destroyes the barrier,to simplefy code also contains error handle
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
| Function: wait_barrier
/ implementation of barrier,to simplefy code also contains error handle
/ in function
---------------------------------------------------------------------*/

void wait_barrier(Barrier* bar,int iter){
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
    bar->count[current]=bar->reset_counter;
    current = (current+1)%2;//mudar de contador

    //put the new calculated matrix in to matrix
    DoubleMatrix2D *temp = matrix;
    matrix = matrix_aux;
    matrix_aux = temp;
    //finish matrix flag
    if(is_finished){
      flag_exit_thread = 1;
    }else{
      is_finished=1;
    }
    
    //Auto save
    if(periodoS!=0 && iter % periodoS==0){
      pid_t pid = fork();
      if(pid==-1){
        fprintf(stderr, "\nErro ao criar um processo\n");
        exit(EXIT_FAILURE);
      }else if (pid == 0){
        // child process
        auto_save_handler();
      }else{
        // parent process
        //we wait for child to finish saving
        wait(NULL);
      }
    }


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
  int iter;

  for(iter = 0;iter < arg->max_iter;iter++) {

    /* Calcular Pontos Internos */
    for (i = arg->tam_fatia * arg->id; i < arg->tam_fatia * (arg->id + 1); i++) {
      for (j = 0; j < arg->N; j++) {
        double val = (dm2dGetEntry(matrix, i, j+1) +
                      dm2dGetEntry(matrix, i+2, j+1) +
                      dm2dGetEntry(matrix, i+1, j) +
                      dm2dGetEntry(matrix, i+1, j+2))/4;
        
        //verificamos se ainda existe algum calculo acima do maxD
        //nao usamos mutexes pk a verificacao so e feita apos de se esperar q todos
        //o q implica q a alteracao e feita concorrentemente para o mesmo valor
        // e a verificacao so e verificada apos todas terem alterado
        if(fabs(dm2dGetEntry(matrix_aux, i+1, j+1) - val) >= arg->maxD){
          is_finished = 0;
        }
        dm2dSetEntry(matrix_aux, i+1, j+1, val);
        }
    }

    wait_barrier(arg->bar,iter);
    //if is over  
    if (flag_exit_thread)
      break;//get of the calculation

  }
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

  if(argc != 11) {
    fprintf(stderr, "\nNumero invalido de argumentos.\n");
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf max_iter tarefas maxD fichS periodoS\n\n");
    return 1;
  }

  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  int max_iter = parse_integer_or_exit(argv[6], "max_iter");
  int tar = parse_integer_or_exit(argv[7], "tarefas"); //var global
  double maxD = parse_double_or_exit(argv[8], "maxD");
  file_name = argv[9];
  periodoS = parse_integer_or_exit(argv[10],"PeriodoS");

  /* Verificacao argumentos*/
  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f max_iter=%d tarefas=%d maxD=%.1f fichS=%s periodoS=%d\n",
	N, tEsq, tSup, tDir, tInf, max_iter,tar,maxD,file_name,periodoS);

  if(N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || max_iter < 1 || tar < 1 || maxD<0 || periodoS<0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
	" Lembrar que N >= 1, temperaturas >= 0, max_iter >= 1, tarefas >= 1, maxD >= 0,fichS a String, periodoS >= 0\n\n");
    return 1;
  }  
  ///////////////////////
  //Ur barrier for simul
  Barrier* bar = new_barrier(tar);

  // Load matrix from file if exists
  FILE *f = fopen(file_name,"r");
  if(f!=NULL){
    //try to read
    matrix = readMatrix2dFromFile(f,N+2,N+2);
    //close stream
    fclose(f);
  }
  if (matrix == NULL) {
    //No file found or corrupted, create matrix from scratch
    matrix = dm2dNew(N+2, N+2);
    if (matrix == NULL) {
      fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para a matrix.\n\n");
      return -1;
    }

    dm2dSetLineTo (matrix, 0, tSup);
    dm2dSetLineTo (matrix, N+1, tInf);
    dm2dSetColumnTo (matrix, 0, tEsq);
    dm2dSetColumnTo (matrix, N+1, tDir);
  }

  matrix_aux = dm2dNew(N+2, N+2);

  if (matrix_aux == NULL) {
    fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para a matrix aux.\n\n");
    return -1;
  }
  //copy both
  dm2dCopy (matrix_aux, matrix);

  int i;
  int res;//used for error handle

  /* Reservar Memória para Trabalhadoras */
  SimulArg* simul_args = (SimulArg *)malloc(tar * sizeof(SimulArg));
  pthread_t* tid = (pthread_t *)malloc(tar * sizeof(pthread_t));

  if (simul_args == NULL || tid == NULL) {
    fprintf(stderr, "\nErro ao alocar memória para trabalhadoras.\n");    
    return -1;
  }

  /* Criar Trabalhadoras */
  for (i = 0; i < tar; i++) {
    simul_args[i].id = i;
    simul_args[i].N = N;
    simul_args[i].maxD=maxD;
    simul_args[i].bar=bar;
    simul_args[i].max_iter=max_iter;
    simul_args[i].tam_fatia = N/tar;
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
  dm2dPrint(matrix);

  /* Remover ficheiro de calculo */
  if(periodoS!=0&&remove(file_name)!=0){
    fprintf(stderr, "\nErro apagar ficheiro.\n"); 
  }
  /* Libertar Memória */
  destroy_barrier(bar);
  dm2dFree(matrix);
  dm2dFree(matrix_aux);
  free(simul_args);
  free(tid);

  return 0;
}
