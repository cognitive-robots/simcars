#pragma once

#include <ori/simcars/geometry/typedefs.hpp>
#include <ori/simcars/map/traffic_light_interface.hpp>
#include <ori/simcars/agent/agent_interface.hpp>

#include <SFML/Graphics.hpp>

namespace ori
{
namespace simcars
{
namespace visualisation
{

sf::Vector2f to_sfml_vec(const geometry::Vec& vec);

sf::Color to_sfml_colour(map::ITrafficLightStateHolder::FaceColour face_colour);

sf::Color to_sfml_colour(agent::IAgent::Class agent_class);

sf::Color to_sfml_colour(agent::IAgent::Status agent_status);

}
}
}
