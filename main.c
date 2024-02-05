#include "quadtree.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

const int POPULATION = 100;

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
                          int currentIdx, float dt) {

  int *nearbyParticles = NULL;

  for (int i = 0; i < POPULATION; i++) {
    if (i == currentIdx) {
      continue;
    }

    float dist = Vector2Length(Vector2Subtract(
        particles->position[i], particles->position[currentIdx]));

    if (dist < particles->radius[currentIdx] + particles->radius[i]) {
      // Collide
      Vector2 p1 = particles->position[currentIdx];
      Vector2 p2 = particles->position[i];
      Vector2 pdir; // Just in case the 2 particles somehow make it exactly
                    // ontop of one another
      if (Vector2LengthSqr(Vector2Subtract(p2, p1)) == 0) {
        pdir = (Vector2){1, 0};
      } else {
        pdir = Vector2Normalize(Vector2Subtract(p2, p1));
      }
      float distMove = ((particles->radius[i] + particles->radius[currentIdx]) -
                        Vector2Length(Vector2Subtract(p2, p1))) /
                       2;
      p1 = Vector2Add(Vector2Scale(pdir, -distMove), p1);
      p2 = Vector2Add(Vector2Scale(pdir, distMove), p2);

      Vector2 v1 = particles->velocity[currentIdx];
      Vector2 v2 = particles->velocity[i];
      Vector2 v1f = Vector2Subtract(
          v1,
          Vector2Scale(Vector2Subtract(p1, p2),
                       (2 * particles->mass[i]) /
                           (particles->mass[currentIdx] + particles->mass[i]) *
                           Vector2DotProduct(Vector2Subtract(v1, v2),
                                             Vector2Subtract(p1, p2)) /
                           Vector2LengthSqr(Vector2Subtract(p1, p2))));
      Vector2 v2f = Vector2Subtract(
          v2,
          Vector2Scale(Vector2Subtract(p2, p1),
                       (2 * particles->mass[currentIdx]) /
                           (particles->mass[i] + particles->mass[currentIdx]) *
                           Vector2DotProduct(Vector2Subtract(v2, v1),
                                             Vector2Subtract(p2, p1)) /
                           Vector2LengthSqr(Vector2Subtract(p2, p1))));

      particles->position[currentIdx] = p1;
      particles->position[i] = p2;
      particles->velocity[currentIdx] = v1f;
      particles->velocity[i] = v2f;
    }
  }
}

void UpdateParticles(Particles *particles, tree_root_t *root, float dt) {

  for (int i = 0; i < POPULATION; i++) {
    CollideWithWalls(&(particles->position[i]), &(particles->velocity[i]), dt);
    CollideWithParticles(particles, root, i, dt);

    particles->position[i] = Vector2Add(
        particles->position[i], Vector2Scale(particles->velocity[i], dt));
  }
}

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Here We Go again!");
  SetTargetFPS(30);

  Particles particles;

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
      UpdateParticles(&particles, &root, subDT);
    }

    /* for (int i = 0; i < root.nodeNum; i++) {
       printf(" %i : %i \n", root.nodes[i].idx, root.nodes[i].elementIdx);
     }
     */

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(10, 10);
    for (int i = 0; i < root.nodeNum; i++) {
      if (root.nodes[i].elementIdx != -1) {
        DrawRectangleLines(root.nodes[i].rect.x, root.nodes[i].rect.y,
                           root.nodes[i].rect.w, root.nodes[i].rect.h, GREEN);
      }
    }
    for (int i = 0; i < POPULATION; i++) {
      DrawCircleV(particles.position[i], particles.radius[i],
                  particles.color[i]);
    }

    int mouse_x = 500;
    int mouse_y = 500;
    aabb_t query_rect = (aabb_t){mouse_x - 200, mouse_y - 200, 400, 400};
    int *results = (int *)malloc(sizeof(int) * root.nodeNum);

    DrawRectangle(query_rect.x, query_rect.y, query_rect.w, query_rect.h,
                  BLACK);

    int size = GetElementsInRect(&root, query_rect, results);
    printf("size : %i\n", size);
    if (results) {
      for (int i = 1; i < size; i++) {
        printf("%i : %i at : %f, %f\n", i, results[i], particles.position[results[i]].x, particles.position[results[i]].y);
        if (CheckCollisionPointAABB(particles.position[results[i]].x,
                                    particles.position[results[i]].y,
                                    query_rect)) {
          DrawCircleV(particles.position[results[i]], 10, YELLOW);
        } else {
          DrawCircleV(particles.position[results[i]], 10, BLACK);
        }
      }
      free(results);
    }
    EndDrawing();
  }

  CleanUpTree(&root);

  CloseWindow();

  return 0;
}
