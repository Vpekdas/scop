#include "math/vector.hpp"

std::ostream& operator<<(std::ostream& out, const Vector2& vec2)
{
    return out << vec2.u << " " << vec2.v << "\n";
}