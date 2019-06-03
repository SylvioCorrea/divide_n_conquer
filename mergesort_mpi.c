/*
Modelos:
A: divisao e conquista pura. No máximo um processo por core
    (ou 16 por nodo com hyperthread).
B: inflar propositalmente o número de processos para mais
    de um por core. O SO vai fazer o balanceamento de carga
    ao deixar os processos menos trabalhosos em espera
C: modelo do artigo onde o pai divide o trabalho com ele
    mesmo mais um filho
*/

/*
IMPORTANT: expected number of process to run this
implementation: 2^(TREE_HEIGHT) - 1
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

//Number of 
#define DELTA 30
#define TREE_HEIGHT 4
//#define ARR_SIZE DELTA * 2^(TREE_HEIGHT)
#define ARR_SIZE 30


void make_arr(int arr[], int size) {
    int i;
    for(i=0; i<size; i++) {
        arr[i] = size-i;
    }
}

void bubblesort(int arr[], int size) {
    int i, temp;
    int ordered = 0;
    while(!ordered) {
        ordered = 1;
        for(i=0; i<size-1; i++) {
            if(arr[i]>arr[i+1]) {
                temp = arr[i];
                arr[i] = arr[i+1];
                arr[i+1] = temp;
                ordered = 0;
            }
        }
    }
}

void merge(int arr1[], int arr2[], int size, int res[]) {
    int size1 = size/2;
    int size2 = size-size1;
    int i = 0;
    int j = 0;
    int k = 0;
    while(j<size1 && k<size2) {
        printf("merge begin\n");
        if(arr1[j]<arr2[k]) {
            printf("in\n");
            res[i] = arr1[j];
            j++;
        } else {
            printf("in2\n");
            res[i] = arr2[k];
            k++;
        }
        i++;
    }
    
    if(j<size1) {
        printf("in3\n");
        for(j; j<size1; j++) {
            res[i] = arr1[j];
            i++;
        }
    } else {
        printf("in4\n");
        for(k; k<size2; k++) {
            res[i] = arr2[k];
            i++;
        }
    }   
}

int calc_father(int child) {
    if(child%2 == 0) {
        return (child-2)/2;
    }
    return (child-1)/2;
}

void main(int argc, char** argv) {
    
    int arr[ARR_SIZE];
    
    int i;
	int my_rank;       // Identificador deste processo
	int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)  
	int message;       // Buffer para as mensagens
	int delta = 1000;
	int curr_size;
	int mid;
	int father, lchild, rchild;
	MPI_Status status; // estrutura que guarda o estado de retorno          
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega pega o numero do processo atual (rank)
    
    printf("size of the array: %d\n", ARR_SIZE);
    printf("conquering point: %d\n", DELTA);
    printf("number of processes: %d\n\n", proc_n);
    
    // recebo vetor
    if ( my_rank != 0 ) {
        father = calc_father(my_rank);
        MPI_Recv(arr, ARR_SIZE, MPI_INT, father, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("[%d] recebeu.\n", my_rank);
        MPI_Get_count(&status, MPI_INT, &curr_size);  // descubro tamanho da mensagem recebida
        lchild = status.MPI_SOURCE * 2 + 1;
        rchild = status.MPI_SOURCE * 2 + 2;
    }
    else {
        make_arr(arr, ARR_SIZE);      // sou a raiz e portanto gero o vetor - ordem reversa
        curr_size = ARR_SIZE;
        lchild = 1;
        rchild = 2;
    }
    
    //Important: this will hold the results to be sent up the tree.
    int *res;
    
    // dividir ou conquistar?
    if ( curr_size <= delta ) {
        bubblesort(arr, curr_size);  // conquisto
        res = arr; //pointers to the same memory location.
    }
    else {
        // dividir
        lchild = status.MPI_SOURCE * 2 + 1;
        rchild = status.MPI_SOURCE * 2 + 2;
        mid = curr_size/2;
        //Manda as duas metades do vetor.
        MPI_Send (arr, mid, MPI_INT, lchild, 1, MPI_COMM_WORLD);
        MPI_Send (&arr[mid], curr_size-mid, MPI_INT, rchild,
                  1, MPI_COMM_WORLD);

        // receber dos filhos
        MPI_Recv (arr, mid, MPI_INT, lchild, MPI_ANY_TAG,
                  MPI_COMM_WORLD, &status);
        printf("[%d] recebeu devolucao do lchild.\n", my_rank);           
        MPI_Recv (&arr[mid], curr_size-mid, MPI_INT, rchild,
                  MPI_ANY_TAG, MPI_COMM_WORLD, &status);   
        printf("[%d] recebeu devolucao do rchild.\n", my_rank);
        
        res = malloc(curr_size * sizeof(int));
        if(!res){
            printf("malloc failed!\n");
            exit(1);
        }
        
        merge(arr, &arr[mid], curr_size, res);
    }

    // mando para o pai
    if ( my_rank !=0 ) {
        MPI_Send(res, curr_size, MPI_INT, status.MPI_SOURCE,
                 1, MPI_COMM_WORLD);  // tenho pai, retorno vetor ordenado pra ele
        
    }
    else {
        printf("Root results:\n");
        for(i=0; i<ARR_SIZE; i++) {
            printf("%d, ", res[i]);
        }
    }
    
    free(res);
    
    MPI_Finalize();
}
