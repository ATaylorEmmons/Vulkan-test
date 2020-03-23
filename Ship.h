#pragma once

#include "Vulkan/include/vulkan.h"

#include "FillShapeMaterial.h"

#include "Input.h"
#include "Transform.h"
#include "Memory.h"

//Winding order = counter_clockwise
float TEST_SHIP_DATA[30] = {
    0.0f, .5f,    0.0f, 0.0f, 0.8f,
    -.5f, 0.0f,  .5f, 0.0f, 0.0f,
    0.0f, -.5f,   .5f, 0.0f, 0.0f,

    0.0f, .5f,    0.0f, 0.0f, 0.8f,
    0.0f, -.5f,   .5f, 0.0f, 0.0f,
    .5f, 0.0f,   .5f, 0.0f, 0.0f,
};

struct DrawRange {
    uint32_t firstVertex;
    uint32_t countVertex;

    uint32_t firstInstance;
    uint32_t countInstance;
};

struct Ship {
    Transform transform;
    MemoryRange memoryRange;

    float* attributes;
    FillShapeMaterial* material;

    float forwardSpeed = 5.0f;
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};
    Vector2 drag = {0.0f, 0.0f};

    float driftSpeed = 2.5f;
    Vector2 driftVelocity = {0.0f, 0.0f};
    Vector2 driftAcceleration = {0.0f, 0.0f};
    Vector2 driftDrag = {0.0f, 0.0f};

    Ship(FillShapeMaterial& mat, MemoryRange memRange, float* vertices) {
        this->attributes = vertices;
        this->memoryRange = memRange;

        material = &mat;
    }

    void Update(Input& input, GameTime time) {

        Vector2 forward = {0.0f, 0.0f};
        Vector2 left = {0.0f, 0.0f};
        Vector2 right = {0.0f, 0.0f};

        Vector2 posToMouse = (input.Mouse.worldPosition - transform.position);
        float theta = atan2f(posToMouse.y, posToMouse.x);


        transform.theta = theta -.5f*PI_F;
       

        if(input.W.IsDown && posToMouse.length() > 1.0f) {
           forward = {cos(theta), sin(theta)};
        }

        if(input.A.IsDown) {
            left = {sin(theta), -cos(theta)};
        }

         if(input.D.IsDown) {
            right = {-sin(theta), cos(theta)};
        }

        this->velocity = ((float)time.dt)*this->acceleration + this->velocity;
        this->acceleration = (forwardSpeed)*((float)time.dt)*forward - this->drag;
        this->drag = 1.0f * this->velocity;
        

        this->driftVelocity = ((float)time.dt)*this->driftAcceleration + this->driftVelocity;
        this->driftAcceleration = (driftSpeed)*((float)time.dt)*(left + right) - this->driftDrag;
        this->driftDrag = 1.0f * this->driftVelocity;
    
        Vector2 shift = this->velocity + this->driftVelocity;
        transform.Translate(shift);

        transform.Update();
    }


    void Draw(VkCommandBuffer& commandBuffer) {

        DrawRange range;
        range.firstVertex = 0;
        range.countVertex = 6;
        range.firstInstance = 0;
        range.countInstance = 1;

        vkCmdPushConstants(
					commandBuffer, 
					material->layout, 
					VK_SHADER_STAGE_VERTEX_BIT, 
					0, 
					MAX_PUSHCONSTANT_SIZE, 
					transform.pack());	

		vkCmdDraw(commandBuffer, range.countVertex, range.countInstance, range.firstVertex, range.firstInstance);
    }
};