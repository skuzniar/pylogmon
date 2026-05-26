#ifndef Types_dot_h
#define Types_dot_h

#include <ostream>
#include <iostream>

namespace O3 {

//-----------------------------------------------------------------------------------------------------------------
// Notification types
//-----------------------------------------------------------------------------------------------------------------
enum class Event : uint8_t
{
    Write,
    Delete,
    Extend,
    Attribute
};

inline std::ostream& operator<<(std::ostream& s, Event o)
{
    switch(o)
    {
        // clang-format off
        case Event::Write:     s << "Write";     break;
        case Event::Delete:    s << "Delete";    break;
        case Event::Extend:    s << "Extend";    break;
        case Event::Attribute: s << "Attribute"; break;
        default: s << std::to_string(static_cast<std::underlying_type_t<Event>>(o)) + "(Invalid Event)"; break;
        // clang-format on
    };
    return s;
}

} // namespace O3

#endif
