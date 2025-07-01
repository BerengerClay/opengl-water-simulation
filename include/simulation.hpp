#pragma once
#include <vector>

class Simulation
{
public:
  Simulation(int size, float dx, float dt, float damping = 0.99f);

  // Met à jour u,v,h sur la grille selon SWE
  void update();

  // Ajoute une goutte gaussienne
  void addDrop(int x, int y, float amplitude, int radius = 3);

  // Accès aux hauteurs
  const std::vector<float> &getHeight() const;
  std::vector<float> getVelocity() const;
  int getSize() const;
  std::pair<float, float> getLocalVelocity(int x, int z) const;

private:
  int N;
  float dx, dt, g = 9.81f, damping;
  std::vector<float> h, u, v, h_new, u_new, v_new;
};
