#ifndef _Vector2d_H
#define _Vector2d_H

#include "Packet.h"

#include <tuple>
#include <cmath>

class Vector2d {
public:
    Vector2d()
    : x_(0)
    , y_(0) {
    }
    
    Vector2d(float x, float y)
    : x_(x)
    , y_(y) {
    }

    Vector2d(const Vector2d& rhs)
    : x_(rhs.x_)
    , y_(rhs.y_) {
    }

    inline float getX() const {
        return x_;
    }

    inline float getY() const {
        return y_;
    }

    void reset() {
        x_ = 0.0f;
        y_ = 0.0f;
    }

    Vector2d operator -() const {
        return Vector2d(-x_, -y_);
    }

    Vector2d& operator =(const Vector2d& rhs) {
        x_ = rhs.x_;
        y_ = rhs.y_;
        return *this;
    }

    Vector2d& operator +=(const Vector2d& v) {
        x_ += v.x_;
        y_ += v.y_;
        return *this;
    }

    Vector2d& operator -=(const Vector2d& v) {
        x_ -= v.x_;
        y_ -= v.y_;
        return *this;
    }
    
    Vector2d& operator *=(float s) {
        x_ *= s;
        y_ *= s;
        return *this; 
    }

    Vector2d& operator /=(float s) {
        x_ /= s;
        y_ /= s;
        return *this; 
    }

    void rotate(float angle) {
        const auto cs = std::cos(angle);
        const auto sn = std::sin(angle);
        const auto px = x_ * cs - y_ * sn;
        const auto py = x_ * sn + y_ * cs;
        x_ = px;
        y_ = py;
    }

    float length() const {
        return std::sqrt((x_ * x_) + (y_ * y_));
    }

    Vector2d normal() const {
        const auto len = length();
        return Vector2d(x_ / len, y_ / len);
    }

    Vector2d perpendicular() const {
        return Vector2d(y_, -x_);
    }

    void write(Packet* packet) const {
        packet->write(x_);
        packet->write(y_);
    }

    void read(Packet* packet) {
        packet->read(x_);
        packet->read(y_);
    }

    friend bool operator ==(const Vector2d& a, const Vector2d& b) {
        return std::tie(a.x_, a.y_) == std::tie(b.x_, b.y_);
    }

    friend bool operator !=(const Vector2d& a, const Vector2d& b) {
        return !(a == b);
    }

private:
    float x_, y_;
};

static Vector2d operator +(const Vector2d& a, const Vector2d& b) {
    return Vector2d(a) += b;
}

static Vector2d operator -(const Vector2d& a, const Vector2d& b) {
    return Vector2d(a) -= b;
}

static Vector2d operator *(float s, const Vector2d& v) {
    return Vector2d(v) *= s;
}

static Vector2d operator /(float s, const Vector2d& v) {
    return Vector2d(v) /= s;
}

static float dot(const Vector2d& a, const Vector2d& b) {
    return (a.getX() * b.getX()) + (a.getY() * b.getY());
}

static float determinant(const Vector2d& a, const Vector2d& b) {
    return (a.getX() * b.getY()) + (a.getY() * b.getX());
}

static float cross(const Vector2d& a, const Vector2d& b) {
    return (a.getX() * b.getY()) - (a.getY() * b.getX());
}

static float angle(const Vector2d& a, const Vector2d& b) {
    return std::atan2(determinant(a, b), dot(a, b));
}

static bool intersect(const Vector2d& aa, const Vector2d& ab, const Vector2d& ba, const Vector2d& bb) {
    Vector2d p = aa;
    Vector2d r = ab - aa;
    Vector2d q = ba;
    Vector2d s = bb - ba;

    const auto t = cross((q - p), s) / cross(r, s);
    const auto u = cross((q - p), r) / cross(r, s);

    return (0.0 <= t && t <= 1.0) &&
           (0.0 <= u && u <= 1.0);
}

static Vector2d getIntersect(const Vector2d& aa, const Vector2d& ab, const Vector2d& ba, const Vector2d& bb) {
    const auto pX = (aa.getX()*ab.getY() - aa.getY()*ab.getX())*(ba.getX() - bb.getX()) - (ba.getX()*bb.getY() - ba.getY()*bb.getX())*(aa.getX() - ab.getX());
    const auto pY = (aa.getX()*ab.getY() - aa.getY()*ab.getX())*(ba.getY() - bb.getY()) - (ba.getX()*bb.getY() - ba.getY()*bb.getX())*(aa.getY() - ab.getY());
    
    const auto denominator = (aa.getX() - ab.getX())*(ba.getY() - bb.getY()) - (aa.getY() - ab.getY())*(ba.getX() - bb.getX());
    
    return Vector2d(pX / denominator, pY / denominator);    
}

static Vector2d lerp(const Vector2d& a, const Vector2d& b, float t) {
    return Vector2d(a + t * (b - a));
}

#endif  // _Vector2d_H
