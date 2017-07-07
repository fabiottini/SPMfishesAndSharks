#ifndef MATRIX_H

#define MATRIX_H

#define PERCENTAGE_FISH   50
#define PERCENTAGE_SHARKS 25
#define PERCENTAGE_EMPTY  25
 
#define EMPTY 0     //IDENTIFY THE EMPTY 
#define FISH  1     //IDENTIFY THE FISH
#define SHARK 2     //IDENTIFY THE SHARK

#define BA_EMPTY 0  //BREEDING AGE EMPTY CELL
#define BA_FISH  2  //BREEDING AGE FISH  CELL
#define BA_SHARK 3  //BREEDING AGE SHARK CELL

//DEFINE IF THE INITIAL MATRIX IS CREATED WITH SINGLE OR MULTI THREAD
#define SINGLE_TH_GENERATOR 1 //0: MULTI-THREAD GENERATOR 1: SINGLE THREAD GENERATOR
//DEFINE IF ENABLE OR NOT OPENMP 
#define SEQUENTIAL_EXECUTION 0  //0: OPENMP ENABLE 1: OPENMP DISABLE

/**
 * Indentify the single element
 * 	 - 2 bits for type of element:
 * 			1 => FISH 
 *          2 => SHARKS
 *   - 6 bits for the age: 2^6 = 64 << 20
 * i use more bits than i need to reach 1 Byte per cell
 * and simplify the allignament problem.
 */
struct cell{
  uint8_t type:2;
  uint8_t age:6;
};

class MyRandGen{
  private:
    int A;  
  public:
    MyRandGen(int);
    int gen(int);
};

class Matrix{
  private:
    int N;
    long int NN;
    int NT;
    int M;
    unsigned int* SEED;
    cell** M1;
    cell** M2;
    void initialize();

  public:
    Matrix( int, int, int, int, int );
    void _export(FILE*, cell**);
    void _print(cell**);
    void _free(cell**);
    cell** _getM1();
    cell** _getM2();
    unsigned int*   _getSEED();

};
#endif
