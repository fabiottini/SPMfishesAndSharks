#ifndef FS_H
#define FS_H

#define SERVER_PORT 60001

class FishSharks{ 
  private:
    int N;
    long int NN;
    int M; 
    int NT;
    char SOCKET;

    void socketCom(Matrix);
    void returnMatrixSoket(int, Matrix);
    cell** readStatus(cell**, cell**, unsigned int*, int, Matrix, FILE*, int);
    void editingAction(cell**,cell**, int, int, int, unsigned int*);

  public:
    FishSharks( int, int, int, int, int, int );


};

#endif
