 /* 
   Sequential Mandelbrot set
 */
// mpirun -n 5 ./SSSP_MPI_sync 1 In_5_7 o_5_1 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

//SP-alg: 
#include <stdbool.h>
#include <math.h>// ceil

//file write
#include <string.h>

//#include <time.h>//time measure
//#include <math.h>//time calculate
//comm tag
#define data_tag 0
#define dpr_tag 1
#define ter_tag 2
//dpr
#define white false
#define black true



typedef struct EdgeInfo
{
	int a, b, dis;
} EIn;
void mpi_wait(int msec){
	double tt1, tt2;
	tt1 = MPI_Wtime();
	while(true){
		tt2 = MPI_Wtime();
		if(tt2 - tt1 >= msec / 1000)
			return;
	}
	
}

int main(int argc, char *argv[])
{
	
	/*
	struct timespec tt1, tt2;
	clock_gettime(CLOCK_REALTIME, &tt1);*/

	//int thread_num = atoi(argv[1]);
	int source_id = atoi(argv[4]) - 1;//when read, id - 1

	//printf("start MPI\n");
	int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//rank = ver_id - 1
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//time measure
	double io11, io12, cp11, cp12;
	double cm11, cm12, sy11, sy12;
	double io1 = 0.0;
	double cp1 = 0.0;
	double cm1 = 0.0;
	double sy1 = 0.0;
	io11 = MPI_Wtime();
	
	
	
	//printf("[%d]start read\n", rank);
    int error;
    MPI_File fh;
	char *input;
	MPI_Offset fsize;
	
	error = MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY ,
				MPI_INFO_NULL, &fh);
	if (error != MPI_SUCCESS)
        printf("[%d]ERROR: MPI_File_open\n", rank);
	MPI_File_get_size( fh, &fsize);
	//printf("[%d]inp fsize: %d\n",rank, fsize);
	input = (char *)malloc( fsize * sizeof(char) );
	fsize--;/* get rid of text file eof */
	error = MPI_File_read( fh, input, fsize, MPI_CHAR, MPI_STATUS_IGNORE);
	if (error != MPI_SUCCESS)
        printf("[%d]ERROR: MPI_File_read\n", rank);
    MPI_File_close(&fh);
	input[fsize] = '\0';
	//printf("[%d]input: %s\n", rank, input);
	
	
	//read the input
	int vert_num = 0;
	int edge_num = 0;
	int i, j, k;
	
	//read num of vertices, edges
	sscanf(input,"%d %d",&vert_num,&edge_num);
	
	//dijkstra matrix
	int * weight;
	weight = (int *)calloc( vert_num * vert_num, sizeof(int));
	//to pthread
	EIn edges;
	char *inp_split = strtok(input, "\n");
		 
	//read edges
	inp_split = strtok(NULL, "\n");
    while (inp_split != NULL) {
		sscanf(inp_split,"%d %d %d", &edges.a, &edges.b, &edges.dis);
		weight[(edges.a - 1)* vert_num + edges.b - 1] = edges.dis;//when read, id - 1
		weight[(edges.b - 1)* vert_num + edges.a - 1] = edges.dis;//when read, id - 1
		
        inp_split = strtok(NULL, "\n");
    }
	
	//leave my weight
	int * myweight;
	myweight = (int *)malloc( vert_num * sizeof(int));
	for(i = 0; i < vert_num; i++){
		myweight[i] = weight[rank * vert_num + i];
		//printf("[%d]myw[%d]: %d ", rank, i, myweight[i]);
	}
	free((void *)weight);
	
	io12 = MPI_Wtime();
	io1 += io12 - io11;
	
	cp11 = MPI_Wtime();
	
	
	
	//sp per vertex
	int dist, newdist, flag, flags, linkc, newpar, parent, count;
	int *dj;
	dist = 1e9;
	linkc = 0;
	parent = -1;
	count = 0;
	dj = (int*)malloc(vert_num * sizeof(int));
	
	//first: source send dist to neighber
	if(rank == source_id){
		dist = 0;
		parent = source_id;
	}
	
	for(i = 0; i < vert_num; i++)
		if(myweight[i] != 0) {
			linkc++;
			//parent = i;
		}
	
	//dpr
	MPI_Status *status;
	MPI_Request *request, req_dpr;
	status = (MPI_Status *)malloc(linkc * sizeof(MPI_Status));
	request = (MPI_Request *)malloc(linkc * sizeof(MPI_Request));
	int mpi_r_flag;
	
	int * new_dist;
	new_dist = (int*)calloc( linkc, sizeof(int));
	
	//loop no mae, to fill the recv buffer, to provide the dropping
	for (j = 0; j < vert_num; j++) /* get next edge */
		if (myweight[j] != 0) {
			dj[j] = dist + myweight[j];
			/* send distance to proc j */
			cm11 = MPI_Wtime();
			MPI_Send(&dj[j], 1, MPI_INT, j, data_tag, MPI_COMM_WORLD);
			cm12 = MPI_Wtime();
			cm1 +=  cm12 - cm11;
		}
	//MPI_Barrier(MPI_COMM_WORLD);
	//printf("[%d]start sp loop\n", rank);
	do{
		cm11 = MPI_Wtime();
		//receive
		for( i = 0; i < linkc; i++){
			MPI_Irecv(&new_dist[i], 1, MPI_INT, MPI_ANY_SOURCE, 
				data_tag, MPI_COMM_WORLD, &request[i]);
		}
		MPI_Waitall(linkc, request, status);
		cm12 = MPI_Wtime();
		cm1 +=  cm12 - cm11;
		count++;
		
		newdist = 1e9;
		flag = 0;
		
		//deal receive to decise new dist
		for( i = 0; i < linkc; i++){
			
			//MPI_Request_get_status( request[i], &mpi_r_flag, &status);
			if( new_dist[i] < newdist){
				newdist = new_dist[i];
				newpar = status[i].MPI_SOURCE;
				//printf("[%d]renew newdist: %d, newpar: %d\n"
				//	, rank, newdist, newpar);
			}
		}
		if (newdist < dist){
			dist = newdist; /* start searching around vertex */
			parent = newpar;
			printf("[%d]new dist: %d, parent: %d\n", rank, dist, parent);
			flag = 1;
		}
		
		for (j = 0; j < vert_num; j++) /* get next edge */
			if (myweight[j] != 0) {
				dj[j] = dist + myweight[j];
				/* send distance to proc j */
				cm11 = MPI_Wtime();
				MPI_Send(&dj[j], 1, MPI_INT, j, data_tag, MPI_COMM_WORLD);
				cm12 = MPI_Wtime();
				cm1 +=  cm12 - cm11;
			}
		
		sy11 = MPI_Wtime();
		//MPI_Barrier(MPI_COMM_WORLD);
		
		
		//Reduce flag
		//cm11 = MPI_Wtime();
		MPI_Allreduce(&flag, &flags, 1,
               MPI_INT, MPI_MAX, MPI_COMM_WORLD);
		sy12 = MPI_Wtime();
		sy1 +=  sy12 - sy11;
		//cm12 = MPI_Wtime();
		//cm1 +=  cm12 - cm11;
		flag = flags;
	}while(flag != 0);
	free(request);
	free((void *)myweight);
	free((void *)new_dist);
	cp12 = MPI_Wtime();
	cp1 = cp12 - cp11;
	
	//write
	
	io11 = MPI_Wtime();
	//printf("[%d]write time\n", rank);
	//gather parents
	int *parents;
	parents = (int* ) malloc(vert_num * sizeof(int));
	MPI_Allgather(&parent, 1, MPI_INT,
                  parents, 1, MPI_INT,
                  MPI_COMM_WORLD);
				  
	if(rank == 0) {
		for(i=0;i < vert_num; i++) printf("p[%d] %d, ", i, parents[i]);
		printf("\n");
	}
	int *sp;
	sp = (int *)malloc(sizeof(int) * vert_num);
	char *output; 
	char str_tmp[33]; 
	output = (char* ) malloc(3 * (vert_num + 1) * sizeof(char));
	//str_tmp = (char* ) malloc(33 * sizeof(char));
	j = rank;
	k = -1;
	//printf("write i = %d\n", i);
	do{
		
		//printf("	write j = %d\n", j);
		k++;
		sp[k] = j;
		j = parents[j];
	}while(j != source_id);
	sprintf(output, "%d", source_id + 1);//when write, id + 1
	for(;k >= 0; k--){
		sprintf(str_tmp, " %d", sp[k] + 1);//when write, id + 1
		//printf("[%d]sp[%d] : %d\n", rank, k, sp[k]);
		strcat(output, str_tmp);
	}
	strcat(output, "\n");
	
    for(i = 0; output[i] != '\0'; ++i);
	printf("[%d]output: %s\n", rank, output);
	
	error = MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
	if (error != MPI_SUCCESS)
        printf("[%d]ERROR: MPI_File_open\n", rank);
	error = MPI_File_write_ordered( fh, output, i, MPI_CHAR, MPI_STATUS_IGNORE);
	if (error != MPI_SUCCESS)
        printf("[%d]ERROR: MPI_File_write_all\n", rank);
    MPI_File_close(&fh);
	io12 = MPI_Wtime();
	io1 += io12 - io11;
	
	printf("[%d]io1: %3f, cp1: %3f, cm1: %3f, sy1: %3f, linkc* count: %d\n"
		, rank, io1, cp1, cm1, sy1, linkc* count);
	
	
	MPI_Finalize();
	return 0;
}
