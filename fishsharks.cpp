#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>  // for INT_MAX
#include <omp.h>
#include <string.h>

#include "matrix.h"
#include "fishsharks.h" 

/**
 * The function generate a string with formatted date time
 * 
 * @return String yyyymmdd_hhMMss
 */
char* _getCurrentTime(){
  time_t     now = time(0);
  struct tm  tstruct;
  char*       buf;
  buf = new char[100];
  tstruct = *localtime(&now);
  strftime(buf,100, "%Y%m%d_%H%M%S", &tstruct); 
  return (char*)buf;
}

/**
 * The function parallelize the access to the matrix and each thread and
 * update the status in the matrix enabled for writing
 * 
 * @param  MR     The Matrix enabled for Reading
 * @param  MS     The Matrix enabled for writing
 * @param  SEED   The Array of SEED of each Thread
 * @param  M      The number of iteraction
 * @param  m      The object Matrix
 * @param  EXPORT The file pointer where to export OR NULL if no exportation
 *                needed
 * @param  PRINT  A value from 0-2 to choose which print
 * @return        Last matrix
 */
cell** FishSharks::readStatus(cell** MR, cell **MS, unsigned int* SEED, int M,
                              Matrix m, FILE* EXPORT, int PRINT){
  int i, iter;
  cell** tmp_M;

  if( PRINT == 2 ){
    printf("INIT MATRIX:\n");
    m._print(m._getM1());
  }

  //EVOLVING LOOP
  for( iter=0; iter<M; iter++){
    #if SEQUENTIAL_EXECUTION == 0 
    #pragma omp parallel for num_threads(NT) schedule(static)
    #endif
    for(i=0;i<N;i++){
        int j, tid;
        
        tid = omp_get_thread_num();
        MyRandGen rg(SEED[tid]); 

        #if SEQUENTIAL_EXECUTION == 0 
        //#pragma vector always
        //#pragma omp simd
        #pragma ivdep
        #endif
        for(j=0;j<N;j++){

          int type, age;
          int r;
          short int ff=0, ss=0, ee=0, ff_cba=0, ss_cba=0;
          int ii,jj, ii_STOP, jj_STOP;

          /*****************************************************
           * EXAMPLE loop execution
           *      +----------+----------+----------+
           *      | i-1, j-1 |          |          |
           *      +----------+----------+----------+
           *      |          |   i,j    |          |
           *      +----------+----------+----------+
           *      |          |          |          |
           *      +----------+----------+----------+
           ******************************************************/

          ii_STOP = (i+1 < N)? i+1:N-1;
          jj_STOP = (j+1 < N)? j+1:N-1;

          ii = (i-1 >= 0)?i-1:0;
          while(ii<=ii_STOP){
           jj = (j-1 >= 0)?j-1:0;
           while(jj<=jj_STOP){

                 type = (int)(MR[ii][jj]).type;
                 age  = (int)(MR[ii][jj]).age;
                 if( type == EMPTY ){
                    ee++;
                 }else if( type == FISH ){
                   ff++;
                   if(age == BA_FISH)
                      ff_cba++; //COUNT BREEDING AGE
                 }else if(type == SHARK ){
                    ss++;
                    if(age == BA_SHARK)
                      ss_cba++; //COUNT BREEDING AGE
                 }
                 jj++;
            }
            ii++;
          }

          type = (int)(MR[i][j]).type;
          age  = (int)(MR[i][j]).age;

          switch( type ){
            case EMPTY: //EMPTY
                /**
                 * EVOLVE
                 *    NEW F or S IF:
                 *      >= 4 neighbors F or S  and  >= 3 with AGE F=2 OR S=3
                 *      and <  4  neighbors S or F
                 * DIES
                 *    -
                 */

                 //ee--; //remove it self

                 if( ((ff >= 4 && ff_cba >= 3) && ss < 4 ) ){ //FISH BORN
                   MS[i][j].type = FISH;
                   MS[i][j].age  = 1;
                 }else if((ss >=4 && ss_cba >= 3) && ff < 4){ //SHARKS BORN
                   MS[i][j].type = SHARK;
                   MS[i][j].age  = 1;
                 }else{
                   MS[i][j].type = type;
                   MS[i][j].age  = age;
                 }
                 break;
            case FISH: //FISHES
                /**
                 * EVOLVE
                 *    AGE ++
                 * DIES
                 *    >= 5 neighbors S
                 *    >= 8 neighbors F
                 */

                 //ff--; //remove it self
                 if(age < 10 ){
                   if( ss>=5 || ff>=8 ){ //DIES: >= 5 neighbors S || DIES: >= 8 neighbors F
                     MS[i][j].type = EMPTY;
                     MS[i][j].age  = BA_EMPTY;
                   }else{
                     MS[i][j].type = FISH;
                     MS[i][j].age  = ++age;
                   }
                 }else{                 //DIE => OLD
                   MS[i][j].type = (uint8_t)EMPTY;
                   MS[i][j].age  = (uint8_t)BA_EMPTY;
                 }
                 break;
            case SHARK: //SHARKS
                /**
                 * EVOLVE
                 *    AGE ++
                 * DIES
                 *    >= 6 neighbors S and F=0
                 *    1/32 probability
                 */

                 //ss--; //remove it self
                 if(age < 20 ){
                   if( ss>=6 && ff==0 ){          //DIES: >= 6 neighbors S and F=0
                     MS[i][j].type = EMPTY;
                     MS[i][j].age  = BA_EMPTY;
                   }else{
                     r = rg.gen(32);

                     if (r > 1){                //AGE++
                       MS[i][j].type = SHARK;
                       MS[i][j].age  = ++age;
                     }else{                      //DIES: 1/32 probability
                       MS[i][j].type = EMPTY;
                       MS[i][j].age  = BA_EMPTY;
                     }
                   }
                 }else{                       //DIE => OLD
                   MS[i][j].type = EMPTY;
                   MS[i][j].age  = BA_EMPTY;
                 }
                 break;
               }
        }
    }

    //EXPORT SELECT
    if(EXPORT != NULL){
      for(i=0;i<(N+2);i++){ fprintf(EXPORT,"%s","_____"); }
      fprintf(EXPORT, "\nITER: %d \n\nREAD MATRIX:\n",iter);
      m._export(EXPORT,MR);
      fprintf(EXPORT,"WRITE MATRIX:\n");
      m._export(EXPORT,MS);
    }

    //EXCHANGE the matrix
    if(iter<(M-1)){ //except if its the last loop
      tmp_M = MR;
      MR    = MS;
      MS    = tmp_M;
      tmp_M = NULL;
    }
  }

  if(PRINT >= 1){
    if(PRINT == 2){
       for(i=0;i<(N+2);i++) printf("%s","_____");
       printf("\n\n");
    }
  }

  if(iter == 0) return MR; //If no iteration return the generated matrix
  
  return MS;
}

