#include "TextHelper.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/freeglut.h>

namespace KMeans{
	void drawText(const char* str, int x, int y){
		float color[4] = { 1, 1, 1, 1 };
		void *font = GLUT_BITMAP_8_BY_13;
		std::stringstream ss;
		ss << std::fixed << std::setprecision(3);
		ss << str << std::ends;  // add 0(ends) at the end
		drawString(ss.str().c_str(), x, y, color, font);
	}

	void drawString(const char *str, int x, int y, float color[4], void *font)
	{
		glColor4fv(color);          // set text color
		glRasterPos2i(x, y);        // place text position
		// loop all characters in the string
		while (*str)
		{
			glutBitmapCharacter(font, *str);
			++str;
		}
	}

	void drawString3D(const char *str, float pos[3], float color[4], void *font)
	{
		glColor4fv(color);          // set text color
		glRasterPos3fv(pos);        // place text position
		// loop all characters in the string
		while (*str)
		{
			glutBitmapCharacter(font, *str);
			++str;
		}
	}
} // namespace KMeans
