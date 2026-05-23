#pragma once
#include <glm/glm.hpp>
#include <omp.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>

#include "constants.h"
#include "utils.h"


struct Particle;
struct Neighbour
{
    Particle* j;
    float q, q2;
};


// Fundamental fluid particle
struct Particle
{
  void *parent;
  /* Mass of the particle */
  float mass;
  /* Color of the particle */
  float r, g, b, a;
  /* Duration of the particle */
  float duration;
  /* Pressure on the particle */
  float press;
  /* Near pressure on the partice */
  float press_near;
  /* Density of the particle */
  float rho;
  /* Near density of the partilce */
  float rho_near;
  /* */
  float sigma;
  /* */
  float beta;
  /* 3D Position of the particle */
  glm::vec3 pos;
  glm::vec3 pos_old;
  /* Velocity of the particle */
  glm::vec3 vel;
  /* Force acting on the particle */
  glm::vec3 force;
  /* Neighbour particles & their weighted distances  */
  std::vector < Neighbour > neighbours;
};

class ParticleEmitter
{

private:
  void *parent;
  /* Max particles to consider */
  const unsigned int max_particles;
  /* Lifespan of a particle */
  const float lifespan;

public:
  /* Tracking of emitted particles */
  std::vector<Particle> particles;
  ParticleEmitter(const unsigned int, const float);
  /* Emit a new particle */
  void emit(glm::vec3);
  // void check_life();
  // void destroy();
};

class World
{
private:
  /* Mouse as attracter */
  glm::vec3 attractor;
  /* Attracter toggle */
  bool attracting;
  typedef SpatialIndex<Particle> IndexType;
  IndexType indexsp;
  /* Physics */
  void calc_velocity();
  void calc_density();
  void calc_pressure();
  void calc_pressure_force();
  void calc_viscosity();

public:
  World();
  // ~World();
  void init();
  void step();
  void simulate(const int);

  /* Particle emitter */
  ParticleEmitter particle_emitter;

};
