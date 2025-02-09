#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int rank, nbp;

    MPI_Init(&argc, &argv);

    MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

    MPI_Comm_size(globComm, &nbp);
    MPI_Comm_rank(globComm, &rank);

    int token;

    if (rank==0) {
        token = 1;
        MPI_Send(&token, 1, MPI_INT, 1, 1, globComm);
        MPI_Recv(&token, 1, MPI_INT, nbp-1, 1, globComm, MPI_STATUS_IGNORE);
        printf("Token = %d\n", token);

    } else if (rank==nbp-1) {
        MPI_Recv(&token, 1, MPI_INT, rank-1, 1, globComm, MPI_STATUS_IGNORE);
        token++;
        MPI_Send(&token, 1, MPI_INT, 0, 1, globComm);

    } else {
        MPI_Recv(&token, 1, MPI_INT, rank-1, 1, globComm, MPI_STATUS_IGNORE);
        token++;
        MPI_Send(&token, 1, MPI_INT, rank+1, 1, globComm);

    }
    
    MPI_Finalize();
    return 0;
}