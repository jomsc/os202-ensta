from mpi4py import MPI
import numpy as np 

# MPI initialization 
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

ARRAY_SIZE = 50
MAX = 100 

# chaque process crée son tableau puis le trie
array = np.random.randint(0, MAX+1, ARRAY_SIZE)
array.sort()

# quantile local
quantiles = np.quantile(array, np.linspace(0, 1, size+1))

# on recupere les quantiles de tous les autres process
quantiles_globaux = np.zeros((size+1)*size, dtype='float')
comm.Allgather([quantiles, MPI.FLOAT],
               [quantiles_globaux, MPI.FLOAT])

# on cree les quantiles finaux
quantiles_globaux.sort()
quantiles_finaux = np.quantile(quantiles_globaux, np.linspace(0, 1, size+1))

bucket_locs = []
# on envoie les donnees aux autres process
for j in range(0, size):
    bucket_locs.append(array[(array >= quantiles_finaux[j]) 
                            & (array < quantiles_finaux[j+1])])

final_array = comm.alltoall(bucket_locs)
final_array = np.concatenate(final_array)
final_array.sort()
print(f"{rank}: {final_array}")
