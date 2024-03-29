#pragma once

#include <ori/simcars/structures/set_interface.hpp>
#include <ori/simcars/agent/scene_interface.hpp>

namespace ori
{
namespace simcars
{
namespace causal
{

class ICausalDiscoverer
{
public:
    virtual ~ICausalDiscoverer() = default;

    virtual void discover_entity_causal_links(
            agent::IScene const *scene,
            structures::ISet<std::pair<std::string, std::string>>* reward_discovered,
            structures::ISet<std::pair<std::string, std::string>>* agency_discovered,
            structures::ISet<std::pair<std::string, std::string>>* hybrid_discovered,
            structures::ISet<std::string> const *agents_of_interest = nullptr) const = 0;
};

}
}
}
