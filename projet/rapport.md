# Rapport OS202 de Maxime KUNSCH et Joseph MOUSCADET

## Partie 1

### Question 1
La machine sur laquelle est exécutée le code possède un processeur Intel i7-8750H avec 16 Go de RAM. 
Il possède 6 coeurs physique, 9 Mo de cache L3, 6x256 Ko de cache L2 et 6x64 Ko de cache L1.

### Question 2
On prend les paramètres de simulation suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 0]
- Position initiale du foyer (col, ligne) : (100, 100)

On obtient comme temps moyen pour chaque pas de temps environ 125 ms.
Pour le temps d'avancement moyen par itération, on a en moyenne 10 ms.
Pour le temps d'affichage moyen par itération, on a en moyenne 18 ms.

Il faut cependant noter la présence de la ligne suivante dans le main : \
```std::this_thread::sleep_for(0.1s);``` \
Ce qui rajoute 100ms à chaque pas de temps. 
Sans cette ligne, on est à 8.29 ms par pas de temps.
On a alors 2.31 ms comme temps d'avancement moyen et 5.00 ms comme temps d'affichage
moyen. 

### Question 3
Voir fichier model.cpp, dans la fonction Model::update.

### Question 4
On a bien la même simulation.

### Question 5
On prend les paramètres de simulation suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)

Temps d'avancement moyen par itération :
| nombre de threads | temps d'avancement moyen par itération | accélération |
| ----------------- | -------------------------------------- | ------------ |
| 1 | 2.33 ms | / |
| 2 | 1.60 ms | 1.44x |
| 4 | 1.08 ms | 2.14x |
| 6 | 0.98 ms | 2.35x |
| 8 | 1.24 ms | 1.86x |
| 12 | 1.32 ms | 1.75x |

On voit donc que le temps d'avancement moyen baisse quand le nombre de threads augmente,
jusqu'à une limite ou le temps réaugmente peu à peu. Cela est probablement dû aux paramètres
de la simulation choisie : elle n'est pas très grande, donc arrive un moment où 
il n'est pas rentable de rajouter des threads. 

## Partie 2

### Questions 1 à 3 
Voir fichier simulation.cpp.

### Question 4 
En utilisant les paramètres suivants (identiques à la partie précédente) : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)

| nombre de processus | temps moyen par itération | accélération |
| ----------------- | -------------------------------------- | ------------ |
| 2 | 5.27 ms | 1.57x |
| 4 | 5.12 ms | 1.62x |
| 6 | 6.90 ms | 1.20x |

On a donc une réduction d'environ 36% du temps moyen par itération pour 2 processus 
par rapport au code initial (mesuré dans la première partie). 4 processus accélèrenet encore plus la simulation. C'est cohérent. Par contre, le résultat pour 6 processus nous paraît inexplicable.

## Partie 3

### Question 1

Voir les fichiers simulation.cpp et model.cpp.
On utilise l'option mpi ```--bind-to none```.

### Question 2
On utilise 2 processus MPI et les paramètres suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)

| nombre de threads | temps moyen global par itération | accélération |
| ----------------- | -------------------------------------- | ------------ |
| 1 | 5.65 ms | 1.56x |
| 2 | 5.64 ms | 1.58x |
| 3 | 5.56 ms | 1.60x |
| 4 | 6.08 ms | 1.46x |
| 6 | 6.11 ms | 1.46x |

Ainsi, on a un optimum pour 3 threads. Cela est probablement dû au fait que rajouter des threads 
réduit le temps de l'avancement mais augmente le temps d'affichage le temps de rassembler les données 
(se référer aux questions suivantes). 

### Question 3 
Avec les mêmes paramètres, on a comme temps moyen pour l'avancement par itération : 

| nombre de threads | temps moyen pour l'avancement par itération |
| ----------------- | -------------------------------------- |
| 1 | 0.43 ms | 
| 2 | 0.06 ms |
| 3 | 0 ms | 
| 4 | 0 ms | 
| 6 | 0 ms | 

Ainsi, on a bien une réduction du temps moyen pour l'avancement lorsque l'on 
augmente le nombre de threads. Cependant, cela ne mène pas forcément à une simulation 
globalement plus rapide car la phase d'affichage prend alors plus de temps. 

## Partie 4

Voir fichier simulation.cpp.
