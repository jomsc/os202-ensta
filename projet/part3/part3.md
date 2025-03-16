# Partie 3

## Question 1

Voir les fichiers simulation.cpp et model.cpp.
On utilise l'option mpi ```--bind-to none```.

## Question 2
On utilise 2 processus MPI et les paramètres suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)
  
Temps moyen global par itération : 
1 thread : 5.65 ms
2 threads : 5.64 ms
3 threads : 5.56 ms
4 threads : 6.08 ms
6 threads : 6.11 ms

Ainsi, on a un optimum pour 3 threads. Cela est probablement dû au fait que rajouter des threads 
réduit le temps de l'avancement mais augmente le temps d'affichage le temps de rassembler les données 
(se référer aux questions suivantes). 

## Question 3 
Avec les mêmes paramètres, on a comme temps moyen pour l'avancement par itération : 
1 thread : 0.43 ms
2 threads : 0.06 ms
3 threads : 0 ms (trop faible por être mesuré)
4 threads : 0 ms (trop faible por être mesuré)
6 threads : 0 ms (trop faible por être mesuré)

Ainsi, on a bien une réduction du temps moyen pour l'avancement lorsque l'on 
augmente le nombre de threads. Cependant, cela ne mène pas forcément à une simulation 
globalement plus rapide car la phase d'affichage prend alors plus de temps. 