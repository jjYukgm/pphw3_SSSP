 /* 
   Sequential Mandelbrot set
 */
// ./SSSP_Pthread 1 In_5_7 o_5_1 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

//SP-alg: que
#include <stdbool.h>
#include <math.h>// ceil

//enable & disable
#include <string.h>
#include <time.h>//time measure
#include <math.h>//time calculate



	
typedef struct EdgeInfo
{
	int a, b, dis;//, da, db, num;
	
	union pthread_mutex_t * mutex; 
} EIn;

typedef struct ptReturn
{
	int b, db, parent;
} PRet;

typedef struct Qnode
{
	int id;
	struct Qnode* next;
	//next = (Dqn *)malloc(sizeof(Dqn));
	
} Dqn;

Dqn *last;
bool *inque;
int *d, *parent;
int vert_num;
//union 
pthread_mutex_t mutex; 

Dqn* newNode(int id, Dqn* ends) {
  Dqn *new;      
  new = (Dqn *)malloc(sizeof(Dqn));
  new->id = id;    
  new->next = ends;   
  return new;
}

void *b_part(void *edge2) {
	//long* data = static_cast <long*> threadId;
	//printf("Hello World! It's me, thread #%ld!\n", *data);
	EIn edges ;
	int newdist_j;
	//PRet *bpreturn;
	Dqn *node;
	struct timespec sy11, sy12;
	double sy1 = 0.0;
	//bpreturn = (PRet* ) malloc(sizeof(PRet));
	//bpreturn->b = -1;
	//bpreturn->db = -1;
	//bpreturn->parent = -1;
	//memcpy(&data,(EIn*)edge,sizeof(EIn));
	edges =  *(EIn*)edge2;
	if (edges.b < vert_num && edges.b != edges.a && edges.dis != 0 ) { /* if an edge */
		newdist_j = d[edges.a] + edges.dis;
		
		clock_gettime(CLOCK_REALTIME, &sy11);
		//printf("[%d]newdist :%d\n", edges.b,  newdist_j);
		pthread_mutex_lock(&mutex); 
		// enter critical section 
		clock_gettime(CLOCK_REALTIME, &sy12);
		if (newdist_j < d[edges.b]){
			d[edges.b] = newdist_j;
			parent[edges.b] = edges.a;
			if (!inque[edges.b]) {
				//printf("enque :%d\n", edges.b );
				node = newNode(edges.b, last->next);
				last->next = node;
				last = node;
				inque[edges.b] = true;
				
				
				//bpreturn->b = edges.b;
				//bpreturn->db = newdist_j;
				//bpreturn->parent = edges.a;
			}
		}
		pthread_mutex_unlock(&mutex); // leave critical section
		sy1 += sy12.tv_sec - sy11.tv_sec;
		sy1 += (sy12.tv_nsec - sy11.tv_nsec) / 1000000000.0;
	}
	
	pthread_exit(&sy1);
}

