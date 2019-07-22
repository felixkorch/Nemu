#pragma once

class GLPrimitive {
protected:
	unsigned int handle;
public:

	GLPrimitive()
		: handle(0)
	{}

	GLPrimitive(GLPrimitive& other) = delete;
	GLPrimitive& operator=(GLPrimitive& other) = delete;

	GLPrimitive(GLPrimitive&& other)
		: handle(other.handle)
	{
		other.handle = 0;
	}

	bool Valid() { return handle != 0; }
};