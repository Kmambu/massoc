M2: TME SoCLib pédagogique
--------------------------

Le TME se divise en 4 parties: vcilink, vcilink2, vcilink3 et iss. Les exercises
sont à faire dans l'ordre.

Les questions sont là pour orienter votre travail et vous aider progresser tout au
long du TME.

A) VciLink : plate-forme simple 1 initiateur/1 cible
----------------------------------------------------

Editer et compléter le fichier system.cpp qui correspond au toplevel de la plate-forme.

Questions:

  1) Faut-il définir une table de correspondance mémoire ? Pourquoi ?

  2) Quel sont les noms des modules initiateur et cibles dans cette plate-forme ?

  3) Quel est le nom du bus qui permet de connecter l'initiateur à la cible ?

B) VciLink2, VciLink3: plate-formes avec réseau d'interconnexion
----------------------------------------------------------------

Editer et compléter le fichier system.cpp qui correspond au toplevel de la plate-forme.

  a) Tout d'abord avec 1 initiateur, un réseau et une cible ;
  b) Ensuite avec 1 initiateur, un réseau et 2 cibles.

Questions:

  1) Qu'est-ce qui définit un segment mémoire (i.e. ses propriétés) ?

  2) Quelle est la plage d'adresses manipulée par l'initiateur ?

  3) Quelles sont la base et la taille du segment mémoire correspondant ?

  4) Que signifie les valeurs numériques dans les paramètres génériques
     du reseau d'interconnexion local_crossbar ?

  5) Combien de segments faut-il définir si la plate-forme contient 2 cibles ?

  6) Quelles sont les propriétés du ou des segments pour que l'initiateur
     écrive d'abord dans la première cible et après 1024 valeurs écrive dans la
     seconde cible ?

C) ISS: créer une plate-forme avec un processeur MiniMIPS, un réseau et une mémoire.
------------------------------------------------------------------------------------

  Editer et compléter le fichier system.cpp qui correspond au toplevel de la plate-forme.

  Prenez le temps d'analyser et de comprendre le petit programme qui est contenu
  dans la mémoire RAM (cf. soclib_vci_simpleram.h).

Questions:

  1) Que faut-il ajouter comme module pour placer un réseau d'interconnexion entre
     la mémoire et le processeur ?

  2) Quelles sont les propriétés de la table de correspondance et du segment qu'elle
     contient ?

  3) Que faut-il faire pour séparer la mémoire d'instruction de la mémoire de données ?
    a) Quels modules faut-il rajouter ?
    b) A quelle adresse commence les instructions et idem pour les données ?
    c) Quels sont les valeurs des propriétés des segments qui doivent composer
       la table de correspondance ?

Bon travail !
