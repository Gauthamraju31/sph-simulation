#include "world.h"

// Between [0,1]
float rand01()
{
  return (float)rand() * (1.f / RAND_MAX);
}

// --------------------------------------------------------------------
// Between [a,b]
float randab(float a, float b)
{
  return a + (b - a) * rand01();
}

ParticleEmitter::ParticleEmitter(const unsigned int N, float l) : max_particles{N}, lifespan(l)
{
  // Initialize particles
  // We will make a block of particles with a total width of 1/4 of the screen.
  float w = worldConstants::SIM_W / 4;
  for (float y = worldConstants::bottom + 1; y <= 10000; y += worldConstants::r * 0.5f)
  {
    for (float x = -w; x <= w; x += worldConstants::r * 0.5f)
    {
      if (particles.size() > max_particles)
      {
        break;
      }

      Particle p;
      p.pos = glm::vec2(x, y);
      p.pos_old = p.pos + 0.001f * glm::vec2(rand01(), rand01());
      p.force = glm::vec2(0, 0);
      p.sigma = 3.f;
      p.beta = 4.f;
      particles.push_back(p);
    }
  }
}

void ParticleEmitter::emit(glm::vec2 pos)
{

  if (particles.size() > max_particles)
  {
    //destroy(-1);
    // particles.pop_front();
    particles.pop_back();
  }

  Particle p;
  p.pos = pos;
  p.pos_old = p.pos + 0.001f * glm::vec2(rand01(), rand01());
  p.force = glm::vec2(0, 0);
  p.sigma = 3.f;
  p.beta = 4.f;
  particles.push_back(p);
}

/* ToDo: Destroy Particle at the index mentioned
void ParticleEmitter::destroy(int index)
{
  if (index>-1)
  { 
     particles.pop_front();
  }
  else 
  { 
     particles. 
  }
}
*/

World::World() : particle_emitter{2048, -1}, indexsp{4093, worldConstants::r, true}
{
}

// VELOCITY
// This modified verlet integrator has dt = 1 and calculates the velocity
// For later use in the simulation.
void World::calc_velocity()
{
#pragma omp parallel for
  for (int i = 0; i < (int)particle_emitter.particles.size(); ++i)
  {
    // Apply the currently accumulated forces
    particle_emitter.particles[i].pos += particle_emitter.particles[i].force;

    // Restart the forces with gravity only. We'll add the rest later.
    particle_emitter.particles[i].force = glm::vec2(0.0f, -worldConstants::G);

    // Calculate the velocity for later.
    particle_emitter.particles[i].vel = particle_emitter.particles[i].pos - particle_emitter.particles[i].pos_old;

    // If the velocity is really high, we're going to cheat and cap it.
    // This will not damp all motion. It's not physically-based at all. Just
    // a little bit of a hack.
    const float max_vel = 2.0f;
    const float vel_mag = glm::dot(particle_emitter.particles[i].vel, particle_emitter.particles[i].vel);
    // If the velocity is greater than the max velocity, then cut it in half.
    if (vel_mag > max_vel * max_vel)
    {
      particle_emitter.particles[i].vel *= .5f;
    }

    // Normal verlet stuff
    particle_emitter.particles[i].pos_old = particle_emitter.particles[i].pos;
    particle_emitter.particles[i].pos += particle_emitter.particles[i].vel;

    // If the Particle is outside the bounds of the world, then
    // Make a little spring force to push it back in.
    if (particle_emitter.particles[i].pos.x < -worldConstants::SIM_W)
      particle_emitter.particles[i].force.x -= (particle_emitter.particles[i].pos.x - -worldConstants::SIM_W) / 8;
    if (particle_emitter.particles[i].pos.x > worldConstants::SIM_W)
      particle_emitter.particles[i].force.x -= (particle_emitter.particles[i].pos.x - worldConstants::SIM_W) / 8;
    if (particle_emitter.particles[i].pos.y < worldConstants::bottom)
      particle_emitter.particles[i].force.y -= (particle_emitter.particles[i].pos.y - worldConstants::bottom) / 8;
    //if( particles[i].pos.y > SIM_W * 2 ) particles[i].force.y -= ( particles[i].pos.y - SIM_W * 2 ) / 8;

    // Handle the mouse attractor.
    // It's a simple spring based attraction to where the mouse is.
    /*
	const float attr_dist2 = glm::dot( particles[i].pos - attractor, particles[i].pos - attractor );
        const float attr_l = worldConstants::SIM_W / 4;
        if( attracting )
        {
            if( attr_dist2 < attr_l * attr_l )
            {
                particle_emitter.particles[i].force -= ( particle_emitter.particles[i].pos - attractor ) / 256.0f;
            }
        }
	*/

    // Reset the nessecary items.
    particle_emitter.particles[i].rho = 0;
    particle_emitter.particles[i].rho_near = 0;
    particle_emitter.particles[i].neighbours.clear();
  }

  // update spatial index
  indexsp.Clear();
  for (auto &particle : particle_emitter.particles)
  {
    indexsp.Insert(glm::vec3(particle.pos, 0.0f), &particle);
  }
}

