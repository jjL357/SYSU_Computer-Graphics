#pragma once

#include <GL\glew.h>
#include <iostream>
#include <string>
#include "stb_image.h"

using namespace std;

class CTexture {
public:
	GLuint textureID;

	CTexture();

	~CTexture();

	void loadTexture(string path, bool constrain = false);
};
