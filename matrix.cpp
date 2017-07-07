#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <omp.h>
#include <fstream>

#include "matrix.h"

/**
 * INITIALIZE the random function
 * Given a generic number G, coprime with N, G generate a finite cyclic group of order N that is Zn
 * 		Zn = {0,1,2,...,N-1}
 * so G^0 mod n , G^1 mod n, G^2 mod n..... G^(n-1) mod n are all the numbers in Zn.
 * But we need a non ordered sequence of numbers in the interval, so we return N
 * mod P, where P is prime, so coprime with G and N.
 *
 * @param SEED the seed of the current Thread
 */
MyRandGen::MyRandGen(int SEED){
  A = 1061;
  this-> A = (this->A*SEED)%541;
  this-> A = (this-> A < 0)?this-> A * -1:this-> A ;
 // srand(SEED);
}

/**
 * Return the random generated value
 *
 * @param  MAX The upper limit of generated number
 * @return     Int pseudo-random number
 */
int MyRandGen::gen(int MAX){
  this->A *=1061;
  this->A %=541;
  return ((this->A%MAX)>0)?(this->A%MAX):(this->A%MAX)*-1;
  //return rand()%MAX;
}

/**
 * The function generate 2 matrix and inizialize them with a predefined
 * percentage of fish sharks and empty cells             
 */
void Matrix::initialize(){

  int i; 
  M1 = new cell*[N]; //MATRIX ALLOCATION
  for ( i = 0; i < N; i++ ){
     M1[i] = new cell[N]; //CELL ALLOCATION
  } 

  M2 = new cell*[N]; //BACK MATRIX
  for ( i = 0; i < N; i++ ){
     M2[i] = new cell[N]; 
  }

  int count_s = 0, count_f = 0, count_e =0;

  #if SINGLE_TH_GENERATOR  == 0 && SEQUENTIAL_EXECUTION == 0
  #pragma omp parallel for num_threads(NT) default(shared)  schedule(static)
  #endif
  for ( i = 0; i < N; i++ ){
     int j, tid, typeSW = 0;

      tid = omp_get_thread_num();
      MyRandGen randg(this->SEED[tid]);

      for( j = 0; j < N; j++ ){
        //pseudo-random type assignment
        typeSW = randg.gen(100);
          if(typeSW < PERCENTAGE_EMPTY){            // 25%  EMPTY
            (M1[i][j]).type = EMPTY;
            (M1[i][j]).age  = BA_EMPTY;
        }else if(typeSW < (PERCENTAGE_SHARKS*2)){   // 25% SHARKS
            (M1[i][j]).type = SHARK;
            (M1[i][j]).age  = BA_SHARK;
        }else{                                      // 50% FISH
            (M1[i][j]).type = FISH;
            (M1[i][j]).age  = BA_FISH;
        }
      }
    }
}

/**
 * The function allow to write the matrix passed as parameter to the file pointer
 * 
 * @param myfile The file pointer where export the matrix
 * @param M      The matrix to export in the file
 */
void Matrix::_export(FILE*  myfile, cell** M){
  int i,j,jj;
  uint8_t type, age;
  char t = ' ';

  if(myfile!=NULL){
    for(i=0;i<N;i++){
      for(j=0;j<N;j++){
          type = (uint8_t)M[i][j].type;
          age  = (uint8_t)M[i][j].age;

          t = ((uint8_t)type==EMPTY)?'E':(((uint8_t)type==FISH)?'F':'S');

          if( i == 0  && j == 0){
              fprintf (myfile,"    ");
              for(jj=0;jj<N;jj++){
                fprintf (myfile,"%5d",jj);
              }
              fprintf (myfile,"\n");
          }
          if( j == 0  )
              fprintf (myfile,"%4d ",i);

          if(j == N-1){
            if( (uint8_t)type == (uint8_t)EMPTY){
              fprintf (myfile,"|    |");
            }else{
              fprintf (myfile,"|%c:%2u|",t,age);
            }
          }else{
            if( (uint8_t)type == (uint8_t)EMPTY){
              fprintf (myfile,"|    ");
            }else{
              fprintf (myfile,"|%c:%2u",t,age);
            }
          }
      }
      fprintf (myfile,"\n");
    }
  }
}

/**
 * The function allow to print the matrix passed as parameter
 * 
 * @param M The matrix to print
 */
void Matrix::_print(cell** M){
  int i,j,jj;
  uint8_t type, age;
  char t = ' ';

  printf("\n\n");

  for(i=0;i<N;i++){
    for(j=0;j<N;j++){
        type = (uint8_t)M[i][j].type;
        age  = (uint8_t)M[i][j].age;

        t = ((uint8_t)type==EMPTY)?'E':(((uint8_t)type==FISH)?'F':'S');

        if( i == 0  && j == 0){
            printf("    ");
            for(jj=0;jj<N;jj++){
              printf("%5d",jj);
            }
            printf("\n");
        }
        if( j == 0  )
            printf("%4d ",i);

        if(j == N-1){
          if( (uint8_t)type == (uint8_t)EMPTY){
            printf("|    |");
          }else{
            printf("|%c:%2u|",t,age);
          }
        }else{
          if( (uint8_t)type == (uint8_t)EMPTY){
            printf("|    ");
          }else{
            printf("|%c:%2u",t,age);
          }
        }
    }
    printf("\n");
  }
  printf("\n\n");
}

/**
 * Free memory for the passed matrix
 * 
 * @param M The matrix to deallocate
 */
void Matrix::_free(cell** M){
  int i,j;
  for(i=0;i<N;i++){
      if(M[i]){
         free(M[i]);
         M[i] = NULL;
      }
  }
  if(M != NULL)
    free(M);
  M= NULL;
}

/**
 * Return the Matrix 1
 * 
 * @return cell** Matrix 1
 */
cell** Matrix::_getM1(){
  return M1;
}

/**
 * Return the Matrix 2
 * 
 * @return cell** Matrix 2
 */
cell** Matrix::_getM2(){
  return M2;
}

/**
 * Return the array list of the seeds one for each thread
 * 
 * @return Int Array of seeds
 */
unsigned int* Matrix::_getSEED(){
  if( this->SEED != NULL )
    return this->SEED;
  else
    return NULL;
}

/**
 * Constructor Matrices 
 * 
 * @param  N      Size of matrix NxN
 * @param  NT     Number of Thread
 * @param  M      Number of loops evolution
 * @param  SEED   The SEED root
 * @param  PRINT  A value from 0-2 to choose which print 
 */
Matrix::Matrix(int N, int NT, int M, int SEED, int PRINT){
  this->N  = N;
  this->NT = NT;
  this->M  = M;
  this->NN = N*N;
  this->SEED = new unsigned int[NT]; //allocation of container of SEED values for each THREAD
  int i ;

  //random BASE SEED is passed to replicate the experiment
  srand(SEED);
  for(i=0;i<NT;i++){
    this->SEED[i] = rand();
  }

  initialize(); //invoke NxN constructor
}
