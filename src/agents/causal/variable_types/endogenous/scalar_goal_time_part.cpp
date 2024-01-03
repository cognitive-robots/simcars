
#include <ori/simcars/agents/causal/variable_types/endogenous/scalar_goal_time_part.hpp>

namespace ori
{
namespace simcars
{
namespace agents
{
namespace causal
{

bool ScalarGoalTimePartVariable::get_value(temporal::Time &val) const
{
    Goal<FP_DATA_TYPE> goal;
    if (get_parent()->get_value(goal))
    {
        val = goal.time;
        return true;
    }
    else
    {
        return false;
    }
}

bool ScalarGoalTimePartVariable::set_value(temporal::Time const &val)
{
    Goal<FP_DATA_TYPE> goal;
    if (get_parent()->get_value(goal))
    {
        goal.time = val;
        return get_parent()->set_value(goal);
    }
    else
    {
        return false;
    }
}

}
}
}
}