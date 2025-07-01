#include "simulation.hpp"
#include <cmath>
#include <algorithm>

Simulation::Simulation(int size, float dx, float dt, float damping)
    : N(size), dx(dx), dt(dt), damping(damping),
      h((size + 1) * (size + 1), 0.0f),
      u((size + 1) * (size + 1), 0.0f),
      v((size + 1) * (size + 1), 0.0f),
      h_new((size + 1) * (size + 1), 0.0f),
      u_new((size + 1) * (size + 1), 0.0f),
      v_new((size + 1) * (size + 1), 0.0f)
{
}

int Simulation::getSize() const { return N; }
const std::vector<float> &Simulation::getHeight() const { return h; }

void Simulation::addDrop(int cx, int cy, float amp, int radius)
{
  float sigma = radius / 2.0f;
  float twoSigma2 = 2.0f * sigma * sigma;
  for (int dy = -radius; dy <= radius; ++dy)
  {
    for (int dx_ = -radius; dx_ <= radius; ++dx_)
    {
      int x = cx + dx_, y = cy + dy;
      if (x < 0 || x > N || y < 0 || y > N)
        continue;
      float d2 = float(dx_ * dx_ + dy * dy);
      h[y * (N + 1) + x] += amp * std::exp(-d2 / twoSigma2);
    }
  }
}

void Simulation::update()
{
  float coeff = g * dt / (2.0f * dx);
  int stride = N + 1;

  // Calcul des nouvelles vitesses
  for (int y = 1; y < N; ++y)
  {
    for (int x = 1; x < N; ++x)
    {
      int i = y * stride + x;
      float dhdx = (h[i + 1] - h[i - 1]);
      float dhdy = (h[i + stride] - h[i - stride]);
      u_new[i] = damping * (u[i] - coeff * dhdx);
      v_new[i] = damping * (v[i] - coeff * dhdy);
    }
  }

  // Calcul de la nouvelle hauteur
  float inv2dx = dt / (2.0f * dx);
  for (int y = 1; y < N; ++y)
  {
    for (int x = 1; x < N; ++x)
    {
      int i = y * stride + x;
      float du = (u[i + 1] - u[i - 1]);
      float dv = (v[i + stride] - v[i - stride]);
      h_new[i] = h[i] - inv2dx * (du + dv);
    }
  }

  // Échanges
  std::swap(h, h_new);
  std::swap(u, u_new);
  std::swap(v, v_new);
}

std::vector<float> Simulation::getVelocity() const
{
  std::vector<float> velocity;
  velocity.resize((N+1)*(N+1));
  for (int i = 0; i < (N+1)*(N+1); ++i)
      velocity[i] = std::sqrt(u[i]*u[i] + v[i]*v[i]);
  return velocity;
}

std::pair<float, float> Simulation::getLocalVelocity(int x, int z) const
{
    int idx = z * (N + 1) + x;
    return {u[idx], v[idx]};
}
