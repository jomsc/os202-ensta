# Partie 1

## Question 1
La machine sur laquelle est exécutée le code possède un processeur Intel i7-8750H avec 16 Go de RAM. 
Il possède 6 coeurs physique, 9 Mo de cache L3, 6x256 Ko de cache L2 et 6x64 Ko de cache L1.

## Question 2
On prend les paramètres de simulation suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 0]
- Position initiale du foyer (col, ligne) : (100, 100)

On obtient comme temps moyen pour chaque pas de temps environ 125 ms.
Pour le temps d'avancement moyen par itération, on a en moyenne 10 ms.
Pour le temps d'affichage moyen par itération, on a en moyenne 18 ms.

Il faut cependant noter la présence de la ligne suivante dans le main :
```std::this_thread::sleep_for(0.1s);```
Ce qui rajoute 100ms à chaque pas de temps. 
Sans cette ligne, on est à 8.29 ms par pas de temps.
On a alors 2.31 ms comme temps d'avancement moyen et 5.00 ms comme temps d'affichage
moyen. 

## Question 3
Voir fichier model.cpp, dans la fonction Model::update.

## Question 4
On a bien la même simulation.

## Question 5
On prend les paramètres de simulation suivants : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)

Temps d'avancement moyen par itération :
1 thread : 2.33 ms
2 threads : 1.60 ms
4 threads : 1.08 ms
6 threads : 0.98 ms
8 threads : 1.24 ms
12 threads : 1.32 ms

On voit donc que le temps d'avancement moyen baisse quand le nombre de threads augmente,
jusqu'à une limite ou le temps réaugmente peu à peu. Cela est probablement dû aux paramètres
de la simulation choisie : elle n'est pas très grande, donc arrive un moment où 
il n'est pas rentable de rajouter des threads. 