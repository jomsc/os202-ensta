# Partie 2

## Questions 1 à 3 
Voir fichier simulation.cpp.

## Question 4 
En utilisant les paramètres suivants (identiques à la partie précédente) : 
- Taille du terrain : 300
- Nombre de cellules par direction : 300
- Vecteur vitesse [100, 100]
- Position initiale du foyer (col, ligne) : (150, 150)

On obtient alors comme temps moyen par itération : 
pour 2 processus : 5.27 ms
pour 4 processus : 5.12 ms
pour 6 processus : 6.90 ms

On a donc une réduction d'environ 36% du temps moyen par itération pour 2 processus 
par rapport au code initial (mesuré dans la première partie). C'est plutôt cohérent. 
Par contre, le résultat pour 6 processus nous paraît inexplicable.