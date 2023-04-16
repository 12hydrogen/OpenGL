#pragma once

#include <string>

namespace opengl
{
	class error
	{
	private:
		std::string info;
		std::string more;
	public:
		error(const std::string &info, const std::string &more = std::string()):
		info(info), more(more) {}

		std::string what() const
		{
			return info + (more.empty() ? "" : " Info: " + more + "\n") ;
		}
	};
}
