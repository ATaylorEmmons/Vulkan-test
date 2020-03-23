#pragma once

#include "LinearAlgebra.h"

struct View {
    static const uint32_t SIZE = sizeof(float)*8;

    Vector2 position;
    float zoom;

    Vector2 resolution;
    float unitRatio;

    float data[8];

    View() {
        position = Vector2::Origin();
        zoom = 1.0f;

        resolution = {0.0f, 0.0f};
        unitRatio = 1.0f;
    }

    Vector2 screenToWorld(Vector2 screenPos) {

        Vector2 ret;
        ret.x = (screenPos.x - .5f*resolution.x)*(1.0f/unitRatio);
        ret.y = -(screenPos.y - .5f*resolution.y)*(1.0f/unitRatio);

        return ret;
    }

    void* pack() {
        data[0] = position.x;
        data[1] = position.y;
        data[2] = zoom;
        data[3] = unitRatio;
        data[4] = resolution.x;
        data[5] = resolution.y;
        data[6] = 0.0f;
        data[7] = 0.0f;

        return (void*)data;
    }
};

struct Transform {
    
    float scale;
    float theta;
    Vector2 position;
    Vector2 origin;
    Vector3 tint;
    Mat3 matrix;
    float data[16];

    Transform() {
        matrix = Mat3::I();
        scale = 1.0f;
        theta = 0.0f;
        origin = {0.0f, 0.0};
        position = {0.0f, 0.0f};
        tint = {1.0f, 1.0f, 1.0f};
    }

    /* TODO: Transform methods 

    */

    void Translate(Vector2 shift) {
        this->position += shift;
    }

    void Rotate(float a) {
        this->theta += a;
    }

    void Update() {
        matrix = Mat3::I();

        Mat3 originMat = {1.0f, 0.0f, origin.x, 
                          0.0f, 1.0f, origin.y,
                          0.0f, 0.0f, 1.0f};

        Mat3 scaleMat = {scale, 0.0f, 0.0f, 
                          0.0f, scale, 0.0f,
                          0.0f, 0.0f, 1.0f};

        Mat3 rotateMat = {cosf(-theta), sinf(-theta), 0.0f, 
                          -sinf(-theta), cosf(-theta), 0.0f,
                           0.0f, 0.0f, 1.0f};

        Mat3 translateMat = {1.0f, 0.0f, position.x, 
                             0.0f, 1.0f, position.y,
                             0.0f, 0.0f, 1.0f};
       
       matrix = translateMat*rotateMat*scaleMat*originMat;
    }

    void* pack() {

        //Pack the transform matrix
        for(uint32_t i = 0; i < 3; i++) {
            for(uint32_t j = 0; j < 3; j++) {
                data[4*i + j] = matrix[3*i + j];
            }
        }

        data[3] = tint.x;
        data[7] = tint.y;
        data[11] = tint.z;
        

        return (void*)(data);
    }
};