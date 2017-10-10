
/*--------------------------------------------------------------------
| Disclamer: Este código foi feito um pouco à pressa. Não 
|            está perfeito mas ilustra a utilização da mplib
| v2: alguns warnings corrigidos.
---------------------------------------------------------------------*/

/*--------------------------------------------------------------------
| Includes
---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "mplib3.h"

/*--------------------------------------------------------------------
| Defines
---------------------------------------------------------------------*/

#define STRSZ 10
#define BUFFSZ 256

typedef struct {
  int id;
  int n;
} argsSimular_t;

/*--------------------------------------------------------------------
| Function: strupr
| Description: converte uma string para uppercase
---------------------------------------------------------------------*/

char *strupr (char *str){
  int i, n;

  n = strlen (str);
  for (i=0; i<n; i++)
    str[i]= toupper(str[i]);
  return str;
}

/*--------------------------------------------------------------------
| Function: slaveThread
| Description: envia numa mensagem para a thread 0 e espera uma resposta
|              faz isto n vezes
---------------------------------------------------------------------*/

void *slaveThread(void *a) {
   char send_buff[BUFFSZ];
   char receive_buff[BUFFSZ];
   argsSimular_t *arg   = (argsSimular_t *) a;
   int           myid = arg->id;
   int i,j;
   
   for (i=0; i<STRSZ; i++)
     send_buff[i]=96+myid;
   send_buff[10]=0;
   
   for (j=0; j<arg->n; j++) {
     printf ("task=%d vai enviar %s para task 0\n", myid, send_buff);
     enviarMensagem (myid, 0, send_buff, strlen(send_buff)+1);
     receberMensagem (0, myid, receive_buff, BUFFSZ);
     printf ("task=%d recebeu %s da task 0\n", myid, receive_buff);
   }
   return 0;
} 

/*--------------------------------------------------------------------
| Function: main
| Description: cria n threads
|              para cada um dos filhos recebe uma mensagem e envia 
               uma resposta n vezes
---------------------------------------------------------------------*/


int main (int argc, char** argv) {
  int numTarefas;
  int i, j;
  char buff[BUFFSZ];

  argsSimular_t *slave_args;
  pthread_t *slaves;

  if (argc < 2) {
    printf("client_server_1 n_tarefas\n");
    return 1;
  }

  numTarefas = atoi(argv[1]);

  if ((numTarefas<1) || (numTarefas>26)){
    printf ("n_tarefas deve ser entre 1 e 26\n");
    return 1;
  }
  
  slave_args = (argsSimular_t*)malloc(numTarefas*sizeof(argsSimular_t));
  slaves     = (pthread_t*)malloc(numTarefas*sizeof(pthread_t));

  if (inicializarMPlib(1,numTarefas+1) == -1) {
    printf("Erro ao inicializar MPLib.\n"); return 1;
  }


  /* create slaves */
  for (i=0; i<numTarefas; i++) {
    slave_args[i].id = i+1;
    slave_args[i].n  = numTarefas;

    pthread_create(&slaves[i], NULL, slaveThread, &slave_args[i]);
  }
  
  for (i=0; i<numTarefas; i++) {
    for (j=0; j<numTarefas; j++) {
      receberMensagem (i+1, 0, buff, BUFFSZ);
      strupr (buff);
      enviarMensagem (0, i+1, buff, strlen(buff)+1);
    }
  }
  
  /* Esperar que os Escravos Terminem */
  for (i = 0; i < numTarefas; i++) {
    if (pthread_join(slaves[i], NULL)) {
      fprintf(stderr, "\nErro ao esperar por um escravo.\n");    
      return -1;
    }  
  }

  return 0;
}
