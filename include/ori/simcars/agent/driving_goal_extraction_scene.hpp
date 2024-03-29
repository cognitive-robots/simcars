#pragma once

#include <ori/simcars/structures/stl/stl_dictionary.hpp>
#include <ori/simcars/map/map_interface.hpp>
#include <ori/simcars/agent/driving_scene_abstract.hpp>
#include <ori/simcars/agent/driving_goal_extraction_agent.hpp>

namespace ori
{
namespace simcars
{
namespace agent
{

template <typename T_map_id>
class DrivingGoalExtractionScene : public virtual ADrivingScene
{
    geometry::Vec min_spatial_limits, max_spatial_limits;
    temporal::Duration time_step;
    temporal::Time min_temporal_limit, max_temporal_limit;

    structures::stl::STLDictionary<std::string, IDrivingAgent*> driving_agent_dict;

    map::IMap<T_map_id> const *map;

protected:
    DrivingGoalExtractionScene() : map(nullptr) {}

public:
    ~DrivingGoalExtractionScene()
    {
        structures::IArray<IDrivingAgent*> const *driving_agents = driving_agent_dict.get_values();

        for (size_t i = 0; i < driving_agents->count(); ++i)
        {
            delete (*driving_agents)[i];
        }
    }

    static DrivingGoalExtractionScene* construct_from(IDrivingScene *driving_scene)
    {
        DrivingGoalExtractionScene *new_driving_scene = new DrivingGoalExtractionScene;

        new_driving_scene->min_spatial_limits = driving_scene->get_min_spatial_limits();
        new_driving_scene->max_spatial_limits = driving_scene->get_max_spatial_limits();
        new_driving_scene->time_step = driving_scene->get_time_step();
        new_driving_scene->min_temporal_limit = driving_scene->get_min_temporal_limit();
        new_driving_scene->max_temporal_limit = driving_scene->get_max_temporal_limit();

        structures::IArray<IDrivingAgent*> *driving_agents =
                driving_scene->get_mutable_driving_agents();

        for(size_t i = 0; i < driving_agents->count(); ++i)
        {
            IDrivingAgent *driving_goal_extraction_agent =
                    new DrivingGoalExtractionAgent<T_map_id>(
                        (*driving_agents)[i], new_driving_scene);

            new_driving_scene->driving_agent_dict.update(driving_goal_extraction_agent->get_name(), driving_goal_extraction_agent);
        }

        delete driving_agents;

        return new_driving_scene;
    }
    static DrivingGoalExtractionScene* construct_from(IDrivingScene *driving_scene,
                                                            map::IMap<T_map_id> const *map)
    {
        DrivingGoalExtractionScene *new_driving_scene = new DrivingGoalExtractionScene;

        new_driving_scene->min_spatial_limits = driving_scene->get_min_spatial_limits();
        new_driving_scene->max_spatial_limits = driving_scene->get_max_spatial_limits();
        new_driving_scene->time_step = driving_scene->get_time_step();
        new_driving_scene->min_temporal_limit = driving_scene->get_min_temporal_limit();
        new_driving_scene->max_temporal_limit = driving_scene->get_max_temporal_limit();

        new_driving_scene->map = map;

        structures::IArray<IDrivingAgent*> *driving_agents =
                driving_scene->get_mutable_driving_agents();

        for(size_t i = 0; i < driving_agents->count(); ++i)
        {
            IDrivingAgent *driving_goal_extraction_agent =
                    new DrivingGoalExtractionAgent<T_map_id>(
                        (*driving_agents)[i], new_driving_scene, map);

            new_driving_scene->driving_agent_dict.update(driving_goal_extraction_agent->get_name(),
                                                         driving_goal_extraction_agent);
        }

        delete driving_agents;

        return new_driving_scene;
    }

    IDrivingScene* driving_scene_deep_copy() const override
    {
        DrivingGoalExtractionScene *new_driving_scene = new DrivingGoalExtractionScene;

        new_driving_scene->min_spatial_limits = this->min_spatial_limits;
        new_driving_scene->max_spatial_limits = this->max_spatial_limits;
        new_driving_scene->time_step = this->time_step;
        new_driving_scene->min_temporal_limit = this->min_temporal_limit;
        new_driving_scene->max_temporal_limit = this->max_temporal_limit;

        new_driving_scene->map = this->map;

        structures::IArray<IDrivingAgent*> const *driving_agents = this->driving_agent_dict.get_values();

        for(size_t i = 0; i < driving_agents->count(); ++i)
        {
            new_driving_scene->driving_agent_dict.update((*driving_agents)[i]->get_name(),
                                                         (*driving_agents)[i]->driving_agent_deep_copy(new_driving_scene));
        }

        return new_driving_scene;
    }

    geometry::Vec get_min_spatial_limits() const override
    {
        return min_spatial_limits;
    }
    geometry::Vec get_max_spatial_limits() const override
    {
        return max_spatial_limits;
    }

    temporal::Duration get_time_step() const override
    {
        return time_step;
    }

    temporal::Time get_min_temporal_limit() const override
    {
        return min_temporal_limit;
    }
    temporal::Time get_max_temporal_limit() const override
    {
        return max_temporal_limit;
    }

    structures::IArray<IDrivingAgent const*>* get_driving_agents() const
    {
        structures::stl::STLStackArray<IDrivingAgent const*> *driving_agents =
                new structures::stl::STLStackArray<IDrivingAgent const*>(driving_agent_dict.count());
        cast_array(*driving_agent_dict.get_values(), *driving_agents);
        return driving_agents;
    }
    IDrivingAgent const* get_driving_agent(std::string const &driving_agent_name) const override
    {
        return driving_agent_dict[driving_agent_name];
    }

    bool has_map() const
    {
        return map != nullptr;
    }
    map::IMap<T_map_id> const* get_map() const
    {
        return map;
    }

    structures::IArray<IDrivingAgent*>* get_mutable_driving_agents() override
    {
        structures::stl::STLStackArray<IDrivingAgent*> *driving_agents =
                new structures::stl::STLStackArray<IDrivingAgent*>(driving_agent_dict.count());
        driving_agent_dict.get_values(driving_agents);
        return driving_agents;
    }
    IDrivingAgent* get_mutable_driving_agent(std::string const &driving_agent_name) override
    {
        return driving_agent_dict[driving_agent_name];
    }
};

}
}
}
