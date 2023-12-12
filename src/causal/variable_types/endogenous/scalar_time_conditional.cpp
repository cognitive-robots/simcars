
#include <ori/simcars/causal/variable_types/endogenous/scalar_time_conditional.hpp>

#include <ori/simcars/causal/variable_context.hpp>

namespace ori
{
namespace simcars
{
namespace causal
{

FP_DATA_TYPE ScalarTimeConditionalVariable::get_value() const
{
    return get_other_parent()->get_value() > VariableContext::get_current_time() ?
                get_endogenous_parent_1()->get_value() : get_endogenous_parent_2()->get_value();
}

}
}
}