// DENSITY
// Calculate the density by basically making a weighted sum
// of the distances of neighbouring particles within the radius of support (r)
void World::calc_density()
{
#pragma omp parallel for
  for (int i = 0; i < (int)particle_emitter.particles.size(); ++i)
  {
    particle_emitter.particles[i].rho = 0;
    particle_emitter.particles[i].rho_near = 0;

    // We will sum up the 'near' and 'far' densities.
    float d = 0;
    float dn = 0;
    IndexType::NeighbourList neigh;
    neigh.reserve(64);
    indexsp.Neighbours(glm::vec3(particle_emitter.particles[i].pos, 0.0f), neigh);
    for (int j = 0; j < (int)neigh.size(); ++j)
    {
      if (neigh[j] == &particle_emitter.particles[i])
      {
        // do not calculate an interaction for a Particle with itself!
        continue;
      }

      // The vector seperating the two particles
      const glm::vec2 rij = neigh[j]->pos - particle_emitter.particles[i].pos;

      // Along with the squared distance between
      const float rij_len2 = glm::dot(rij, rij);

      // If they're within the radius of support ...
      if (rij_len2 < worldConstants::rsq)
      {
        // Get the actual distance from the squared distance.
        float rij_len = sqrt(rij_len2);

        // And calculated the weighted distance values
        const float q = 1 - (rij_len / worldConstants::r);
        const float q2 = q * q;
        const float q3 = q2 * q;

        d += q2;
        dn += q3;

        // Set up the Neighbour list for faster access later.
        Neighbour n;
        n.j = neigh[j];
        n.q = q;
        n.q2 = q2;
        particle_emitter.particles[i].neighbours.push_back(n);
        // particle_emitter.particles[i].neighbours.push_back(std::vector < std::map<neigh[j], q, q2>)
      }
    }

    particle_emitter.particles[i].rho += d;
    particle_emitter.particles[i].rho_near += dn;
  }
}

// PRESSURE
// Make the simple pressure calculation from the equation of state.
void World::calc_pressure()
{
#pragma omp parallel for
  for (int i = 0; i < (int)particle_emitter.particles.size(); ++i)
  {
    particle_emitter.particles[i].press = worldConstants::k * (particle_emitter.particles[i].rho - worldConstants::rest_density);
    particle_emitter.particles[i].press_near = worldConstants::k_near * particle_emitter.particles[i].rho_near;
  }
}

// PRESSURE FORCE
// We will force particles in or out from their neighbours
// based on their difference from the rest density.
void World::calc_pressure_force()
{
#pragma omp parallel for
  for (int i = 0; i < (int)particle_emitter.particles.size(); ++i)
  {
    // For each of the neighbours
    glm::vec2 dX(0);
    for (const Neighbour &n : particle_emitter.particles[i].neighbours)
    {
      // The vector from Particle i to Particle j
      const glm::vec2 rij = (*n.j).pos - particle_emitter.particles[i].pos;

      // calculate the force from the pressures calculated above
      const float dm = n.q * (particle_emitter.particles[i].press + (*n.j).press) + n.q2 * (particle_emitter.particles[i].press_near + (*n.j).press_near);

      // Get the direction of the force
      const glm::vec2 D = glm::normalize(rij) * dm;
      dX += D;
    }

    particle_emitter.particles[i].force -= dX;
  }
}

// VISCOSITY
// This simulation actually may look okay if you don't compute
// the viscosity section. The effects of numerical damping and
// surface tension will give a smooth appearance on their own.
// Try it.
void World::calc_viscosity()
{
#pragma omp parallel for
  for (int i = 0; i < (int)particle_emitter.particles.size(); ++i)
  {
    // We'll let the color be determined by
    // ... x-velocity for the red component
    // ... y-velocity for the green-component
    // ... pressure for the blue component
    particle_emitter.particles[i].r = 0.3f + (20 * fabs(particle_emitter.particles[i].vel.x));
    particle_emitter.particles[i].g = 0.3f + (20 * fabs(particle_emitter.particles[i].vel.y));
    particle_emitter.particles[i].b = 0.3f + (0.1f * particle_emitter.particles[i].rho);

    /*
    // For each of that particles neighbours
    for (const Neighbour &n : particle_emitter.particles[i].neighbours)
    {
      const glm::vec2 rij = (*n.j).pos - particle_emitter.particles[i].pos;
      const float l = glm::length(rij);
      const float q = l / worldConstants::r;

      const glm::vec2 rijn = (rij / l);
      // Get the projection of the velocities onto the vector between them.
      const float u = glm::dot(particle_emitter.particles[i].vel - (*n.j).vel, rijn);
      if (u > 0)
      {
        // Calculate the viscosity impulse between the two particles
        // based on the quadratic function of projected length.
        const glm::vec2 I = (1 - q) * ((*n.j).sigma * u + (*n.j).beta * u * u) * rijn;

        // Apply the impulses on the current particle
        particle_emitter.particles[i].vel -= I * 0.5f;
      }
    }
    */
  }
}

void World::init(void)
{
}

void World::step(void)
{
  calc_velocity();
  calc_density();
  calc_pressure();
  calc_pressure_force();
  calc_viscosity();
}

void World::simulate(const int steps)
{
    std::cout << "--------------------------------" << std::endl;
    std::cout  << "Number of steps: " << steps << std::endl;
    std::cout << "Number of particles: 50"  << std::endl;
    const auto beg = std::chrono::high_resolution_clock::now();
    for( unsigned int i = 0; i < steps; ++i )
    {
        step();
    }
    const auto end = std::chrono::high_resolution_clock::now();

    const auto duration( end - beg );
    std::cout << "Elapsed time: " << std::chrono::duration_cast< std::chrono::milliseconds >( duration ).count() << " milliseconds" << std::endl;
    std::cout << "Microseconds per step: " << std::chrono::duration_cast< std::chrono::microseconds >( duration ).count() / (double)steps << std::endl;
    std::cout << std::endl;
}