#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

const int POPULATION = 1000;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

typedef struct Particles {
  Vector2 *position;
  Vector2 *velocity;
  Vector2 *acceleration;
  float mass;
  float radius;

} Particles;

void CollideWithWalls(Vector2 *position, Vector2 *velocity, float dt) {
  if (position->x + velocity->x * dt < 0 ||
      position->x + velocity->x * dt > SCREEN_WIDTH) {
    velocity->x *= -1;
  }
  if (position->y + velocity->y * dt < 0 ||
      position->y + velocity->y * dt > SCREEN_HEIGHT) {
    velocity->y *= -1;
  }
}

void CollideWithParticles(Particles *particles, int currentIdx, float dt) {
  for (int i = 0; i < POPULATION; i++) {
    if (i == currentIdx) {
      continue;
    }

    float dist = Vector2Length(Vector2Subtract(
        particles->position[i], particles->position[currentIdx]));
    if (dist < 2 * particles->radius) {
      // Collide
      Vector2 p1 = particles->position[currentIdx];
      Vector2 p2 = particles->position[i];
      Vector2 v1 = particles->velocity[currentIdx];
      Vector2 v2 = particles->velocity[i];
      Vector2 v1f = Vector2Subtract(
          v1, Vector2Scale(Vector2Subtract(p1, p2),
                           (2 * particles->mass) /
                               (particles->mass + particles->mass) *
                               Vector2DotProduct(Vector2Subtract(v1, v2),
                                                 Vector2Subtract(p1, p2)) /
                               Vector2LengthSqr(Vector2Subtract(p1, p2))));
      Vector2 v2f = Vector2Subtract(
          v2, Vector2Scale(Vector2Subtract(p2, p1),
                           (2 * particles->mass) /
                               (particles->mass + particles->mass) *
                               Vector2DotProduct(Vector2Subtract(v2, v1),
                                                 Vector2Subtract(p2, p1)) /
                               Vector2LengthSqr(Vector2Subtract(p2, p1))));

    }
  }
}

void UpdateParticles(Particles *particles, float dt) {
  for (int i = 0; i < POPULATION; i++) {
    CollideWithWalls(&(particles->position[i]), &(particles->velocity[i]), dt);
    CollideWithParticles(particles, i, dt);

    particles->position[i] = Vector2Add(
        particles->position[i], Vector2Scale(particles->velocity[i], dt));
  }
}

int main(void) {
  const int maxVelocity = 10;
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Here We Go again!");
  SetTargetFPS(30);

  Particles particles;

  particles.position = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
  particles.velocity = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
  particles.acceleration = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);

  for (int i = 0; i < POPULATION; i++) {
    particles.position[i] =
        (Vector2){rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT};
    particles.velocity[i] = (Vector2){rand() % maxVelocity - maxVelocity / 2,
                                      rand() % maxVelocity - maxVelocity / 2};
    particles.acceleration[i] = (Vector2){0, 0};
    particles.mass = 1;
    particles.radius = 5;
  }

  float deltaTime = GetFrameTime();

  if (deltaTime <= 0) {
    deltaTime = 1;
  }

  while (!WindowShouldClose()) {

    UpdateParticles(&particles, deltaTime);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(10, 10);

    for (int i = 0; i < POPULATION; i++) {
      DrawCircleV(particles.position[i], particles.radius, BLUE);
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
