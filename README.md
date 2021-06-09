# nrf24l01-crypt
Un simple moyen d'envoyer et de recevoir des messages cryptés ou non

## Fonctionnement
Utiliser le serial pour communiquer à distance avec le module nrf24l01 ou un autre module sans file (vous aurez besoin de changer le code pour adapter votre module).

Ce programme vous propose un moyen d'envoyer un message crypté (taper juste votre message et ENTREE) ou non crypter (ajoutez juste le caractère $ devant votre message pour dire au programme de ne pas encoder votre message).

La réception est aussi simple : le programme contient un filtre qui détecte si un message est encodé ou pas et le décode s'il le faut.

## Attention
Pour pouvoir envoyer ou recevoir des messages cryptés vous devez installer la librarie crypto (<https://github.com/alexElArte/Crypto>) et vous devez avoir les mêmes clés, elles se trouve au début (e.g keyC et keyM).

Pour le serial vous devez le configurer à la bonne vitesse (par défault 115200) sans "Nouvelle Ligne" (en bas à droite), configurez-le en "Pas de fin de ligne", l'affichage sera meilleur.
