
struct Particle {
  Vec3 position;
  Vec3 velocity;

  u16 lifespan;
  u16 age;
};

void UpdateParticles();
Particle* SpawnParticle(Texture texture, Vec3 position, Vec3 velocity, int lifespan);
