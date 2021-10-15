#include <Engine/exception.hpp>

#include <src/log.hpp>

namespace prescyengine {

PrescyException::PrescyException(const std::string& msg) :
    std::runtime_error(msg) {
    E_ERROR("PrescyException: {0}", msg);
};

}
