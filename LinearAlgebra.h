#pragma once

#include <cmath>
#include "Vulkan/include/vulkan.h"


#define PI_F 3.1415962f

struct Vector2 {
    float x, y;

     Vector2 operator+(Vector2 a) {
        Vector2 ret;
        ret.x = this->x + a.x;
        ret.y = this->y + a.y;

        return ret;
    }

    Vector2& operator+=(Vector2 a) {
        this->x += a.x;
        this->y += a.y;

        return *this;
    }

    Vector2 operator-(Vector2 a) {
        Vector2 ret;
        ret.x = this->x - a.x;
        ret.y = this->y - a.y;
        return ret;
    }

    Vector2& operator-=(Vector2 a) {
        Vector2 ret;
        this->x -= a.x;
        this->y -= a.y;

        return *this;
    }

    Vector2 operator*(float a) {
        return {x*a, y*a};
    }

    Vector2& operator*=(float a) {
        Vector2 ret;
        this->x *= a;
        this->y *= a;

        return *this;
    }

    Vector2 hadmard(Vector2 a) {
        Vector2 ret;
        ret.x = this->x * a.x;
        ret.y = this->y * a.y;
    }

    float dot(Vector2 a) {
        return this->x*a.x + this->y*a.y;
    }

    float length() {
        return sqrtf(this->x*this->x + this->y*this->y);
    }

    void Normalize() {
        (*this) *= (1.0f/length());
    }

    Vector2 normalized() {
        Vector2 ret = (*this)*(1.0f/length());
        return ret;
    }

    static Vector2 Origin() {
        return {0.0f, 0.0f};
    }
};

Vector2 operator*(Vector2 v, float a) {
    Vector2 ret;
    ret.x = a*v.x;
    ret.y = a*v.y;

    return ret;
}

Vector2 operator*(float a, Vector2 v) {
    Vector2 ret;
    ret.x = a*v.x;
    ret.y = a*v.y;

    return ret;
}


struct Vector3 {
    float x, y, z;
};

struct Mat3 {
    float indices[9];

    float& operator[](uint32_t index) {
        return this->indices[index];
    }

    static Mat3 I() {
        return Mat3{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    }

    Mat3 operator*(Mat3 b) {
        Mat3 ret;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
               ret[3*i + j] = 0;
                for (int k = 0; k < 3; k++)
                {
                    ret[3*i + j] += indices[3*i + k] * b[3*k + j];
                }
            }
        }
        
        return ret;
    }

};

Vector2 vkExtent2DToVector2(VkExtent2D extent) {
    Vector2 ret;
    ret.x = (float)extent.width;
    ret.y = (float)extent.height;
    
    return ret;
}

 VkExtent2D Vector2TovkExtent2D(Vector2 a) {

    VkExtent2D ret;
    ret.width = (uint32_t)a.x;
    ret.height = (uint32_t)a.y;
    
    return ret;
}