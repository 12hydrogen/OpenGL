#pragma once

#include <string>

namespace opengl
{
	class error
	{
	private:
		std::string info;
	public:
		error(const std::string &info):
		info(info) {}

		std::string what() const
		{
			return info;
		}
	};
}
