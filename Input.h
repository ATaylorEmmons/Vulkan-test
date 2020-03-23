
#pragma once
#include "Transform.h"

/* 
  _______ ____  _____   ____  
 |__   __/ __ \|  __ \ / __ \ 
    | | | |  | | |  | | |  | |
    | | | |  | | |  | | |  | |
    | | | |__| | |__| | |__| |
    |_|  \____/|_____/ \____/ 
 _______________________________________
 [                                     ]
---------------------------------------                                               
    -Build the input buttons at build as an asset
        with a tool

*/

struct InputPress {
	bool IsDown;
	bool wasDown;
	int repeatCount;


	void updatePreviousState() {
		this->wasDown = this->IsDown;
	}

	void update() {
		
		if (this->wasDown && this->IsDown) {
			this->repeatCount++;
		}
		else {
			this->repeatCount = 0;
		}
	}
};

struct MouseInput {
	InputPress Left;
	InputPress Middle;
	InputPress Right;

	float mouseWheel;

	Vector2 screenPosition;
	Vector2 worldPosition;

};


struct Input {
	
	InputPress W = {0};
	InputPress S = {0};
	InputPress A = {0};
	InputPress D = {0};

	MouseInput Mouse = {0};

	void update() {
		W.updatePreviousState();
		S.updatePreviousState();
		A.updatePreviousState();
		D.updatePreviousState();

		Mouse.Left.updatePreviousState();
		Mouse.Middle.updatePreviousState();
		Mouse.Right.updatePreviousState();

		glfwPollEvents();

		W.update();
		S.update();
		A.update();
		D.update();

		Mouse.Left.update();
		Mouse.Middle.update();
		Mouse.Right.update();

	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Input* input = (Input*)glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_W) {
			if (action == GLFW_PRESS) {
				input->W.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->W.IsDown = false;
			}
		} 

		if (key == GLFW_KEY_S) {
			if (action == GLFW_PRESS) {
				input->S.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->S.IsDown = false;
			}
		}

		if (key == GLFW_KEY_A) {
			if (action == GLFW_PRESS) {
				input->A.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->A.IsDown = false;
			}
		} 

		if (key == GLFW_KEY_D) {
			if (action == GLFW_PRESS) {
				input->D.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->D.IsDown = false;
			}
		}
	}

	static void mouse_pos_callback(GLFWwindow* window, double posX, double posY) {
		Input* input = (Input*)glfwGetWindowUserPointer(window);
		input->Mouse.screenPosition.x = (float)posX;
		input->Mouse.screenPosition.y = (float)posY;
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		Input* input = (Input*)glfwGetWindowUserPointer(window);

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				input->Mouse.Right.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->Mouse.Right.IsDown = false;
			}
		} 

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS) {
				input->Mouse.Middle.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->Mouse.Middle.IsDown = false;
			}
		} 

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				input->Mouse.Left.IsDown = true;
			}
			if (action == GLFW_RELEASE) {
				input->Mouse.Left.IsDown = false;
			}
		} 

	}
};