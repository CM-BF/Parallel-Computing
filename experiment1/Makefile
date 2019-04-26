CC=g++

All: pi1 pi2 pi3 pi4 PSRS

pi1: pi1.cpp
	$(CC) pi1.cpp -fopenmp -o bin/pi1

pi2: pi2.cpp
	$(CC) pi2.cpp -fopenmp -o bin/pi2

pi3: pi3.cpp
	$(CC) pi3.cpp -fopenmp -o bin/pi3

pi4: pi4.cpp
	$(CC) pi4.cpp -fopenmp -o bin/pi4

PSRS: PSRS.cpp
	$(CC) PSRS.cpp -fopenmp -o bin/PSRS

clear:
	rm bin/*