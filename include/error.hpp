#pragma once

#include <string>
#include <exception>

namespace opengl
{
	class error: public std::exception
	{
	private:
		std::string info;
	public:
		error(const std::string &info, const std::string &more = std::string())
		{
			this->info = info + (more.empty() ? "" : " Info: " + more + "\n");
		}

		const char* what() const noexcept
		{
			return info.c_str();
		}
	};
}
