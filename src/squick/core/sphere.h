

#ifndef SQUICK_SPHERE_H
#define SQUICK_SPHERE_H

#include <iostream>
#include <string>
#include "space_node.h"

class Line;
class Box;
class Plane;
class Vector3;

class Sphere : public SpaceNode
{
public:
    Sphere()
    {
        mvCenter = Vector3::Zero();
        mfRadius = 0.0f;
    }

    Sphere(const Vector3&  vCenter, float fRadius)
    {
        this->mvCenter = vCenter;
        this->mfRadius = fRadius;
    }

    virtual ~Sphere() {}

    bool operator==(const Sphere& other) const
    {
        return (mvCenter == other.mvCenter) && (mfRadius == other.mfRadius);
    }

    bool operator!=(const Sphere& other) const
    {
        return !((mvCenter == other.mvCenter) && (mfRadius == other.mfRadius));
    }

    void operator*=(const float fScale)
    {
    }

    void operator/=(const float fScale)
    {
    }

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
    Vector3        mvCenter;
    float            mfRadius;

};

#endif