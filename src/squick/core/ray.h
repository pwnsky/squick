

#ifndef SQUICK_RAY_H
#define SQUICK_RAY_H

#include <iostream>
#include <string>
#include "vector3.h"

class Line;
class Box;
class Plane;
class Sphere;
class Vector3;

class Ray
{
public:
    bool Contains(const Vector3& v)
    {
        return false;
    }

    bool CollideWithLine(const Line& l)
    {
        return false;
    }
    
    bool CollideWithPlane(const Plane& p)
    {
        return false;
    }

    bool CollideWithBox(const Box& b)
    {
        return false;
    }

    bool CollideWithSphere(const Sphere& s)
    {
        return false;
    }


private:
    Line* m_pLine;

};

#endif