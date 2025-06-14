# 🌊 Simulateur d'eau OpenGL

## Ce projet est un simulateur d'eau en **C++ et OpenGL**.

## 🖥️ Dépendances

### Sur Arch Linux / Manjaro :

```bash
sudo pacman -S glew glfw-x11 glm
```

### Sur Ubuntu / Debian :

```bash
sudo apt install libglew-dev libglfw3-dev libglm-dev
```

---

## 📁 Structure du projet

```
water_sim/
├── CMakeLists.txt
├── shaders/
│   ├── wave.vert
│   └── wave.frag
├── include/
│   ├── grid.hpp
│   └── shader_utils.hpp
├── src/
│   ├── main.cpp
│   ├── grid.cpp
│   └── shader_utils.cpp
└── build/   (sera généré par CMake)
```

---

## ⚙️ Compilation

Depuis la racine du projet :

```bash
mkdir build
cd build
cmake ..
make
```

---

## 🚀 Exécution

Toujours dans le dossier `build/` :

```bash
./water_sim
```
