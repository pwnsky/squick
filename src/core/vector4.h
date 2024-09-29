#pragma once

class Vector4 {
  private:
    float x, y, z, w;

  public:
    Vector4() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }
    Vector4 &operator=(const Vector4 &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;

        return *this;
    }

    float X() const { return this->x; }

    float Y() const { return this->y; }

    float Z() const { return this->z; }
    float W() const { return this->w; }

    void SetX(float x) { this->x = x; }

    void SetY(float y) { this->y = y; }

    void SetZ(float z) { this->z = z; }
    void SetW(float z) { this->w = w; }

    std::string ToString() const {
        return lexical_cast<std::string>(x) + "," + lexical_cast<std::string>(y) + "," + lexical_cast<std::string>(z) + "," + lexical_cast<std::string>(w);
    }
};