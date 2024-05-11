/* Author: Cristian Tiriolo
 * Shown during COEN320 tutorials
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include "cTimer.h"

struct thread_args {    /* Used as argument to the start routine thread_start() */
	int** location;      /* Location of the row */
	int n_row;        /* number of raws of m*/
	int n_col;        /* number of columns of m*/
	int period_sec;    //desired period of the thread in seconds
	int period_msec;   //desired period of the thread in milliseconds
};


void *thread_start (void *arg) {
	struct thread_args *targs = ( struct thread_args * ) arg;

	int **loc=targs->location;
	int n_row=targs->n_row;
	int n_col=targs->n_col;

	int period_sec=targs->period_sec;
	int period_msec=targs->period_msec;


	int i_pos=0,j_pos=0; //initializing the indexes of the cursor


	cTimer timer(period_sec,period_msec); //initialize, set, and start the timer

	while(true){

		*(*(loc+i_pos)+j_pos)=1; //setting the cursor element to 1

		int **p; //starting location of the matrix
		for (int i = 0 ; i <n_row ; i++)
		{
			p=loc+i;
			for (int j = 0; j <n_col; j++)
				cout<<"\t"<<*(*p+j); //printing the m[i][j] value of the matrix
			cout<<endl;
		}
		cout<<endl;

		*(*(loc+i_pos)+j_pos)=0; //resetting the cursor element to 0

		j_pos++; //incrementing the index of the cursor
		if (j_pos==n_col){// if j_pos==n_col the cursor goes into the next line
			j_pos=0;
			i_pos++;
		}
		if (i_pos==n_row){//if j_pos==n_col the cursor has reached the last element of the matrix. We can break the cycle
			break;
		}

		timer.waitTimer();
	}//end_while


	return NULL;
}//end_thread_start()
/*
int main (int argc, char* argv[]) {


	//initilize a matrix with all zeros
	int n_row=3, n_col=4;
	int **m;
	m = new int *[n_row];
	for(int i = 0; i <n_row; i++)
		m[i] = new int[n_col];



	//input arguments of the thread_start routine
	struct thread_args targs;
	targs.location=m;
	targs.n_col=n_col;
	targs.n_row=n_row;
	targs.period_sec=0;
	targs.period_msec=500;

	pthread_t thread_id;//ID of the thread
	int err_no;
	err_no = pthread_create(&thread_id, NULL, &thread_start, &targs); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the thread \n");
	}


	err_no = pthread_join(thread_id, NULL); //force the main to wait for the termination of the thread
	if (err_no != 0){
		printf("ERROR when joining the thread\n");
	}

	pthread_exit(EXIT_SUCCESS);
}
*/
