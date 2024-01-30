#include<raylib.h>
#include<raymath.h>
#include<stdlib.h>
#include<stdio.h>

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

void CollideWithWalls(Vector2 *position, Vector2 *velocity, float dt){
	if(position->x + velocity->x * dt < 0 || position->x + velocity->x * dt > SCREEN_WIDTH){
		velocity->x *= -1;
	}
	if(position->y + velocity->y * dt < 0 || position->y + velocity->y * dt > SCREEN_HEIGHT){
		velocity->y *= -1;
	}
}

void CollideWithParticles(){

}

void UpdateParticles(Particles *particles, float dt){
	for(int i = 0; i < POPULATION; i++){
		CollideWithWalls(&(particles->position[i]), &(particles->velocity[i]), dt);
		CollideWithParticles();

		particles->position[i] = Vector2Add(particles->position[i], Vector2Scale(particles->velocity[i], dt));

	}
}

int main(void){
	const int maxVelocity = 10;
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Here We Go again!");
	SetTargetFPS(30);

	Particles particles;

	particles.position = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
	particles.velocity = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);
	particles.acceleration = (Vector2 *)malloc(sizeof(Vector2) * POPULATION);

	for(int i = 0; i < POPULATION; i++){
		particles.position[i] = (Vector2){rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT};
		particles.velocity[i] = (Vector2){rand() % maxVelocity - maxVelocity / 2, rand() % maxVelocity - maxVelocity / 2};
		particles.acceleration[i] = (Vector2){0, 0};
		particles.mass = 1;
		particles.radius = 5;	
	}

	float deltaTime = GetFrameTime();

	if(deltaTime <= 0){
		deltaTime = 1;
	}

	while(!WindowShouldClose()){
		
		UpdateParticles(&particles, deltaTime);

		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawFPS(10, 10);

			for (int i = 0; i < POPULATION; i++){
				DrawCircleV(particles.position[i], particles.radius, BLUE);
			}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}
