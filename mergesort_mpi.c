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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define DELTA 1000
#define ARR_SIZE DELTA * 2^(8)

void make_arr(int arr[], size) {
    int i;
    for(i=0; i<size; i++) [
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
                oredered = 0;
            }
        }
    }
}

void merge(int arr[], int size) {
    int mid = size/2;
    int i = 0;
    int j = mid;
    int temp;
    while(i<mid) {
        if(arr[i] > arr[j]) {
            temp = arr[i];
            arr[i] = arr[j]
            arr[j] = temp;
            j++;
        }
        i++;
    }
}

void main(int argc, char** argv) {
    
    int i;
	int my_rank;       // Identificador deste processo
	int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)  
	int message;       // Buffer para as mensagens
	int delta = 1000;
	int curr_size;
	int mid;
	int lchild, rchild;
	MPI_Status status; // estrutura que guarda o estado de retorno          
    MPI_Init(&argc , &argv);
    my_rank = MPI_Comm_rank();  // pega pega o numero do processo atual (rank)
    
    // recebo vetor
    if ( my_rank != 0 ) {
        MPI_Recv(arr, ARR_SIZE, pai, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
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

    // dividir ou conquistar?
    if ( tam_vetor <= delta ) {
        bubblesort(arr, curr_size);  // conquisto
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
        printf("[%d] recebeu devolucao do lchild.\n");           
        MPI_Recv (&arr[mid], curr_size-mid, MPI_INT, rchild,
                  MPI_ANY_TAG, MPI_COMM_WORLD, &status);   
        printf("[%d] recebeu devolucao do rchild.\n");
        merge(arr, curr_size);
    }

    // mando para o pai
    if ( my_rank !=0 ) {
        MPI_Send(arr, curr_size, MPI_INT, status.MPI_SOURCE,
                 1, MPI_COMM_WORLD);  // tenho pai, retorno vetor ordenado pra ele
    }
    else {
        printf("Root results:\n");
        for(i=0; i<ARR_SIZE; i++) {
            printf("%d, ", arr[i]);
        }
    }
    
    MPI_Finalize();
}
