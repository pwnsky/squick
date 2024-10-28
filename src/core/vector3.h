#pragma once

#include "vector2.h"
#include <iostream>
#include <string>

class Vector3 {
  private:
    float x, y, z;
    void StartData() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

  public:
    // construction
    Vector3() { StartData(); }

    Vector3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector3(const Vector2 &v, float z) {
        this->x = v.X();
        this->y = v.Y();
        this->z = z;
    }

    Vector3(float coordinate[3]) {
        this->x = coordinate[0];
        this->y = coordinate[1];
        this->z = coordinate[2];
    }

    Vector3(double coordinate[3]) {
        this->x = (float)coordinate[0];
        this->y = (float)coordinate[1];
        this->z = (float)coordinate[2];
    }

    Vector3(const Vector3 &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }
    Vector3(const char *v, const int len) { FromString(std::string(v, len)); }
    Vector3(const std::string &v) { FromString(v); }
    //----------------------------------------------------------------------------
    bool operator<(const Vector3 &v) const { return this->Length() < v.Length(); }

    bool operator>(const Vector3 &v) const { return this->Length() > v.Length(); }

    Vector3 &operator=(const Vector3 &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;

        return *this;
    }

    bool operator==(const Vector3 &v) const { return std::abs(this->x - v.x) < 0.001f && std::abs(this->y - v.y) < 0.001f && std::abs(this->z - v.z) < 0.001f; }

    bool operator!=(const Vector3 &v) const {
        return std::abs(this->x - v.x) >= 0.001f || std::abs(this->y - v.y) >= 0.001f || std::abs(this->z - v.z) >= 0.001f;
    }

    //----------------------------------------------------------------------------
    // Arithmetic Operations
    Vector3 operator+(const Vector3 &v) const {
        Vector3 xV;

        xV.x = this->x + v.x;
        xV.y = this->y + v.y;
        xV.z = this->z + v.z;
        return xV;
    }

    Vector3 operator-(const Vector3 &v) const {
        Vector3 xV;

        xV.x = this->x - v.x;
        xV.y = this->y - v.y;
        xV.z = this->z - v.z;
        return xV;
    }

    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }

    Vector3 operator/(float s) const {
        if (std::abs(s) > 0.001f) {
            return Vector3(x / s, y / s, z / s);
        }

        return Zero();
    }

    //----------------------------------------------------------------------------
    // Arithmetic Updates
    Vector3 &operator+=(const Vector3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3 &operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Vector3 operator/=(float s) {
        // if (std::abs(s) > 0.001f)
        { return Vector3(x / s, y / s, z / s); }

        // return Zero();
    }

    //----------------------------------------------------------------------------
    float X() const { return this->x; }

    float Y() const { return this->y; }

    float Z() const { return this->z; }

    void SetX(float x) { this->x = x; }

    void SetY(float y) { this->y = y; }

    void SetZ(float z) { this->z = z; }

    //----------------------------------------------------------------------------
    bool IsZero() const { return std::abs(x) < 0.001f && std::abs(y) < 0.001f && std::abs(z) < 0.001f; }
    //----------------------------------------------------------------------------
    inline float SquaredMagnitude() const { return x * x + y * y + z * z; }

    //----------------------------------------------------------------------------
    inline float SquaredLength() const { return SquaredMagnitude(); }

    //----------------------------------------------------------------------------
    inline float Magnitude() const { return sqrtf(x * x + y * y + z * z); }

    //----------------------------------------------------------------------------
    inline float Length() const { return Magnitude(); }

    //----------------------------------------------------------------------------
    inline Vector3 Direction() const {
        if (this->IsZero()) {
            return Zero();
        }

        float lenSquared = SquaredMagnitude();
        float invSqrt = 1.0f / sqrtf(lenSquared);
        return Vector3(x * invSqrt, y * invSqrt, z * invSqrt);
    }

    //----------------------------------------------------------------------------
    inline Vector3 Normalized() const { return Direction(); }

    //----------------------------------------------------------------------------
    float Distance(const Vector3 &v) const {
        Vector3 vX = *this - v;
        return vX.Length();
    }

    //----------------------------------------------------------------------------
    bool FromString(const std::string &value) {
        std::vector<std::string> values;
        Split(value, values, ",");
        if (values.size() != 3) {
            return false;
        }
        try {
            x = lexical_cast<float>(values.at(0));
            y = lexical_cast<float>(values.at(1));
            z = lexical_cast<float>(values.at(2));

            return true;
        } catch (const std::exception &) {
        }

        return false;
    }

    //----------------------------------------------------------------------------
    std::string ToString() const { return lexical_cast<std::string>(x) + "," + lexical_cast<std::string>(y) + "," + lexical_cast<std::string>(z); }

    // Special values.
    inline static const Vector3 &Zero() {
        static Vector3 v(0, 0, 0);
        return v;
    }
    inline static const Vector3 &One() {
        static Vector3 v(1, 1, 1);
        return v;
    }
    inline static const Vector3 &UnitX() {
        static Vector3 v(1, 0, 0);
        return v;
    }
    inline static const Vector3 &UnitY() {
        static Vector3 v(0, 1, 0);
        return v;
    }
    inline static const Vector3 &UnitZ() {
        static Vector3 v(0, 0, 1);
        return v;
    }

    Vector2 xx() const { return Vector2(this->x, this->x); }

    Vector2 yx() const { return Vector2(this->y, this->x); }

    Vector2 zx() const { return Vector2(this->z, this->x); }

    Vector2 xy() const { return Vector2(this->x, this->y); }

    Vector2 yy() const { return Vector2(this->y, this->y); }

    Vector2 zy() const { return Vector2(this->z, this->y); }

    Vector2 xz() const { return Vector2(this->x, this->z); }

    Vector2 yz() const { return Vector2(this->y, this->z); }

    Vector2 zz() const { return Vector2(this->z, this->z); }

    bool Split(const std::string &str, std::vector<std::string> &result, std::string delim) {
        if (str.empty()) {
            return false;
        }

        std::string tmp;
        size_t pos_begin = str.find_first_not_of(delim);
        size_t pos = 0;
        while (pos_begin != std::string::npos) {
            pos = str.find(delim, pos_begin);
            if (pos != std::string::npos) {
                tmp = str.substr(pos_begin, pos - pos_begin);
                pos_begin = pos + delim.length();
            } else {
                tmp = str.substr(pos_begin);
                pos_begin = pos;
            }

            if (!tmp.empty()) {
                result.push_back(tmp);
                tmp.clear();
            }
        }
        return true;
    }
};