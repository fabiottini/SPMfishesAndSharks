#!/bin/bash

EXE="fishsharks_mic"
ERROR=0

if [ $# -lt 5 ]
  then
    printf "ERROR: # of INPUT\n\n" 
    printf "./fishsharks_test.sh <S> <E> <N> <M> <SEED>\n \
      <S>: FROM TREAD\n \
      <E>: TO THREAD \n \
      <N>: SIZE OF MATRIX NxN\n \
      <M>: # OF LOOPS\n \
      <SEED>: SEED ROOT\n\n"
      exit
fi 

from=${1:?INSERT FROM}
if ! [[ "$from" =~ ^[0-9]+$ ]]
        then
   printf "ERROR: FROM INPUT\n"
   ERROR=1
fi
to=${2:?INSERT TO}
if ! [[ "$to" =~ ^[0-9]+$ ]]
        then
   printf "ERROR: TO INPUT\n"
   ERROR=1
fi
N=${3:?INSERT N}
if ! [[ "$N" =~ ^[0-9]+$ ]]
        then
   printf "ERROR: N INPUT\n"
   ERROR=1
fi
M=${4:?INSERT M}
if ! [[ "$M" =~ ^[0-9]+$ ]]
        then
   printf "ERROR: M INPUT\n"
   ERROR=1
fi
SEED=${5:?INSERT SEED}
if ! [[ "$SEED" =~ ^[0-9]+$ ]]
        then
   printf "ERROR: SEED INPUT\n"
   ERROR=1
fi

if [ $from -gt $to ]
  then
  printf "ERROR: FROM > TO\n"
  ERROR=1
fi

if [ $ERROR -eq 0 ]
  then

	echo "./$EXE $N <TH> $M $SEED 1 0" > test.txt

	for i in $(seq $from $to); do
	   echo "./$EXE $N $i $M $SEED 1 0"
	   echo "TH: $i" >> test.txt
	   ./$EXE $N $i $M $SEED 1 0 >> test.txt
	done

else 
	exit

fi
