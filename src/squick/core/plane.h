

#ifndef SQUICK_PLANE_H
#define SQUICK_PLANE_H

#include <iostream>
#include <string>

class Line;
class Box;
class Vector3;

class Plane
{

private:
    Vector3 mvCorner[4];

    Vector3 mvRadius[2];

    Vector3 mvCenter;
};

#endif