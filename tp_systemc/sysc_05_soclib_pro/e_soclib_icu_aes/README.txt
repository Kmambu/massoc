TME SoCLib: Incorporation d'un accélérateur AES simple
------------------------------------------------------

L'objectif de ce TME est de vous faire ajouter un nouveau composant à la
bibliothèque SoCLib. Ce composant sera utilisé dans votre système de
simulation et vous compléterez également le logiciel embarqué pour vous en
servir correctement.

A/ Ecriture du composant AES
----------------------------

Un squelette du composant vous est fourni dans le répertoire 'vci_aes' au sein
de votre plateforme. A vous de le compléter pour qu'il fonctionne
correctement.

Ce composant possède deux interfaces, c'est à dire deux segments dans l'espace
d'adressage.

- le segment AES_DATA, par lequel l'utilisateur accède aux données internes
  de l'accélerateur. En écriture, ces données sont celles qui seront
  déchiffrées par l'accélérateur. En lecture, ces données contiennent le
  résultat du déchiffrement. Ce segment a une taille maximale de 4 Koctets.

- le segment AES_CONF, par lequel l'utilisateur peut configurer quelques
  registres de configuration :
   - r_size : la taille des données que l'utilisateur a écrit dans AES_DATA
     pour le déchiffrement
   - r_mode : si r_mode == SOCLIB_AES_RUNNING, alors l'accélérateur doit
     démarrer le déchiffrement.
   - r_irq : si ce registre est écrit, il repasse à 'false' automatiquement
     (peu importe la valeur de l'écriture)


-> Ouvrez le fichier 'vci_aes/caba/source/src/vci_aes.cpp' et completez le.
Regardez dans les autres fichiers qui composent ce composant pour savoir
comment s'en servir.

B/ Ecriture du code logiciel embarqué
-------------------------------------

Un squelette du code logiciel embarqué vous est déjà fourni dans
'soft/main.c'. Complétez le, aux endroits indiqués par les commentaires qui
commencent par '// TME', afin de finir la configuration de l'accélérateur AES,
ainsi que son lancement.

C/ Aller plus loin
------------------

Pour ceux qui arriveraient à terminer le sujet ci-dessus avant le temps
imparti, voici quelques pistes d'amélioration de votre accélérateur AES :

- Ajout d'une latence. Il est peu probable qu'un vrai composant AES puisse
  déchiffrer un message en 1 cycle. Ajoutez une latence paramétrable (par le
  constructeur du composant) et faites en sorte de respecter cette latence
  lors d'un déchiffrement, afin de soulever l'interruption qui marque la fin
  du traitement.

- Modifiez la sémantique de r_mode, afin de le composant puisse déchiffrer
  mais également chiffrer des messages. Modifiez le code logiciel embarqué
  pour lancer le chiffrement d'un message, puis le déchiffrement du résultat
  et vérifiez que le résultat en clair correspond bien au message original.

