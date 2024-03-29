#pragma once

#include <ori/simcars/agent/constant_interface.hpp>
#include <ori/simcars/agent/valueless_constant_abstract.hpp>

#include <sstream>

namespace ori
{
namespace simcars
{
namespace agent
{

template <typename T>
class AConstant : public virtual IConstant<T>, public virtual AValuelessConstant
{
public:
    IValuelessConstant* valueless_constant_shallow_copy() const override
    {
        return this->constant_shallow_copy();
    }

    std::string get_value_as_string() const override
    {
        std::stringstream string_stream;
        string_stream << this->get_value();
        std::string value_as_string = string_stream.str();

        std::stringstream updated_string_stream;

        size_t i = 0, j;
        while (true)
        {
            j = value_as_string.find('\n', i);

            if (j == i || (j == i + 1 && value_as_string[i] == ','))
            {
                i = j + 1;
                continue;
            }

            std::string line;
            if (j != std::string::npos && value_as_string[j - 1] == ',')
            {
                line = value_as_string.substr(i, (j - i) - 1);
            }
            else
            {
                line = value_as_string.substr(i, j - i);
            }

            updated_string_stream << line;

            if (j == std::string::npos)
            {
                break;
            }
            else
            {
                updated_string_stream << " ";
            }

            i = j + 1;
        }

        return updated_string_stream.str();
    }
};

}
}
}
