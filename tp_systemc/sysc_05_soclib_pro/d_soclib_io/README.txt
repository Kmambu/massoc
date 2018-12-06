TME SoCLib "Pro"
----------------

L'objectif de ce TME est de vous faire prendre en main l'outil de conception SoCLib.
Vous allez manipuler des plate-formes réalistes et écrire du code applicatif capable
de s'exécuter sur ces plate-formes.


A) Prise en main de SoCLib "Pro" (a_soclib_monoproc)
------------------------------------------------------

La plate-forme décrite ici est un monoprocesseur couplé avec un
cache et d'autres périphériques (memoire, TTY).

Objectif: comprendre le fonctionnement d'une plate-forme de conception simulée:
    - finaliser la construction de la plate-forme
    - compiler une application cible (embarquée) et l'exécuter
    - modifier l'application et valider les changements

Questions:
----------

a) Quels composants sont nécessaires pour construire la plate-forme ?

b) Que faut-il faire pour assembler la plate-forme ? Quels fichiers faut-il écrire/modifier ?

c) A quelle adresse le processeur MIPS charge la première instruction
   au démarrage ?

d) Quelles sont les étapes nécessaires à l'ajout d'un composant ? Que faut-il
   modifier/ajouter dans la plate-forme ?

e) Ou se trouve le code source du programme qui s'exécute sur le processeur mips32 ?
   Que faut-il faire pour changer le message qui s'affiche ?


B) Utilisation de périphériques (b_soclib_icu_timer)
------------------------------------------------------

La plate-forme utilisée dispose d'un contrôleur d'interruption configurable
(ICU) et d'un timer (TIMER).

Objectif: ajouter un composant dans une plate-forme existante:
    - exécuter une application simple (i.e. printf)
    - ajouter un composant timer et le connecter à la plate-forme
    - gérer les interruptions
    - modifier l'application pour afficher un message tous les 50 000 cycles

Questions:
----------

a) Qu'est-ce qu'un contrôleur d'interruption ?

b) Qu'est-ce qu'un timer ? Comment le configure-t-on ?

c) Comment ajouter un composant dans la plate-forme ? Quelles sont les étapes ?

d) A quelle adresse mémoire sont associés le contrôleur d'interruption et le timer ?

e) Qu'est-ce qu'une routine d'interruption ? Comment l'initialiser ?

f) Comment configurer le contrôleur d'interruption pour qu'il soit sensible
   sur l'interruption levée par le timer ?

g) Il est nécessaire d'accusé réception d'une interruption pour que le signal
   soit inactif. Que faut-il faire pour cela ?


C) Utilisation d'un debogueur (c_soclib_icu_timer_gdb)
---------------------------------------------------------

Utiliser un debogueur pour vérifier le comportement d'une application.

Objectif: Modifier la plate-forme précédente pour ajouter un debogueur.


D) VCI I/O (d_soclib_io)

- Rajouter un compteur 4 bit à l'entrée du VciIo.

- Écriver la valeur d'un compteur soft sur la sortie du VciIo.

- Combien de cycles prend la lecture ?

- Combien de cycles prend l'écriture ?


E) Incorporation d'un accélérateur AES simple (e_soclib_icu_aes)

cf. e_soclib_icu_aes/README.txt
