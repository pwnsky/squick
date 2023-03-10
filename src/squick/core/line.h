
#ifndef SQUICK_Line_H
#define SQUICK_Line_H

#include <iostream>
#include <string>

class Line;
class Box;
class Plane;
class Vector3;


class Line
{
public:
    Line(const Vector3& vPointStart, const Vector3& vDirection, const int length)
    {
    }
    
    Line(const Vector3& vPointStart, const Vector3& vPointEnd)
    {
    }
   
    Line(const Vector3& vPoint, const Vector3& vDirection, const int length)
    {
    }

    Vector3 PointStart() const
    {
        return Vector3::Zero();
    }

    Vector3 Direction() const
    {
        return Vector3::Zero();
    }

    Vector3 PointEnd() const
    {
        return Vector3::Zero();
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
    Vector3 mvStartPoint;
    Vector3 mvEndPoint;


	
};

#endif