#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "queue.h"

const int MAX_MSG = 10000;

void Usage(char *prog_name) {
   fprintf(stderr, "uso: %s <numero de hilos> <numero de mensajes>\n",
         prog_name);
   fprintf(stderr, "numero de mensajes = numero enviado por cada hilo\n");
   exit(0);
}  


void Send_msg(struct queue_s* msg_queues[], int my_rank, 
   int thread_count, int msg_number) {
   int mesg = -msg_number;
   int dest = random() % thread_count;

#  pragma omp critical
   
   Enqueue(msg_queues[dest], my_rank, mesg);
#  ifdef DEBUG
   printf("Hilo %d > enviado %d a %d\n", my_rank, mesg, dest);
#  endif
}  

void Try_receive(struct queue_s* q_p, int my_rank) {
   int src, mesg;
   int queue_size = q_p->enqueued - q_p->dequeued;

   if (queue_size == 0) return;
   else if (queue_size == 1)
#     pragma omp critical
      Dequeue(q_p, &src, &mesg);  
   else
      Dequeue(q_p, &src, &mesg);
   printf("Hilo %d > recibido %d desde %d\n", my_rank, mesg, src);
}   


int Done(struct queue_s* q_p, int done_sending, int thread_count) {
   int queue_size = q_p->enqueued - q_p->dequeued;
   if (queue_size == 0 && done_sending == thread_count)
      return 1;
   else 
      return 0;
}   


int main(int argc, char* argv[]) {
   int thread_count;
   int send_max;
   struct queue_s** msg_queues;
   int done_sending = 0;

   if (argc != 3) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);
   send_max = strtol(argv[2], NULL, 10);
   if (thread_count <= 0 || send_max < 0) Usage(argv[0]);

   msg_queues = malloc(thread_count*sizeof(struct queue_node_s*));

#  pragma omp parallel num_threads(thread_count) \
      default(none) shared(thread_count, send_max, msg_queues, done_sending)
   {
      int my_rank = omp_get_thread_num();
      int msg_number;
      srandom(my_rank);
      msg_queues[my_rank] = Allocate_queue();

#     pragma omp barrier 
                         

      for (msg_number = 0; msg_number < send_max; msg_number++) {
         Send_msg(msg_queues, my_rank, thread_count, msg_number);
         Try_receive(msg_queues[my_rank], my_rank);
      }

#     pragma omp atomic

      done_sending++;
#     ifdef DEBUG
      printf("Hilo %d > envio realizado\n", my_rank);
#     endif

      while (!Done(msg_queues[my_rank], done_sending, thread_count))
         Try_receive(msg_queues[my_rank], my_rank);


      Free_queue(msg_queues[my_rank]);
      free(msg_queues[my_rank]);
   }  

   free(msg_queues);
   return 0;
}  


