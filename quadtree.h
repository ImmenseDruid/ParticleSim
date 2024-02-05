#ifndef MY_TREE
#define MY_TREE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AABB {
  int x;
  int y;
  int w;
  int h;
} aabb_t;

typedef struct node {
  int idx;         // current node idx, cuz i got annoyed with not having it
  int childIdx;    // starting idx for the children of the node
  int childrenNum; // number of children
  int elementIdx;  // the idx for the element
  aabb_t rect;     // rectangle for bounds
} tree_node_t;

typedef struct root {
  tree_node_t *nodes;
  int nodeNum;
  int nodeCapacity;
  int depth;
  aabb_t bounds;
} tree_root_t;

bool CheckCollisionPointAABB(float x, float y, aabb_t box);
void InitNode(tree_node_t *node, int idx, aabb_t rect);
void SplitNode(tree_root_t *root, tree_node_t *node);
void InsertElement(tree_root_t *root, tree_node_t *node, float x, float y,
                   int idx, int depth);
void InsertElementTree(tree_root_t *root, float x, float y, int idx);
void ResetTree(tree_root_t *root);
void CleanUpTree(tree_root_t *root) { free(root->nodes); }

int GetElementsInRect(tree_root_t *root, aabb_t rect, int *result);

#endif

bool CheckCollisionPointAABB(float x, float y, aabb_t box) {
  if (x < box.x || y < box.y || x > box.x + box.w || y > box.y + box.h) {
    return false;
  }
  return true;
}

bool CheckCollisionAABBS(aabb_t b1, aabb_t b2) {
  if (b1.x > b2.x + b2.w || b1.y > b2.y + b2.h || b1.x + b1.w < b2.x ||
      b1.y + b1.h < b2.y) {
    return false;
  }
  return true;
}

void InitTree(tree_root_t *root, int width, int height) {
  root->depth = 10;
  root->nodes = (tree_node_t *)calloc(40, sizeof(tree_node_t));
  root->bounds = (aabb_t){0, 0, width, height};
  root->nodeNum = 1;
  root->nodeCapacity = 40;

  InitNode(&root->nodes[0], 0, root->bounds);
}

void ResetTree(tree_root_t *root) {
  root->nodeNum = 1;
  InitNode(&root->nodes[0], 0, root->bounds);
}

void InitNode(tree_node_t *node, int idx, aabb_t rect) {
  node->idx = idx;
  node->rect = rect;
  node->childIdx = -1;
  node->elementIdx = -1;
  node->childrenNum = -1;
}

void SplitNode(tree_root_t *root, tree_node_t *node) {
  node->childIdx = root->nodeNum;
  int x = node->rect.x;
  int y = node->rect.y;
  int w = node->rect.w >> 1;
  int h = node->rect.h >> 1; // hmmmmm
  InitNode(root->nodes + node->childIdx, node->childIdx, (aabb_t){x, y, w, h});
  InitNode(root->nodes + node->childIdx + 1, node->childIdx + 1,
           (aabb_t){x + w, y, w, h});
  InitNode(root->nodes + node->childIdx + 2, node->childIdx + 2,
           (aabb_t){x, y + h, w, h});
  InitNode(root->nodes + node->childIdx + 3, node->childIdx + 3,
           (aabb_t){x + w, y + h, w, h});
  node->childrenNum = 4;
  root->nodeNum += 4;
}

void InsertElement(tree_root_t *root, tree_node_t *node, float x, float y,
                   int idx, int depth) {

  if (node->elementIdx == -1) {
    // printf("Adding element : %i to node %i \n", idx, node->idx);
    node->elementIdx = idx;
    return;
  } // else if (node->childIdx > root->nodeNum) {
  //   printf("Somehow the child idx is bigger than the amount of nodes\n");

  // } else if (node->childIdx < -1) {
  //   printf("Somehow the child idx is smaller than -1\n");
  // }
  else {
    if (depth > root->depth) {
      return;
    }

    if (node->childrenNum == -1) {
      // Sub divide node
      if (root->nodeNum + 4 > root->nodeCapacity) {
        root->nodeCapacity <<= 1;
        root->nodes = (tree_node_t *)realloc(
            root->nodes, root->nodeCapacity * sizeof(tree_node_t));
        memset(root->nodes + root->nodeNum, 0,
               sizeof(tree_node_t) * (root->nodeCapacity >> 1));
      }
      if (node->rect.w > 5 && node->rect.h > 5) {
        SplitNode(root, node);
      }
    }

    for (int j = 0; j < node->childrenNum; j++) {
      if (node->childIdx + j > root->nodeNum) {
        printf("Hi I broke\n");
        break;
      }
      if (CheckCollisionPointAABB(x, y, root->nodes[node->childIdx + j].rect)) {
        InsertElement(root, &root->nodes[node->childIdx + j], x, y, idx,
                      depth + 1);
      }
    }
  }
}

void InsertElementTree(tree_root_t *root, float x, float y, int idx) {
  if (CheckCollisionPointAABB(x, y, root->nodes[0].rect)) {
    InsertElement(root, root->nodes, x, y, idx, 0);
  }
}

int GetElementsInRect(tree_root_t *root, aabb_t rect, int *result) {
  int *node_queue = (int *)calloc(sizeof(int), root->nodeNum);
  memset(node_queue, -1, sizeof(int) * root->nodeNum);
  // Maximum amount of nodes
  int nodes_size = 0;
  int nodes_capacity = root->nodeNum;

  node_queue[nodes_size] = 0;
  nodes_size++;

  memset(result, -1, sizeof(int) * root->nodeNum);
  int results_size = 0;

  while (nodes_size > 0) { // create breadth first search

    if (CheckCollisionAABBS(rect, root->nodes[node_queue[0]].rect)) {
      // add element to result
      if (root->nodes[node_queue[0]].elementIdx != -1) {
        result[results_size] = root->nodes[node_queue[0]].elementIdx;
        results_size++;
      }
      // add children to queue
      for (int i = 0; i < root->nodes[node_queue[0]].childrenNum; i++) {
        node_queue[nodes_size] = root->nodes[node_queue[0]].childIdx + i;
        nodes_size++;
      }
    }

    // shift the queue
    for (int i = 0; i < nodes_size; i++) {
      node_queue[i] = node_queue[i + 1];
    }
    node_queue[nodes_size] = -1;
    nodes_size--;
  }

  return results_size;
}

