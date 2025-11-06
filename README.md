# 🌊 OpenGL Water Simulation : Rendu Dynamique et Modélisation Physique de l'Eau

## 🌟 Présentation du Projet et Contexte Académique

Ce projet est un simulateur graphique en temps réel des surfaces marines, développé en **C++** et **OpenGL**. Il modélise la dynamique de l'eau et applique des techniques de rendu avancées pour un affichage réaliste.

Le projet a été réalisé **en binôme** (par **Bérenger Chedal-Anglay** et **Martin Kanounnikoff** ) dans le cadre du cours **POGL (Programmation OpenGL)** à l'EPITA, un cours avancé de programmation graphique.

-----

## ✨ Fonctionnalités Clés et Interactivité

Le simulateur offre une expérience interactive et illustre plusieurs concepts avancés :

  * **💧 Simulation d'Impacts Dynamiques (Goutte d'eau) :** Génération et propagation réaliste des ondes de choc sur la surface de l'eau suite à la **chute d’objets** (clic). La modélisation utilise une **fonction gaussienne**  pour appliquer une perturbation initiale à la hauteur de l'eau.
  * **⛵ Mouvement d'un Bateau :** Simulation du passage d'un **bateau** et de son sillage, générant des ondes d’impact de manière réaliste.
  * **🌊 Modélisation de la Surface :** L'eau est modélisée par une **grille uniforme bidimensionnelle** , où chaque point $(x, y)$ contient la hauteur de l'eau ($h$), la vitesse horizontale ($u$) et la vitesse verticale ($v$).
  * **🫧 Rendu Avancé de l'Eau :** Affichage de l'**écume**  et effets de réflexion et de réfraction via les shaders GLSL.
  * **🕹️ Caméra Interactive :** Contrôle total de la caméra permettant d'explorer la scène 3D.

-----

## 🔬 Modélisation Physique et Technique

La dynamique de l'eau est gérée par la résolution numérique des **Équations de Barré de Saint-Venant (Shallow Water)**.

  * **Résolution :** Les équations (Conservation de la masse et de la quantité de mouvement ) sont résolues en utilisant un schéma aux **différences finies** explicite.
  * **Hypothèses :** Le modèle utilise des hypothèses simplificatrices (surface plane , fluide incompressible et homogène , négligence de la viscosité, de l'effet de Coriolis et du frottement ) pour garantir la performance en temps réel.
  * **Impact :** L'impact est modélisé en augmentant la hauteur de l'eau $h(x,y)$ au point d'impact selon une fonction gaussienne.

-----

## ⚙️ Dépendances et Compilation

Ce projet utilise `CMake` pour gérer la compilation et nécessite les bibliothèques suivantes :

### Dépendances Requises

  * **OpenGL 4.x** (Core Profile)
  * **C++11** ou supérieur
  * **GLFW** : Gestion des fenêtres et des entrées (clavier/souris).
  * **GLAD/GLEW** : Chargement des fonctions OpenGL.
  * **GLM** : Bibliothèque de mathématiques pour OpenGL (matrices, vecteurs, etc.).

### Instructions de Compilation

Suivez ces étapes dans votre terminal pour compiler et lancer le simulateur :

1.  **Cloner le dépôt :**
    ```bash
    git clone https://github.com/BerengerClay/opengl-water-simulation.git
    cd opengl-water-simulation
    ```
2.  **Initialisation et compilation avec CMake :**
    ```bash
    mkdir build
    cd build
    cmake .. 
    cmake --build .
    ```
3.  **Exécuter la simulation :**
    ```bash
    ./water_sim
    ```

### ⌨️ Commandes d'Utilisation

| Action | Contrôle |
| :--- | :--- |
| **Mouvement de la Caméra** | Clavier (W, A, S, D) |
| **Rotation de la Caméra** | Souris (Bouton droit maintenu) |
| **Générer un Impact (Goutte)** | Clic Gauche de la souris sur la surface |

-----

## 📖 Documentation Technique

Pour une analyse détaillée du modèle physique (équations Shallow Water), des hypothèses de linéarisation, et du schéma numérique utilisé, consultez la présentation académique du projet :

**[Présentation POGL.pptx - Modélisation et Simulation (Lien Public)](https://1drv.ms/p/c/64b674780fdb40ef/EV68lA8Pu9JErXErdLA_QtcBAd5Z5g6e7xlNgHBn5-khTQ?e=I3G9iX)**