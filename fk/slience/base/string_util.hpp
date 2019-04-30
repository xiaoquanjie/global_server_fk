#ifndef M_BASE_STRING_UTIL_INCLUDE
#define M_BASE_STRING_UTIL_INCLUDE

#include "slience/base/config.hpp"
#include <string>
#include <vector>
#include <sstream>

M_BASE_NAMESPACE_BEGIN

class StringUtil {
public:
	static std::string basename(const std::string& path);

	static std::string remove_from_end(const std::string& src, const std::string& tar);

	static std::string directory(const std::string& path);

	template<typename T>
	static void Split(const std::string source, const std::string& separator, std::vector<T>& array) {
		array.clear();
		std::string::size_type start = 0;
		while (true) {
			std::string::size_type pos = source.find_first_of(separator, start);
			if (pos == std::string::npos) {
				std::string sub = source.substr(start, source.size());
				if (!sub.empty()) {
					std::istringstream iss(sub);
					T t;
					iss >> t;
					array.push_back(t);
				}
				break;
			}

			std::string sub = source.substr(start, pos - start);
			start = pos + separator.size();
			if (!sub.empty()) {
				std::istringstream iss(sub);
				T t;
				iss >> t;
				array.push_back(t);
			}
			else {
				T t;
				array.push_back(t);
			}
		}
	}
};

M_BASE_NAMESPACE_END
#endif