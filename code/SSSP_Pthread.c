 /* 
   Sequential Mandelbrot set
 */
// ./SSSP_Pthread 1 In_5_7 o_5_1 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

//SP-alg: que
#include <sys/queue.h>
//#include "queue.h"
//SP-alg
//#include <vector>//C++
#include <stdbool.h>
#include <limits.h>// int max

//enable & disable
#include <string.h>
#include <time.h>//time measure
#include <math.h>//time calculate


int *d;
int *parent;

typedef struct EdgeInfo
{
	int a, b, dis, da;
	bool vb;
} EIn;



void *b_part(void *edge) {
	//long* data = static_cast <long*> threadId;
	//printf("Hello World! It's me, thread #%ld!\n", *data);
	EIn data ;
	//memcpy(&data,(EIn*)edge,sizeof(EIn));
	data =  *(EIn*)edge;
	if (!data.vb && data.dis != 0 && data.da + data.dis < d[data.b])
	{
		d[data.b] = data.da + data.dis;
		parent[data.b] = data.a;
	}
	
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	//time measure
	struct timespec tt1, tt2;
	clock_gettime(CLOCK_REALTIME, &tt1);
	
	int thread_num = atoi(argv[1]);
	int source_id = atoi(argv[4]) - 1;//when read, id - 1
	FILE *fpin, *fpout;
	int vert_num = 0;
	int edge_num = 0;
	int i, j, k;
	pthread_t threads[thread_num];
	
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
	
	EIn edges;
	//edges = (EIn *)malloc(sizeof(int) * 3 * edge_num);
	//read edges
	for(i = 0; i < edge_num; i++){
		fscanf(fpin,"%d",&edges.a);
		fscanf(fpin,"%d",&edges.b);
		fscanf(fpin,"%d",&edges.dis);
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
	
	//shortest path
	
	//printf("sp time\n");
	//queue
	queue qbps;
	int newdist_j;
	qbps.push(source_id);

	while (!qbps.empty){ /* while a vertex */
		i = qbps.pop();
		for (j = 0; j < vert_num; j++) { /* get next edge */
			edges.b = j ;
			edges.dis = weight[a* vert_num + edges.b];
			if (w[i* vert_num +j] != 0) { /* if an edge */
				newdist_j = d[i] + edges.dis;
				if (newdist_j < d[j]) {
					dist[j] = newdist_j;
					qbps.push(j); /* enqueue vertex if not there */
				}
			}
		}
	}
	
	//dijkstra
	/*
	d = (int *)malloc(sizeof(int) * vert_num);
	parent = (int *)malloc(sizeof(int) * vert_num);
	bool visit[vert_num];
	for ( i = 0; i < vert_num; i++) {
		visit[i] = false;
		d[i] = 1e9;//1e9 INT_MAX
	}
 
	d[source_id] = 0;
	parent[source_id] = source_id;
 
	
	//printf("sp for, source_id = %d\n", source_id);
	int a, min, w;
	//int iter = (int) vert_num/thread_num;
	for ( k = 0; k < vert_num; k++)// worst case: #v = # iter
	{
		a = -1;
		min = 1e9;
		//printf("sp a part\n");
		for ( i = 0; i < vert_num; i++){
			//printf("sp  i = %d, d[i] = %d\n", i, d[i]);
			if (!visit[i] && d[i] < min)
			{
				a = i;
				min = d[i];
				//printf("sp cool! a = %d\n",a);
			}
		}
		if (a == -1) break;
		visit[a] = true;
		edges.a = a;
		for ( i = 0; i < vert_num; i++)
		{
			edges.b = i ;
			edges.dis = weight[a* vert_num + edges.b];
			edges.da = d[a];
			edges.vb = visit[edges.b];

			//printf("before pthread_create\n");
			pthread_create(&threads[i%thread_num], NULL, b_part, (void *)&edges);

			//if(i == 0)
			//for (j = 0; j < thread_num; j++) pthread_join(threads[j], NULL);
		}
	}
	*/
	
	
	pthread_exit(NULL);
	free((void *)weight);
	
	printf("write time\n");
	//write
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
	
	
	//free((void *)edges);
	free((void *)d);
	free((void *)parent);
	free((void *)sp);
	return 0;
}

