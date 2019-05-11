CC=mpicxx

all: pi PSRS

pi: src/pi.cpp
	$(CC) src/pi.cpp -o bin/pi

PSRS: src/PSRS.cpp
	$(CC) src/PSRS.cpp -o bin/PSRS

clean:
	rm bin/*

runpi: 
	mpirun -np 4 bin/pi

runPSRS:
	mpirun -np 3 bin/PSRS
