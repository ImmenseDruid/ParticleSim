# This is a Particle Simulation

This is a Temporary Readme file until I create an actual readme

## Particles
Particles are represented as a position, a velocity, an acceleration, a mass and a radius. they also have their own color to distinguish some particles from others for ease of verification of correct simulation.


## Quadtree
The quad tree as it stands is quite the work in progress, it is slightly more "advanced" than the typical quadtree implementation but still not quite as advanced as I would like to see.
see the link below

Quadtree's still are used for collision between particles, and sometimes a box around a particle to test how many particles are being checked for collision.
- red and blue are regular particles
- black are being checked to see if the particle is inside the rectangle or AABB or (axis aligned bounding box)
- yellow are being checked and are inside the rectangle or AABB

https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
