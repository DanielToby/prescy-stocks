#pragma once

#include <Engine/prescy_engine_platform.hpp>

#include <stdexcept>


//#pragma warning(push)
//#pragma warning(disable : 4275)

namespace prescy {
struct Prescy_Engine PrescyException : public std::runtime_error {
    explicit PrescyException(const std::string& msg);
};
}

//#pragma warning(pop)
