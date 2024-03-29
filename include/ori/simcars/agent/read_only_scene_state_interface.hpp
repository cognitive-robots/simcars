#pragma once

#include <ori/simcars/temporal/typedefs.hpp>
#include <ori/simcars/agent/entity_state_interface.hpp>

namespace ori
{
namespace simcars
{
namespace agent
{

class IReadOnlySceneState
{
public:
    virtual ~IReadOnlySceneState() = default;

    virtual temporal::Time get_time() const = 0;

    virtual structures::IArray<IReadOnlyEntityState const*>* get_entity_states() const = 0;
    virtual IReadOnlyEntityState const* get_entity_state(std::string const &entity_name) const = 0;
};

}
}
}
