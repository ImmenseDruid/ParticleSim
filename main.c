#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

const int POPULATION = 100;

const int SCREEN_WIDTH = 200;
const int SCREEN_HEIGHT = 200;
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

typedef struct quadElement {
  int idx;            // Used to look up particle information
  int x1, y1, x2, y2; // Rectangle for the element
} quad_element_t;

typedef struct quadElNode {
  int next;    // points to the next element
  int element; // points to an element
} quad_element_node_t;

typedef struct quadNode { //
  int firstChild;         // pointer to the first element node
  int count;              // how many nodes exist -1 if not a leaf
} quad_node_t;

typedef struct quadTree {
  quad_node_t *nodes;             // List of all the children in the tree
  quad_element_node_t *elemNodes; // list of the element nodes
  quad_element_t *elements;       // List all of the quads of the tree
  Rectangle root;                 // Original Rectangle
  int freeNode;                   // first free node in the tree

} quad_tree_t;

bool insertParticle(quad_tree_t *tree, quad_node_t *node, Rectangle bounds,
                    Vector2 *position, int idx, int depth) {

  if (node == NULL) {
    printf("Error Null node dont know how this happend?\n");
    return false;
  }
  if (!CheckCollisionPointRec(position[idx],
                              bounds)) { // If not in the rectangle then
                                         // dont insert the particle
    return false;
  }

  if (depth > MAX_TREE_DEPTH) {

    return true;
  }

  if (node->firstChild == -1) {
    int add = tree->freeNode;
    for(int i = 0; i < 4; i++){
        tree->nodes[tree->freeNode + i].firstChild = -1;
        tree->nodes[tree->freeNode + i].count = -1;
        tree->freeNode++;
    }

     node->firstChild = add; // set to tree->freeNode - 5 eventually
    
  }

  if (position[idx].x < bounds.x + bounds.width / 2 &&
      position[idx].y < bounds.y + bounds.height / 2) {
    // would have returned if it was not within
    // bounds so we only have to check where in
    // the bounds it is Upper left

    return insertParticle(
        tree, tree->nodes + node->firstChild,
        (Rectangle){bounds.x, bounds.y, bounds.width / 2, bounds.height / 2},
        position, idx, depth++);
  }
  if (position[idx].x > bounds.x + bounds.width / 2 &&
      position[idx].y < bounds.y + bounds.height / 2) {
    // would have returned if it was not within bounds so we
    // only have to check where in the bounds it is
    // Upper right
    return insertParticle(tree, tree->nodes + node->firstChild + 1,
                          (Rectangle){bounds.x + bounds.width / 2, bounds.y,
                                      bounds.width / 2, bounds.height / 2},
                          position, idx, depth++);
  }
  if (position[idx].x < bounds.x + bounds.width / 2 &&
      position[idx].y > bounds.y + bounds.height / 2) {
    // would have returned if it was not within
    // bounds so we only have to check where in
    // the bounds it is Lower left
    return insertParticle(tree, tree->nodes + node->firstChild + 2,
                          (Rectangle){bounds.x, bounds.y + bounds.height / 2,
                                      bounds.width / 2, bounds.height / 2},
                          position, idx, depth++);
  }
  if (position[idx].x > bounds.x + bounds.width / 2 &&
      position[idx].y > bounds.y + bounds.height / 2) {
    // would have returned if it was not within
    // bounds so we only have to check where in
    // the bounds it is Lower left
    return insertParticle(tree, tree->nodes + node->firstChild + 3,
                          (Rectangle){bounds.x + bounds.width / 2,
                                      bounds.y + bounds.height / 2,
                                      bounds.width / 2, bounds.height / 2},
                          position, idx, depth++);
  }

  return false;
}

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

void UpdateParticles(Particles *particles, float dt) {
  for (int i = 0; i < POPULATION; i++) {
    CollideWithWalls(&(particles->position[i]), &(particles->velocity[i]), dt);
    CollideWithParticles(particles, i, dt);

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

  float deltaTime = GetFrameTime();

  if (deltaTime <= 0) {
    deltaTime = 1;
  }

  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();
    float subDT = deltaTime / SUB_STEPS;

    for (int i = 0; i < SUB_STEPS; i++) {
      UpdateParticles(&particles, subDT);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawFPS(10, 10);

    for (int i = 0; i < POPULATION; i++) {
      DrawCircleV(particles.position[i], particles.radius[i],
                  particles.color[i]);
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
