#include "quadtree.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

const int POPULATION = 1000;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
const int SUB_STEPS = 30;
const int MAX_VELOCITY = 100;
const int MAX_TREE_DEPTH = 10;

typedef struct Particles {
  Vector2 *position;
  Vector2 *velocity;
  Vector2 *acceleration;
  float *mass;
  float *radius;
  Color *color;
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

void CollideWithParticles(Particles *particles, tree_root_t *root,
                          int currentIdx, float dt, int *nearbyParticles) {

  aabb_t particle_rect =
      (aabb_t){particles->position[currentIdx].x - 25,
               particles->position[currentIdx].y - 25, 50, 50};

  int size = GetElementsInRect(root, particle_rect, nearbyParticles);

  for (int i = 0; i < size; i++) {
    if (nearbyParticles[i] == currentIdx) {
      continue;
    }

    float dist =
        Vector2Length(Vector2Subtract(particles->position[nearbyParticles[i]],
                                      particles->position[currentIdx]));

    if (dist <
        particles->radius[currentIdx] + particles->radius[nearbyParticles[i]]) {
      // Collide
      Vector2 p1 = particles->position[currentIdx];
      Vector2 p2 = particles->position[nearbyParticles[i]];
      Vector2 pdir; // Just in case the 2 particles somehow make it exactly
                    // ontop of one another
      if (Vector2LengthSqr(Vector2Subtract(p2, p1)) == 0) {
        pdir = (Vector2){1, 0};
      } else {
        pdir = Vector2Normalize(Vector2Subtract(p2, p1));
      }
      float distMove = ((particles->radius[nearbyParticles[i]] +
                         particles->radius[currentIdx]) -
                        Vector2Length(Vector2Subtract(p2, p1))) /
                       2;
      p1 = Vector2Add(Vector2Scale(pdir, -distMove), p1);
      p2 = Vector2Add(Vector2Scale(pdir, distMove), p2);

      Vector2 v1 = particles->velocity[currentIdx];
      Vector2 v2 = particles->velocity[nearbyParticles[i]];
      Vector2 v1f = Vector2Subtract(
          v1, Vector2Scale(Vector2Subtract(p1, p2),
                           (2 * particles->mass[nearbyParticles[i]]) /
                               (particles->mass[currentIdx] +
                                particles->mass[nearbyParticles[i]]) *
                               Vector2DotProduct(Vector2Subtract(v1, v2),
                                                 Vector2Subtract(p1, p2)) /
                               Vector2LengthSqr(Vector2Subtract(p1, p2))));
      Vector2 v2f = Vector2Subtract(
          v2, Vector2Scale(Vector2Subtract(p2, p1),
                           (2 * particles->mass[currentIdx]) /
                               (particles->mass[nearbyParticles[i]] +
                                particles->mass[currentIdx]) *
                               Vector2DotProduct(Vector2Subtract(v2, v1),
                                                 Vector2Subtract(p2, p1)) /
                               Vector2LengthSqr(Vector2Subtract(p2, p1))));

      particles->position[currentIdx] = p1;
      particles->position[nearbyParticles[i]] = p2;
      particles->velocity[currentIdx] = v1f;
      particles->velocity[nearbyParticles[i]] = v2f;
    }
  }
}

void UpdateParticles(Particles *particles, tree_root_t *root, float dt,
                     int *nearbyParticles) {

  for (int i = 0; i < POPULATION; i++) {
    CollideWithWalls(&(particles->position[i]), &(particles->velocity[i]), dt);
    memset(nearbyParticles, -1, POPULATION * sizeof(int));
    CollideWithParticles(particles, root, i, dt, nearbyParticles);

    particles->position[i] = Vector2Add(
        particles->position[i], Vector2Scale(particles->velocity[i], dt));
  }
}

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Here We Go again!");
  SetTargetFPS(30);

  Particles particles;
  int *nearbyParticles = (int *)malloc(sizeof(int) * POPULATION);

  particles.position = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
  particles.velocity = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
  particles.acceleration = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
  particles.mass = (float *)malloc(sizeof(float) * POPULATION);
  particles.radius = (float *)malloc(sizeof(float) * POPULATION);
  particles.color = (Color *)malloc(sizeof(Color) * POPULATION);

  for (int i = 0; i < POPULATION; i++) {
    particles.position[i] =
        (Vector2){rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT};
    particles.velocity[i] =
        (Vector2){rand() % MAX_VELOCITY - MAX_VELOCITY / 2.0f,
                  rand() % MAX_VELOCITY - MAX_VELOCITY / 2.0f};
    particles.acceleration[i] = (Vector2){0, 0};
    particles.mass[i] = 1;
    particles.radius[i] = 5;
    particles.color[i] = (i < 10) ? RED : BLUE;
  }

  tree_root_t root;

  InitTree(&root, SCREEN_WIDTH, SCREEN_HEIGHT);

  float deltaTime = GetFrameTime();

  if (deltaTime <= 0) {
    deltaTime = 1;
  }

  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();
    float subDT = deltaTime / SUB_STEPS;

    for (int i = 0; i < SUB_STEPS; i++) {
      ResetTree(&root);

      for (int i = 0; i < POPULATION; i++) {
        InsertElementTree(&root, particles.position[i].x,
                          particles.position[i].y, i);
      }
      UpdateParticles(&particles, &root, subDT, nearbyParticles);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(10, 10);
   /* for (int i = 0; i < root.nodeNum; i++) {
      DrawRectangleLines(root.nodes[i].rect.x, root.nodes[i].rect.y,
                         root.nodes[i].rect.w, root.nodes[i].rect.h, GREEN);
    }*/
    for (int i = 0; i < POPULATION; i++) {
      DrawCircleV(particles.position[i], particles.radius[i],
                  particles.color[i]);
    }

    EndDrawing();
  }

  free(nearbyParticles);

  CleanUpTree(&root);

  CloseWindow();

  return 0;
}
