#pragma once

#include <ori/simcars/structures/stl/stl_dictionary.hpp>
#include <ori/simcars/agent/two_file_based_scene_abstract.hpp>
#include <ori/simcars/agent/driving_scene_abstract.hpp>
#include <ori/simcars/agent/highd/highd_driving_agent.hpp>

namespace ori
{
namespace simcars
{
namespace agent
{
namespace highd
{

class HighDScene : public virtual ATwoFileBasedScene<HighDScene>, public virtual ADrivingScene
{
    geometry::Vec min_spatial_limits, max_spatial_limits;
    temporal::Duration time_step;
    temporal::Time min_temporal_limit, max_temporal_limit;

    structures::stl::STLDictionary<std::string, IDrivingAgent*> driving_agent_dict;

protected:
    void save_virt(std::ofstream &output_filestream_1, std::ofstream &output_filestream_2) const override;
    void load_virt(std::ifstream &input_filestream_1, std::ifstream &input_filestream_2,
                   structures::ISet<std::string>* agent_names) override;

public:
    ~HighDScene();

    static HighDScene* construct_from(IDrivingScene *driving_scene);

    IDrivingScene* driving_scene_deep_copy() const override;

    geometry::Vec get_min_spatial_limits() const override;
    geometry::Vec get_max_spatial_limits() const override;

    temporal::Duration get_time_step() const override;

    temporal::Time get_min_temporal_limit() const override;
    temporal::Time get_max_temporal_limit() const override;

    structures::IArray<IDrivingAgent const*>* get_driving_agents() const override;
    IDrivingAgent const* get_driving_agent(std::string const &driving_agent_name) const override;

    structures::IArray<IDrivingAgent*>* get_mutable_driving_agents() override;
    IDrivingAgent* get_mutable_driving_agent(std::string const &driving_agent_name) override;
};

}
}
}
}
