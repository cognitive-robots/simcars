
#include <ori/simcars/visualisation/utils.hpp>

namespace ori
{
namespace simcars
{
namespace visualisation
{

sf::Vector2f to_sfml_vec(geometry::Vec const &vec, bool flip_x, bool flip_y)
{
    FP_DATA_TYPE x;
    FP_DATA_TYPE y;
    if (flip_x)
    {
        x = -vec.x();
    }
    else
    {
        x = vec.x();
    }
    if (flip_y)
    {
        y = -vec.y();
    }
    else
    {
        y = vec.y();
    }
    return sf::Vector2f(x, y);
}

sf::Color to_sfml_colour(map::ITrafficLightStateHolder::FaceColour face_colour)
{
    switch (face_colour)
    {
    case map::ITrafficLightStateHolder::FaceColour::UNKNOWN:
        break;

    case map::ITrafficLightStateHolder::FaceColour::RED:
        return sf::Color(255, 0, 0);

    case map::ITrafficLightStateHolder::FaceColour::YELLOW:
        return sf::Color(255, 255, 0);

    case map::ITrafficLightStateHolder::FaceColour::GREEN:
        return sf::Color(0, 255, 0);
    }

    return sf::Color(255, 255, 255);
}

sf::Color to_sfml_colour(agent::DrivingAgentClass driving_agent_class)
{
    switch(driving_agent_class)
    {
    case agent::DrivingAgentClass::UNKNOWN:
        break;

    case agent::DrivingAgentClass::CAR:
        return sf::Color(255, 128, 0);

    case agent::DrivingAgentClass::VAN:
        return sf::Color(255, 255, 0);

    case agent::DrivingAgentClass::TRAM:
        return sf::Color(128, 255, 0);

    case agent::DrivingAgentClass::BUS:
        return sf::Color(0, 255, 0);

    case agent::DrivingAgentClass::TRUCK:
        return sf::Color(0, 255, 128);

    case agent::DrivingAgentClass::EMERGENCY_VEHICLE:
        return sf::Color(0, 255, 255);

    case agent::DrivingAgentClass::BICYCLE:
        return sf::Color(0, 128, 255);

    case agent::DrivingAgentClass::MOTORCYCLE:
        return sf::Color(0, 0, 255);

    case agent::DrivingAgentClass::PEDESTRIAN:
        return sf::Color(128, 0, 255);

    case agent::DrivingAgentClass::ANIMAL:
        return sf::Color(255, 0, 255);

    case agent::DrivingAgentClass::OTHER_VEHICLE:
        return sf::Color(255, 0, 128);
    }

    return sf::Color(255, 255, 255);
}

/*
sf::Color to_sfml_colour(agent::IAgent::Status agent_status)
{
    switch(agent_status)
    {
    case agent::IAgent::Status::UNKNOWN:
        break;

    case agent::IAgent::Status::NOMINAL:
        return sf::Color(128, 255, 0);

    case agent::IAgent::Status::WARNING:
        return sf::Color(255, 255, 0);

    case agent::IAgent::Status::ERROR:
        return sf::Color(255, 128, 0);

    case agent::IAgent::Status::FATAL:
        return sf::Color(255, 0, 0);
    }

    return sf::Color(255, 255, 255);
}
*/

}
}
}
