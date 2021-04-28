#ifndef UTILS_HPP
#define UTILS_HPP
//#include <string>
#include <sstream>

template <typename... Args>
std::string format(Args... args)
{
    std::ostringstream oss;

    ((oss << args), ...);

    return oss.str();
}

#endif