/**
 * Constructor FishShark
 * 
 * @param  N      Size of matrix NxN
 * @param  NT     Number of Thread
 * @param  M      Number of loops evolution
 * @param  SEED   The SEED root
 * @param  PRINT  A value from 0-2 to choose which print 
 * @param  EXPORT A value from 0-2 to choose which exprot   
 */
FishSharks::FishSharks( int N, int NT, int M, int SEED, int PRINT, int EXPORT ){
    this->N    = N;
    this->NN   = N*N;
    this->NT   = NT;
    this->M    = M; 

    char fn[100] = {};
    FILE* myfile=NULL;

    cell** RES;
    int i;
    double start,end;

    if(PRINT>=1){
       start = omp_get_wtime();
    }

    Matrix m(N, NT, M, SEED, PRINT);

    if( EXPORT >= 1 ){
      strcat(fn,"export_");
      strcat(fn,_getCurrentTime());
      strcat(fn,".txt");
      myfile = fopen (fn,"w+");
      fprintf(myfile, "\nINIT MATIX:\n");
      m._export(myfile,m._getM1());
    }

    if( EXPORT != 2 ){
      RES = readStatus(m._getM1(), m._getM2(), m._getSEED(), M, m,   NULL, PRINT);
    }else{
      RES = readStatus(m._getM1(), m._getM2(), m._getSEED(), M, m, myfile, PRINT);
    }

    if( PRINT == 2 ){
      printf("RESULT MATRIX:\n");
      m._print(RES);
    }
    if( EXPORT >= 1 ){
      for(i=0;i<(N+2);i++) fprintf(myfile, "%s","_____");
      fprintf(myfile, "\nRESULT MATRIX:\n");
      m._export(myfile,RES);
      if(myfile!=NULL) fclose(myfile);
    }

    free(m._getSEED());
    m._free(m._getM1());
    m._free(m._getM2());

    if(PRINT >= 1){
      end = omp_get_wtime();
      printf("TIME: %.16g\n", (end-start));
    }
}



int main (int argc, char *argv[])
{
  int N = 0, NT = 0, M = 0, SEED = 0, x = 0, PRINT=0, EXPORT=0;
  char * endp;
  long i;

  errno = 0;

  if(argc == 7){
    //N => ROW and COLUMN #
    i = strtol(argv[1],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 0 NOT CORRECT\n\n");
    else N = i;

    //NT => THREAD #
    i = strtol(argv[2],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 1 NOT CORRECT\n\n");
    else NT = i;

    //M => ITERATION #
    i = strtol(argv[3],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 2 NOT CORRECT\n\n");
    else M = i;

    //SEED => THE SEED TO UTILIZE TO GENERATE NT SUB-SEED
    i = strtol(argv[4],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 3 NOT CORRECT\n\n");
    else SEED = i;

    /*
      PRINT
        0: NO PRINT
        1: ENABLE TIME PHASES PRINT
        2: ENABLE TIME PHASES PRINT AND INIT MATRIX PRINT AND END MATRIX PRINT
    */
    i = strtol(argv[5],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 4 NOT CORRECT\n\n");
    else PRINT = i;

    /*
    EXPORT
      0: DISABLE generate a file with all the iteration "export_Ymd_HMs"
      1: ENABLE EXPORT OF INITIAL AND FINAL MATRIX
      2: ENABLE EXPORT OF INITIAL AND FINAL MATRIX AND ALL THE INTERMEDIATE STEPS
    */
    i = strtol(argv[6],&endp,10);
    if (*endp!= '\0' || errno !=0 || i>INT_MAX)
      printf("\n---> ERROR: ARGUMENT 5 NOT CORRECT\n\n");
    else EXPORT = i;

    if(errno == 0){
      FishSharks fs(N, NT, M, SEED, PRINT, EXPORT);
    }
  }else{
    printf("\n---> ERROR: ARGUMENT NUMBER NOT CORRECT\n\n");

    //HELP PRINT
    printf("./fishsharks <N> <TH> <M> <S> <P> <E>\n \
            <N>: NxN MATRIX\n \
            <TH>: # of Thread\n \
            <S>: SEED ROOT\n \
            <P>: PRINT OPTION\n\t \
              0: NO print\n\t \
              1: PRINT time of phases\n\t \
              2: PRINT time of phases + initial and last matrix\n \
            <E>: EXPORT OPTION\n\t \
              0: NO export\n\t \
              1: EXPORT only first and last matrix\n\t \
              2: EXPORT first and last matrix + intermediate matrix phases\n\n");
  }
}
