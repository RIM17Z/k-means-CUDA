#ifndef TEXTHELPER_H
#define TEXTHELPER_H
#include <iostream>
#include <sstream>

namespace KMeans {
	template <typename T> std::string to_string(T value)
	{
		std::ostringstream os;
		os << value;
		return os.str();
	}

	void drawText(const char *str, int x, int y);
	void drawString(const char *str, int x, int y, float color[4], void *font);
	void drawString3D(const char *str, float pos[3], float color[4], void *font);
} // namespace KMeans
#endif