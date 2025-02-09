#include <chrono>
#include <random>
#include <cstdlib>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <mpi.h>

#define NB_POINTS 20000000

// Attention , ne marche qu'en C++ 11 ou supérieur :
unsigned long count_points( unsigned long nbSamples ) 
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();
    myclock::duration d = beginning.time_since_epoch();
    unsigned seed = d.count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution <double> distribution ( -1.0 ,1.0);
    unsigned long nbDarts = 0;
    // Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
    for ( unsigned sample = 0 ; sample < nbSamples ; ++ sample ) {
        double x = distribution(generator);
        double y = distribution(generator);
        // Test if the dart is in the unit disk
        if ( x*x+y*y<=1 ) nbDarts ++;
    }
    // Number of nbDarts throwed in the unit disk
    return nbDarts;
}

int main( int nargs, char* argv[] )
{
	int nbp, rank;
	MPI_Init( &nargs, &argv );

	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	
	MPI_Comm_size(globComm, &nbp);
	MPI_Comm_rank(globComm, &rank);
	
	unsigned long ptstoprocess = NB_POINTS/nbp;
	if (rank==nbp-1) { ptstoprocess += NB_POINTS-nbp*ptstoprocess; }

	unsigned long points = count_points(ptstoprocess);

	if (rank != 0) {
		MPI_Send(&points, 1, MPI_UNSIGNED_LONG, 0, 1, globComm);
	} else {
		for (int i=1;i<nbp;i++) {
			unsigned long nbReceived;
			MPI_Recv(&nbReceived, 1, MPI_UNSIGNED_LONG, i, 1, globComm, MPI_STATUS_IGNORE);
			points += nbReceived;
		}
		
		double ratio = 4*double(points)/double(NB_POINTS);

		std::cout << "Pi = " << ratio << std::endl;
	}

	MPI_Finalize();
	return EXIT_SUCCESS;
}

