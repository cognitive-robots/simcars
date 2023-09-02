#pragma once

#include <ori/simcars/geometry/defines.hpp>
#include <ori/simcars/causal/ternary_endogenous_variable_abstract.hpp>

namespace ori
{
namespace simcars
{
namespace causal
{

class ScalarConditionalVariable :
        public ATernaryEndogenousVariable<FP_DATA_TYPE, FP_DATA_TYPE, FP_DATA_TYPE, bool>
{
public:
    using ATernaryEndogenousVariable<FP_DATA_TYPE, FP_DATA_TYPE, FP_DATA_TYPE, bool>::ATernaryEndogenousVariable;

    FP_DATA_TYPE get_value() const override;
};

}
}
}
