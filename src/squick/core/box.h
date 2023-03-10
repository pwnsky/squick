

#ifndef SQUICK_BOX_H
#define SQUICK_BOX_H

#include <iostream>
#include <string>

class Line;
class Box;
class Plane;
class Vector3;

class Box
{
    Vector3 GetCorner(int i) const
    {
        if(i < 8)
        {
            return mvCorner[i];
        }
    }

    Vector3 Corner(int i) const
    {
        return GetCorner(i);
    }

private:
    Vector3 mvCorner[8];

    Vector3 mvRadius[3];

    Vector3 mvCenter;
};

#endif