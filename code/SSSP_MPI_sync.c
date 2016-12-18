 /* 
   Sequential Mandelbrot set
 */
// mpirun -n 2 ./SSSP_MPI_sync 1 In_5_7 o_5_1 1

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
	double tt1, tt2;
	tt1 = MPI_Wtime();
	
	
	
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
	
	
	
	//sp per vertex
	int dist, newdist, flag, flags, linkc, newpar, parent;
	int *dj;
	dist = 1e9;
	linkc = 0;
	dj = (int*)malloc(vert_num * sizeof(int));
	bool have_ball, ball, clean_ball, self_clean;
	//int have_ball, ball, clean_ball, self_clean;
	have_ball = false;
	ball = false;
	clean_ball = false;
	self_clean = white;
	
	//first: source send dist to neighber
	if(rank == source_id){
		dist = 0;
		parent = source_id;
		have_ball = true;
		for(i = 0; i < vert_num; i++){
			if(myweight[i] != 0){
				linkc++;
				dj[i] = dist + myweight[i];
				MPI_Send( &dj[i], 1, MPI_INT, i, data_tag, MPI_COMM_WORLD);
			}
		}
	}
	else{
		for(i = 0; i < vert_num; i++)
			if(myweight[i] != 0) {
				linkc++;
				parent = i;
			}
	}
	
	//dpr
	MPI_Status status;
	MPI_Request *request, req_dpr;
	request = (MPI_Request *)malloc(linkc * sizeof(MPI_Request));
	int mpi_r_flag;
	
	int * new_dist;
	new_dist = (int*)calloc( linkc, sizeof(int));
	
	//MPI_Barrier(MPI_COMM_WORLD);
	//printf("[%d]start sp loop\n", rank);
	do{
		//receive
		for( i = 0; i < linkc; i++){
			MPI_Irecv(&new_dist[i], 1, MPI_INT, MPI_ANY_SOURCE, 
				data_tag, MPI_COMM_WORLD, &request[i]);
		}
		
		/*
		//dual-pass ring algorithm
		{
		if(have_ball){
			if(rank == size - 1){
				//send to root
				MPI_Send(&ball, 1, MPI_C_BOOL, 0, dpr_tag, MPI_COMM_WORLD);
			}
			else{
				//send to next
				MPI_Send(&ball, 1, MPI_C_BOOL, rank + 1, dpr_tag, MPI_COMM_WORLD);
			}
			have_ball = false;
		}
		
		MPI_Start(&req_dpr);
		MPI_Irecv(&ball, 1, MPI_C_BOOL, MPI_ANY_SOURCE, 
				dpr_tag, MPI_COMM_WORLD, &req_dpr);
		if(req_dpr != MPI_REQUEST_NULL){
			have_ball = true;
			//judge the ball
			if(rank == 0 && !ball){
					flag = 1;
				for(i = 1; i < size; i++)
					MPI_Send(&flag, 1, MPI_INT, i, ter_tag, MPI_COMM_WORLD);
				
			}
			else{
				ball = ball | self_clean;
				self_clean = white;
			}
		}
		if(rank != 0 ){
			MPI_Irecv(&flag, 1, MPI_INT, MPI_ANY_SOURCE, 
					ter_tag, MPI_COMM_WORLD, &req_dpr);
		}
		MPI_Request_free( &req_dpr );
		}
		
		*/
		
		//MPI_Barrier(MPI_COMM_WORLD);
		mpi_wait((int) (16000 / size + 1500));
		//MPI_Waitany( linkc, request, &flag, &status);
		//deal receive to decise new dist
		newdist = 1e9;
		flag = 0;
		for( i = 0; i < linkc; i++){
			
			MPI_Request_get_status( request[i], &mpi_r_flag, &status);
			if(mpi_r_flag && new_dist[i] < newdist){
				newdist = new_dist[i];
				newpar = status.MPI_SOURCE;
				printf("[%d]renew newdist: %d, newpar: %d\n"
					, rank, newdist, newpar);
			}
		}
		if (newdist < dist){
			dist = newdist; /* start searching around vertex */
			parent = newpar;
			printf("[%d]new dist: %d, parent: %d\n", rank, dist, parent);
			flag = 1;
			for (j = 0; j < vert_num; j++) /* get next edge */
				if (myweight[j] != 0) {
					dj[j] = dist + myweight[j];
					/*
					if(j < rank)
						self_clean = black;
					*/
					/* send distance to proc j */
					MPI_Send(&dj[j], 1, MPI_INT, j, data_tag, MPI_COMM_WORLD);
				}
		}
		
		//Reduce flag
		MPI_Allreduce(&flag, &flags, 1,
               MPI_INT, MPI_MAX, MPI_COMM_WORLD);
		flag = flags;
	}while(flag != 0);
	free(request);
	free((void *)myweight);
	free((void *)new_dist);
	// Synchronize sender & receiver
	//MPI_Wait( &request, &status);
	
	//write
	
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
	
	
	
	MPI_Finalize();
	return 0;
}