int main(int argc, char *argv[])
{
	//time measure
	struct timespec io11, io12, cp11, cp12;
	double *sytmp;
	double io1 = 0.0;
	double cp1 = 0.0;
	double sy1 = 0.0;
	
	clock_gettime(CLOCK_REALTIME, &io11);
	
	int thread_num = atoi(argv[1]);
	int source_id = atoi(argv[4]) - 1;//when read, id - 1
	FILE *fpin, *fpout;
	//int vert_num = 0;
	int edge_num = 0;
	int i, j, k;
	pthread_t threads[thread_num];
	//union pthread_mutex_t * mutex; 
	//mutex = (pthread_mutex_t *)malloc( sizeof(pthread_mutex_t) );
	pthread_mutex_init (&mutex, NULL);
	
	//read the input
	fpin = fopen(argv[2], "r");
	if (fpin == NULL)
        exit(EXIT_FAILURE);
	//read num of vertices, edges
	fscanf(fpin,"%d",&vert_num);
	fscanf(fpin,"%d",&edge_num);
	
	//dijkstra matrix
	int * weight;
	//memset( weight, 0, vert_num*vert_num*sizeof(int) );
	weight = (int *)calloc( vert_num * vert_num, sizeof(int));
	//to pthread
	EIn edges;
	
	
	//edges = (EIn *)malloc(sizeof(int) * 3 * edge_num);
	
	//printf("read time\n");
	//read edges
	for(i = 0; i < edge_num; i++){
		fscanf(fpin,"%d",&edges.a);
		fscanf(fpin,"%d",&edges.b);
		fscanf(fpin,"%d",&edges.dis);
		
		
		//dijkstra matrix
		weight[(edges.a - 1)* vert_num + edges.b - 1] = edges.dis;//when read, id - 1
		weight[(edges.b - 1)* vert_num + edges.a - 1] = edges.dis;//when read, id - 1
		
		/*
		etmp1.b = edges.b - 1;//when read, id - 1
		etmp1.w = edges.dis;  //when read, id - 1
		link_list[edges.a - 1].push_back(etmp1);//when read, id - 1
		size[edges.a - 1] ++;              //when read, id - 1
		etmp1.b = edges.a - 1;//when read, id - 1
		link_list[edges.b - 1].push_back(etmp1);//when read, id - 1
		size[edges.b - 1] ++;              //when read, id - 1
		*/
	}
	fclose(fpin);
	clock_gettime(CLOCK_REALTIME, &io12);
	io1 += io12.tv_sec - io11.tv_sec;
	io1 += (io12.tv_nsec - io11.tv_nsec) / 1000000000.0;
	
	//shortest path
	
	//printf("sp time\n");
	clock_gettime(CLOCK_REALTIME, &cp11);
	//queue
	//Dqn *node, *head, end;
	Dqn *head, end;
	//PRet *ret;
	//int newdist_j;
	//ret = (PRet *)malloc(sizeof(PRet) * 1);
	
	inque = (bool *)malloc(sizeof(bool) * vert_num);
	for ( i = 0; i < vert_num; i++) inque[i] = false;
	d = (int *)malloc(sizeof(int) * vert_num);
	parent = (int *)malloc(sizeof(int) * vert_num);
	for ( i = 0; i < vert_num; i++) d[i] = 1e9;//1e9 INT_MAX
	d[source_id] = 0;
	parent[source_id] = source_id;
	
	end.id = -1;
	end.next = &end;
	int iter;
	iter = (int) ceil((double)vert_num / thread_num);
	
	
	//que.push
	head = newNode(source_id, &end);
	last = head;
	//printf("head->id :%d\n, source_id:%d\n", head->id, source_id);

	EIn *edgess;//[thread_num];
	edgess = (EIn *)malloc( thread_num * sizeof(EIn) );
	
	//printf("sp while\n");
	
	inque[source_id] = true;
	//edges.num = vert_num;
	//edges.mutex = mutex;
	do{ /* while a vertex */
		//edges.a = head->id;
		//edges.da = d[edges.a];
		//printf("now :%d\n", head->id);
		for(i = 0; i < iter; i++){
			for (j = 0; j < thread_num; j++) { /* get next edge */
				edgess[j].a = head->id;
				edgess[j].b = j + i * thread_num;
				//if(edges.b >= vert_num)
				//	break;
				edgess[j].dis = weight[edgess[j].a* vert_num + edgess[j].b];
				//if(edges.dis == 0)
				//	continue;
				//printf("do edges :%d\n", edgess[j].b );
				//edges.db = d[edges.b];
				pthread_create(&threads[j], NULL, b_part, (void *)&edgess[j]);
			}
			for (j = 0; j < thread_num; j++) {
				pthread_join(threads[j], (void **) &sytmp);
				sy1 += *sytmp;
				//printf("join :%d\n", j);
				
			}
			//que.push(j); /* enqueue vertex if not there */
			/*
			for (j = 0; j < thread_num; j++) {
				pthread_join (threads[j],(void **) &ret);
				if(ret->b!=-1 && !inque[ret->b]){
					printf("enque :%d\n", ret->b);
					node = newNode(ret[j].b, &end);
					last->next = node;
					last = node;
					inque[ret[j].b] = true;
				}
			}
			*/
		}
		//printf("que.pop\n");
		//que.pop
		inque[head->id] = false;
		head = head->next;
	}while (head->id != -1);
	
	free((void *)weight);
	free((void *)d);
	free((void *)edgess);
	clock_gettime(CLOCK_REALTIME, &cp12);
	cp1 += io12.tv_sec - io11.tv_sec;
	cp1 += (io12.tv_nsec - io11.tv_nsec) / 1000000000.0;
	
	printf("write time\n");
	//write
	clock_gettime(CLOCK_REALTIME, &io11);
	int *sp;
	//memset( sp, 0, vert_num*sizeof(int) );
	sp = (int *)malloc(sizeof(int) * vert_num);
	fpout = fopen(argv[3], "w");
	for(i = 0; i < vert_num; i++){
		j = i;
		k = -1;
		//printf("write i = %d\n", i);
		do{
			
			//printf("	write j = %d\n", j);
			k++;
			sp[k] = j;
			j = parent[j];
		}while(j != source_id);
		fprintf(fpout, "%d", source_id + 1);//when write, id + 1
		for(;k >= 0; k--){
			fprintf(fpout, " %d", sp[k] + 1);//when write, id + 1
		}
		fprintf(fpout, "\n");
	}
	fclose(fpout);
	clock_gettime(CLOCK_REALTIME, &io12);
	io1 += io12.tv_sec - io11.tv_sec;
	io1 += (io12.tv_nsec - io11.tv_nsec) / 1000000000.0;
	
	printf("io1 time: %3f\n cp1 time: %3f\n sy1 time: %3f\n", io1, cp1, sy1);
	
	//free((void *)edges);
	free((void *)parent);
	free((void *)sp);
	//pthread_mutex_destory(&mutex);
	pthread_exit(NULL);
	return 0;
}

