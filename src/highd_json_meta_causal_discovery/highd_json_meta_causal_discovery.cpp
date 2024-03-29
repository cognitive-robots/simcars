
#include <ori/simcars/structures/stl/stl_set.hpp>
#include <ori/simcars/geometry/trig_buff.hpp>
#include <ori/simcars/map/highd/highd_map.hpp>
#include <ori/simcars/agent/highd/highd_scene.hpp>
#include <ori/simcars/causal/necessary_driving_causal_discoverer.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <exception>
#include <string>
#include <fstream>
#include <filesystem>

#define CONTROLLER_LOOKAHEAD_STEPS 10

using namespace ori::simcars;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: ./highd_json_meta_causal_discovery reward_diff_threshold "
                     "input_json_meta_file_path trimmed_data_directory_path "
                     "[output_json_meta_file_path]" << std::endl;
        return -1;
    }

    FP_DATA_TYPE reward_diff_threshold = std::atof(argv[1]);

    std::cout << "Reward Diff. Threshold: " << std::to_string(reward_diff_threshold) << std::endl;

    std::string input_json_meta_file_path_str = argv[2];
    std::filesystem::path input_json_meta_file_path(input_json_meta_file_path_str);

    if (!std::filesystem::is_regular_file(input_json_meta_file_path))
    {
        throw std::invalid_argument("Input JSON meta file path '" +
                                    input_json_meta_file_path_str +
                                    "' does not indicate a valid file");
    }

    std::string output_json_meta_file_path_str;
    std::filesystem::path output_json_meta_file_path;

    if (argc > 4)
    {
        output_json_meta_file_path_str = argv[4];
        output_json_meta_file_path = std::filesystem::path(output_json_meta_file_path_str);
        std::filesystem::path output_json_meta_file_path_dir = output_json_meta_file_path.parent_path();

        if (!std::filesystem::is_directory(output_json_meta_file_path_dir) ||
                std::filesystem::is_directory(output_json_meta_file_path))
        {
            throw std::invalid_argument("Output JSON meta file path '" +
                                        output_json_meta_file_path_str +
                                        "' does not indicate a valid location to output a file");
        }
    }

    std::ifstream input_json_meta_filestream(input_json_meta_file_path);
    rapidjson::IStreamWrapper input_json_meta_stream(input_json_meta_filestream);

    rapidjson::Document json_meta_document;
    json_meta_document.ParseStream(input_json_meta_stream);

    uint32_t scene_id = json_meta_document["scene_id"].GetUint();
    uint32_t convoy_head_id = json_meta_document["convoy_head_id"].GetUint();
    uint32_t convoy_tail_id = json_meta_document["convoy_tail_id"].GetUint();
    uint32_t independent_id = json_meta_document["independent_id"].GetUint();

    std::string convoy_head_str = "non_ego_vehicle_" + std::to_string(convoy_head_id);
    std::string convoy_tail_str = "non_ego_vehicle_" + std::to_string(convoy_tail_id);
    std::string independent_str = "non_ego_vehicle_" + std::to_string(independent_id);

    std::cout << "Convoy Head Agent = " << convoy_head_str << std::endl;
    std::cout << "Convoy Tail Agent = " << convoy_tail_str << std::endl;
    std::cout << "Independent Agent = " << independent_str << std::endl;

    std::string raw_data_directory_path_str = argv[3];
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

    structures::ISet<std::string>* agents_of_interest = new structures::stl::STLSet<std::string>;
    agents_of_interest->insert("non_ego_vehicle_" + std::to_string(convoy_head_id));
    agents_of_interest->insert("non_ego_vehicle_" + std::to_string(convoy_tail_id));
    agents_of_interest->insert("non_ego_vehicle_" + std::to_string(independent_id));


    geometry::TrigBuff::init_instance(360000, geometry::AngleType::RADIANS);

    time_point<high_resolution_clock> start_time;
    microseconds time_elapsed;

    std::cout << "Beginning map load" << std::endl;

    start_time = high_resolution_clock::now();

    map::IMap<uint8_t> const *map = map::highd::HighDMap::load(recording_meta_file_path.string());

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished map load (" << time_elapsed.count() << " μs)" << std::endl;

    std::cout << "Beginning scene load" << std::endl;

    start_time = high_resolution_clock::now();

    agent::IDrivingScene *scene = agent::highd::HighDScene::load(tracks_meta_file_path.string(),
                                                                 tracks_file_path.string());

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished scene load (" << time_elapsed.count() << " μs)" << std::endl;

    std::cout << "Beginning causal discovery" << std::endl;

    start_time = high_resolution_clock::now();

    causal::ICausalDiscoverer *causal_discoverer = new causal::NecessaryDrivingCausalDiscoverer(
                map, scene->get_time_step(), CONTROLLER_LOOKAHEAD_STEPS, reward_diff_threshold,
                temporal::Duration(0));

    structures::ISet<std::pair<std::string, std::string>> *reward_entity_causal_links =
            new structures::stl::STLSet<std::pair<std::string, std::string>, causal::PairHasher<std::string, std::string>>;
    structures::ISet<std::pair<std::string, std::string>> *agency_entity_causal_links =
            new structures::stl::STLSet<std::pair<std::string, std::string>, causal::PairHasher<std::string, std::string>>;
    structures::ISet<std::pair<std::string, std::string>> *hybrid_entity_causal_links =
            new structures::stl::STLSet<std::pair<std::string, std::string>, causal::PairHasher<std::string, std::string>>;

    causal_discoverer->discover_entity_causal_links(scene, reward_entity_causal_links,
                                                    agency_entity_causal_links,
                                                    hybrid_entity_causal_links, agents_of_interest);

    time_elapsed = duration_cast<microseconds>(high_resolution_clock::now() - start_time);

    std::cout << "Finished causal discovery (" << time_elapsed.count() << " μs)" << std::endl;


    structures::IArray<std::pair<std::string, std::string>> const *reward_entity_causal_link_array =
            reward_entity_causal_links->get_array();
    structures::IArray<std::pair<std::string, std::string>> const *agency_entity_causal_link_array =
            agency_entity_causal_links->get_array();
    structures::IArray<std::pair<std::string, std::string>> const *hybrid_entity_causal_link_array =
            hybrid_entity_causal_links->get_array();

    std::cout << "Reward Discovered Causal Links: " << std::endl;
    size_t i;
    for (i = 0; i < reward_entity_causal_link_array->count(); ++i)
    {
        std::pair<std::string, std::string> const &entity_causal_link =
                (*reward_entity_causal_link_array)[i];
        std::cout << entity_causal_link.first << " -> " << entity_causal_link.second << std::endl;
    }
    std::cout << "Agency Discovered Causal Links: " << std::endl;
    for (i = 0; i < agency_entity_causal_link_array->count(); ++i)
    {
        std::pair<std::string, std::string> const &entity_causal_link =
                (*agency_entity_causal_link_array)[i];
        std::cout << entity_causal_link.first << " -> " << entity_causal_link.second << std::endl;
    }
    std::cout << "Hybrid Discovered Causal Links: " << std::endl;
    for (i = 0; i < hybrid_entity_causal_link_array->count(); ++i)
    {
        std::pair<std::string, std::string> const &entity_causal_link =
                (*hybrid_entity_causal_link_array)[i];
        std::cout << entity_causal_link.first << " -> " << entity_causal_link.second << std::endl;
    }


    if (argc > 4)
    {
        rapidjson::Value json_reward_causal_links(rapidjson::kObjectType);
        for (i = 0; i < reward_entity_causal_link_array->count(); ++i)
        {
            std::pair<std::string, std::string> const &entity_causal_link =
                    (*reward_entity_causal_link_array)[i];

            uint32_t cause_id;
            if (entity_causal_link.first == convoy_head_str)
            {
                cause_id = convoy_head_id;
            }
            else if (entity_causal_link.first == convoy_tail_str)
            {
                cause_id = convoy_tail_id;
            }
            else if (entity_causal_link.first == independent_str)
            {
                cause_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            uint32_t effect_id;
            if (entity_causal_link.second == convoy_head_str)
            {
                effect_id = convoy_head_id;
            }
            else if (entity_causal_link.second == convoy_tail_str)
            {
                effect_id = convoy_tail_id;
            }
            else if (entity_causal_link.second == independent_str)
            {
                effect_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            std::string cause_str = std::to_string(cause_id);
            rapidjson::Value cause_json_str;
            cause_json_str.SetString(cause_str.c_str(), json_meta_document.GetAllocator());
            rapidjson::Value effect_json_uint;
            effect_json_uint.SetUint(effect_id);
            if (!json_reward_causal_links.HasMember(cause_json_str))
            {
                rapidjson::Value json_causal_effects(rapidjson::kArrayType);
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
                json_reward_causal_links.AddMember(cause_json_str, json_causal_effects,
                                            json_meta_document.GetAllocator());
            }
            else
            {
                rapidjson::Value::Array const &json_causal_effects =
                        json_reward_causal_links[cause_json_str].GetArray();
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
            }
        }
        json_meta_document.AddMember("reward_causal_links", json_reward_causal_links,
                                     json_meta_document.GetAllocator());

        rapidjson::Value json_agency_causal_links(rapidjson::kObjectType);
        for (i = 0; i < agency_entity_causal_link_array->count(); ++i)
        {
            std::pair<std::string, std::string> const &entity_causal_link =
                    (*agency_entity_causal_link_array)[i];

            uint32_t cause_id;
            if (entity_causal_link.first == convoy_head_str)
            {
                cause_id = convoy_head_id;
            }
            else if (entity_causal_link.first == convoy_tail_str)
            {
                cause_id = convoy_tail_id;
            }
            else if (entity_causal_link.first == independent_str)
            {
                cause_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            uint32_t effect_id;
            if (entity_causal_link.second == convoy_head_str)
            {
                effect_id = convoy_head_id;
            }
            else if (entity_causal_link.second == convoy_tail_str)
            {
                effect_id = convoy_tail_id;
            }
            else if (entity_causal_link.second == independent_str)
            {
                effect_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            std::string cause_str = std::to_string(cause_id);
            rapidjson::Value cause_json_str;
            cause_json_str.SetString(cause_str.c_str(), json_meta_document.GetAllocator());
            rapidjson::Value effect_json_uint;
            effect_json_uint.SetUint(effect_id);
            if (!json_agency_causal_links.HasMember(cause_json_str))
            {
                rapidjson::Value json_causal_effects(rapidjson::kArrayType);
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
                json_agency_causal_links.AddMember(cause_json_str, json_causal_effects,
                                            json_meta_document.GetAllocator());
            }
            else
            {
                rapidjson::Value::Array const &json_causal_effects =
                        json_agency_causal_links[cause_json_str].GetArray();
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
            }
        }
        json_meta_document.AddMember("agency_causal_links", json_agency_causal_links,
                                     json_meta_document.GetAllocator());

        rapidjson::Value json_hybrid_causal_links(rapidjson::kObjectType);
        for (i = 0; i < hybrid_entity_causal_link_array->count(); ++i)
        {
            std::pair<std::string, std::string> const &entity_causal_link =
                    (*hybrid_entity_causal_link_array)[i];

            uint32_t cause_id;
            if (entity_causal_link.first == convoy_head_str)
            {
                cause_id = convoy_head_id;
            }
            else if (entity_causal_link.first == convoy_tail_str)
            {
                cause_id = convoy_tail_id;
            }
            else if (entity_causal_link.first == independent_str)
            {
                cause_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            uint32_t effect_id;
            if (entity_causal_link.second == convoy_head_str)
            {
                effect_id = convoy_head_id;
            }
            else if (entity_causal_link.second == convoy_tail_str)
            {
                effect_id = convoy_tail_id;
            }
            else if (entity_causal_link.second == independent_str)
            {
                effect_id = independent_id;
            }
            else
            {
                throw std::runtime_error("Unknown entity string");
            }

            std::string cause_str = std::to_string(cause_id);
            rapidjson::Value cause_json_str;
            cause_json_str.SetString(cause_str.c_str(), json_meta_document.GetAllocator());
            rapidjson::Value effect_json_uint;
            effect_json_uint.SetUint(effect_id);
            if (!json_hybrid_causal_links.HasMember(cause_json_str))
            {
                rapidjson::Value json_causal_effects(rapidjson::kArrayType);
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
                json_hybrid_causal_links.AddMember(cause_json_str, json_causal_effects,
                                            json_meta_document.GetAllocator());
            }
            else
            {
                rapidjson::Value::Array const &json_causal_effects =
                        json_hybrid_causal_links[cause_json_str].GetArray();
                json_causal_effects.PushBack(effect_json_uint, json_meta_document.GetAllocator());
            }
        }
        json_meta_document.AddMember("hybrid_causal_links", json_hybrid_causal_links,
                                     json_meta_document.GetAllocator());

        rapidjson::Value time_elapsed_in_microseconds;
        time_elapsed_in_microseconds.SetInt64(time_elapsed.count());
        json_meta_document.AddMember("time_elapsed_in_microseconds", time_elapsed_in_microseconds,
                                     json_meta_document.GetAllocator());

        std::ofstream output_json_meta_filestream(output_json_meta_file_path);
        rapidjson::OStreamWrapper output_json_meta_stream(output_json_meta_filestream);
        rapidjson::Writer<rapidjson::OStreamWrapper> output_json_meta_writer(
                    output_json_meta_stream);
        json_meta_document.Accept(output_json_meta_writer);
    }


    delete reward_entity_causal_links;
    delete agency_entity_causal_links;
    delete hybrid_entity_causal_links;

    delete agents_of_interest;

    delete causal_discoverer;

    delete scene;

    delete map;

    geometry::TrigBuff::destroy_instance();
}
