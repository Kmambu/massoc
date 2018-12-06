TME SystemC-TLM2: Implementation d'une plateforme simple en TLM2
----------------------------------------------------------------

L'objectif de ce TME est de vous faire construire une plateforme simple,
composée d'un processor Minimips (tel que vous l'avez vu au TME2) et d'une
mémoire, au niveau d'abstraction SystemC-TLM2.

Dans un deuxième temps, vous ajouterez à cette plateforme un mini réseau
d'interconnexion, sous la forme d'un routeur qui est capable de rediriger les
réquêtes d'un unique initiateur (le Minimips) vers plusieurs cibles (des
mémoires), en effectuant le décodage d'adresse adéquat.


A/ Ecriture du composant mémoire en TLM2 (a_minimips_simple/)
-------------------------------------------------------------

Le code du Minimips décrit en TLM2 (qui implémente les mêmes fonctionnalités
que celui que vous avez utilisé au TME2) est déjà disponible dans le fichier
minimips.h.

- Vous devez compléter le fichier main.cpp pour définir la connexion entre le
  port de sortie du processeur (de type "initiator") vers le port d'entrée de
  la mémoire (de type "target").

- Vous devez compléter le fichier memory.h pour attacher la fonction de
  callback "b_transport" au port d'entrée, et pour remplir la fonction
  "b_transport".

Pour cet exercice, vous devrez lire, comprendre et vous inspirer du tutorial
sur SystemC-TLM2 disponible à l'adresse suivante :

http://www.doulos.com/knowhow/systemc/tlm2/tutorial__1/


B/ Ajout d'un routeur (b_minimips_simple/)
------------------------------------------

On souhaite maintenant séparer le code et les données de l'application
assembleur afin de mettre ces deux segments dans des bancs mémoires séparés.

Cette nouvelle plateforme sera alors composée d'un processeur Minimips, de
deux bancs mémoires (l'un contenant le segment de code, l'autre le segment des
données) et d'un routeur à une entrée et deux sorties afin de relier ces trois
composants.

Vous aurez à modifier ou créer les fichiers suivants :

- router.h : le code du routeur à 1 entrée et N sorties. Pour l'écriture de ce
  fichier, vous devrez lire, comprendre et vous inspirer du tutorial
  disponible à l'adresse suivante :
  http://www.doulos.com/knowhow/systemc/tlm2/tutorial__3/

  Attention, la fonction de décodage qui effectue le routage en fonction de
  l'adresse mémoire des transactions doit être modifiée par rapport à
  l'exemple du tutorial !

- memory.h : ce composant doit maintenant être générique dans le sens où il
  doit être initialisé avec des données différentes selon qu'il est utilisé
  pour contenir le segment de code ou de données. Vous modifierez le
  constructeur pour recevoir un tableau d'entiers contenant les données
  d'initialisation.

- main.cpp : il faut maintenant deux bancs mémoire, le routeur, et effectuer
  les connexions adéquates. Il faut également donner en argument de
  construction des bancs mémoires un tableau d'entiers qui contient soit le
  code, soit les données.
