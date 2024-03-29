
#include <ori/simcars/structures/stl/stl_set.hpp>
#include <ori/simcars/geometry/trig_buff.hpp>
#include <ori/simcars/map/highd/highd_map.hpp>
#include <ori/simcars/agent/driving_goal_extraction_scene.hpp>
#include <ori/simcars/agent/basic_fp_action_sampler.hpp>
#include <ori/simcars/agent/basic_driving_agent_controller.hpp>
#include <ori/simcars/agent/basic_driving_simulator.hpp>
#include <ori/simcars/agent/safe_speedy_driving_agent_reward_calculator.hpp>
#include <ori/simcars/agent/basic_driving_agent_agency_calculator.hpp>
#include <ori/simcars/agent/driving_simulation_scene.hpp>
#include <ori/simcars/agent/driving_simulation_scene_factory.hpp>
#include <ori/simcars/agent/highd/highd_scene.hpp>
#include <ori/simcars/causal/necessary_fp_goal_causal_link_tester.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <iostream>
#include <exception>
#include <random>
#include <string>
#include <fstream>
#include <filesystem>

#define REWARD_DIFF_THRESHOLD 0.05f

using namespace ori::simcars;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: ./highd_json_meta_link_test json_meta_file_path trimmed_data_directory_path" << std::endl;
        return -1;
    }

    std::string json_meta_file_path_str = argv[1];
    std::filesystem::path json_meta_file_path(json_meta_file_path_str);

    if (!std::filesystem::is_regular_file(json_meta_file_path))
    {
        throw std::invalid_argument("JSON meta file path '" +
                                    json_meta_file_path_str +
                                    "' does not indicate a valid file");
    }

    std::ifstream json_meta_filestream(json_meta_file_path);
    rapidjson::BasicIStreamWrapper json_meta_stream(json_meta_filestream);

    rapidjson::Document json_meta_document;
    json_meta_document.ParseStream(json_meta_stream);

    uint32_t scene_id = json_meta_document["scene_id"].GetUint();
    uint32_t convoy_head_id = json_meta_document["convoy_head_id"].GetUint();
    uint32_t convoy_tail_id = json_meta_document["convoy_tail_id"].GetUint();
    uint32_t independent_id = json_meta_document["independent_id"].GetUint();

    std::string raw_data_directory_path_str = argv[2];
    std::filesystem::path raw_data_directory_path(raw_data_directory_path_str);

    std::filesystem::path recording_meta_file_path =
            raw_data_directory_path /
                ("scene-" + std::to_string(scene_id) + "-" + std::to_string(convoy_tail_id) +
                 "_follows_" + std::to_string(convoy_head_id) + "-" +
                 std::to_string(independent_id) + "_independent-recordingMeta.csv");

    if (!std::filesystem::is_regular_file(recording_meta_file_path))
    {
        throw std::invalid_argument("Recording meta file path '" +
                                    recording_meta_file_path.string() +
                                    "' does not indicate a valid file");
    }

    std::filesystem::path tracks_meta_file_path =
            raw_data_directory_path /
                ("scene-" + std::to_string(scene_id) + "-" + std::to_string(convoy_tail_id) +
                 "_follows_" + std::to_string(convoy_head_id) + "-" +
                 std::to_string(independent_id) + "_independent-tracksMeta.csv");

    if (!std::filesystem::is_regular_file(tracks_meta_file_path))
    {
        throw std::invalid_argument("Tracks meta file path '" +
                                    tracks_meta_file_path.string() +
                                    "' does not indicate a valid file");
    }

    std::filesystem::path tracks_file_path =
            raw_data_directory_path /
                ("scene-" + std::to_string(scene_id) + "-" + std::to_string(convoy_tail_id) +
                 "_follows_" + std::to_string(convoy_head_id) + "-" +
                 std::to_string(independent_id) + "_independent-tracks.csv");

    if (!std::filesystem::is_regular_file(tracks_file_path))
    {
        throw std::invalid_argument("Tracks file path '" +
                                    tracks_file_path.string() +
                                    "' does not indicate a valid file");
    }


    geometry::TrigBuff::init_instance(360000, geometry::AngleType::RADIANS);

    time_point<high_resolution_clock> start_time;
    microseconds time_elapsed;

    std::cout << "Beginning map load" << std::endl;

    start_time = high_resolution_clock::now();

    map::IMap<uint8_t> const *map;

    try
    {
        map = map::highd::HighDMap::load(recording_meta_file_path.string());
    }
    catch (std::exception const &e)
    {
        std::cerr << "Exception occured during map load:" << std::endl << e.what() << std::endl;
        return -1;
    }

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished map load (" << time_elapsed.count() << " μs)" << std::endl;

    std::cout << "Beginning scene load" << std::endl;

    start_time = high_resolution_clock::now();

    agent::IDrivingScene *scene;

    try
    {
        scene = agent::highd::HighDScene::load(tracks_meta_file_path.string(),
                                               tracks_file_path.string());
    }
    catch (std::exception const &e)
    {
        std::cerr << "Exception occured during scene load:" << std::endl << e.what() << std::endl;
        return -1;
    }

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished scene load (" << time_elapsed.count() << " μs)" << std::endl;


    std::cout << "Beginning action extraction" << std::endl;

    start_time = high_resolution_clock::now();

    agent::IDrivingScene *scene_with_actions;

    try
    {
        scene_with_actions = agent::DrivingGoalExtractionScene<uint8_t>::construct_from(scene, map);
    }
    catch (std::exception const &e)
    {
        std::cerr << "Exception occured during action extraction:" << std::endl << e.what() << std::endl;
        return -1;
    }

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished action extraction (" << time_elapsed.count() << " μs)" << std::endl;


    std::cout << "Beginning relevant event selection" << std::endl;

    start_time = high_resolution_clock::now();

    agent::IDrivingAgent const *convoy_head_agent =
            scene_with_actions->get_driving_agent("non_ego_vehicle_" + std::to_string(convoy_head_id));
    agent::IValuelessVariable const *convoy_head_aligned_linear_velocity_goal_valueless_variable =
            convoy_head_agent->get_variable_parameter(convoy_head_agent->get_name() +
                                                      ".aligned_linear_velocity.goal");
    agent::IVariable<agent::Goal<FP_DATA_TYPE>> const *convoy_head_aligned_linear_velocity_goal_variable =
            dynamic_cast<agent::IVariable<agent::Goal<FP_DATA_TYPE>> const*>(
                convoy_head_aligned_linear_velocity_goal_valueless_variable);
    structures::IArray<agent::IEvent<agent::Goal<FP_DATA_TYPE>> const*> *convoy_head_aligned_linear_velocity_goal_events =
            convoy_head_aligned_linear_velocity_goal_variable->get_events(
                convoy_head_aligned_linear_velocity_goal_variable->get_min_temporal_limit() +
                scene_with_actions->get_time_step());

    if (convoy_head_aligned_linear_velocity_goal_events->count() <= 0)
    {
        std::cerr << "Could not find convoy head event" << std::endl;
        return -1;
    }

    agent::IEvent<agent::Goal<FP_DATA_TYPE>> const *convoy_head_aligned_linear_velocity_goal_event =
            (*convoy_head_aligned_linear_velocity_goal_events)[0];

    agent::IDrivingAgent const *convoy_tail_agent =
            scene_with_actions->get_driving_agent("non_ego_vehicle_" + std::to_string(convoy_tail_id));
    agent::IValuelessVariable const *convoy_tail_aligned_linear_velocity_goal_valueless_variable =
            convoy_tail_agent->get_variable_parameter(convoy_tail_agent->get_name() +
                                                      ".aligned_linear_velocity.goal");
    agent::IVariable<agent::Goal<FP_DATA_TYPE>> const *convoy_tail_aligned_linear_velocity_goal_variable =
            dynamic_cast<agent::IVariable<agent::Goal<FP_DATA_TYPE>> const*>(
                convoy_tail_aligned_linear_velocity_goal_valueless_variable);
    structures::IArray<agent::IEvent<agent::Goal<FP_DATA_TYPE>> const*> *convoy_tail_aligned_linear_velocity_goal_events =
            convoy_tail_aligned_linear_velocity_goal_variable->get_events(
                convoy_tail_aligned_linear_velocity_goal_variable->get_min_temporal_limit() +
                scene_with_actions->get_time_step());

    agent::IEvent<agent::Goal<FP_DATA_TYPE>> const *convoy_tail_aligned_linear_velocity_goal_event = nullptr;

    size_t i;
    for (i = 0; i < convoy_tail_aligned_linear_velocity_goal_events->count(); ++i)
    {
        if ((*convoy_tail_aligned_linear_velocity_goal_events)[i]->get_time() >
                convoy_head_aligned_linear_velocity_goal_event->get_time())
        {
            convoy_tail_aligned_linear_velocity_goal_event =
                    (*convoy_tail_aligned_linear_velocity_goal_events)[i];
            break;
        }
    }

    if (convoy_tail_aligned_linear_velocity_goal_event == nullptr)
    {
        std::cerr << "Could not find convoy tail event" << std::endl;
        return -1;
    }

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished scene load (" << time_elapsed.count() << " μs)" << std::endl;


    agent::IActionSampler<FP_DATA_TYPE> *action_sampler = new agent::BasicFPActionSampler;

    agent::ISimulationSceneFactory *scene_factory = new agent::DrivingSimulationSceneFactory;

    agent::IDrivingAgentController *driving_agent_controller =
                new agent::BasicDrivingAgentController<uint8_t>(map,
                                                                scene_with_actions->get_time_step(),
                                                                10);

    agent::IDrivingSimulator *driving_simulator =
                new agent::BasicDrivingSimulator(driving_agent_controller);

    agent::IRewardCalculator *reward_calculator = new agent::SafeSpeedyDrivingAgentRewardCalculator;

    agent::IAgencyCalculator *agency_calculator = new agent::BasicDrivingAgentAgencyCalculator;

    causal::NecessaryFPGoalCausalLinkTester *causal_link_tester =
            new causal::NecessaryFPGoalCausalLinkTester(action_sampler, scene_factory,
                                                        driving_simulator, reward_calculator,
                                                        agency_calculator, REWARD_DIFF_THRESHOLD,
                                                        temporal::Duration(0));


    std::cout << "Beginning test" << std::endl;

    start_time = high_resolution_clock::now();

    bool reward_link_present;
    bool agency_link_present;
    bool hybrid_link_present;

    causal_link_tester->test_causal_link(
                scene_with_actions,
                convoy_head_aligned_linear_velocity_goal_event,
                convoy_tail_aligned_linear_velocity_goal_event, reward_link_present,
                agency_link_present, hybrid_link_present);

    if (reward_link_present)
    {
        std::cout << "Reward-based test concluded link is present" << std::endl;
    }
    else
    {
        std::cout << "Reward-based test concluded link is not present" << std::endl;
    }
    if (agency_link_present)
    {
        std::cout << "Agency-based test concluded link is present" << std::endl;
    }
    else
    {
        std::cout << "Agency-based test concluded link is not present" << std::endl;
    }
    if (hybrid_link_present)
    {
        std::cout << "Hybrid test concluded link is present" << std::endl;
    }
    else
    {
        std::cout << "Hybrid test concluded link is not present" << std::endl;
    }

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished test (" << time_elapsed.count() << " μs)" << std::endl;

    delete causal_link_tester;

    delete agency_calculator;
    delete reward_calculator;
    delete driving_simulator;
    delete driving_agent_controller;
    delete scene_factory;
    delete action_sampler;

    delete scene_with_actions;
    delete scene;

    delete map;

    geometry::TrigBuff::destroy_instance();
}
