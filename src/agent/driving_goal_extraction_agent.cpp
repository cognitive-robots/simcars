
#include <ori/simcars/utils/exceptions.hpp>
#include <ori/simcars/structures/stl/stl_concat_array.hpp>
#include <ori/simcars/map/lane_interface.hpp>
#include <ori/simcars/agent/defines.hpp>
#include <ori/simcars/agent/basic_constant.hpp>
#include <ori/simcars/agent/basic_event.hpp>
#include <ori/simcars/agent/basic_variable.hpp>
#include <ori/simcars/agent/goal_driving_agent_state.hpp>
#include <ori/simcars/agent/driving_goal_extraction_agent.hpp>

#include <iostream>

namespace ori
{
namespace simcars
{
namespace agent
{

DrivingGoalExtractionAgent::DrivingGoalExtractionAgent() {}

void DrivingGoalExtractionAgent::extract_aligned_linear_velocity_change_events()
{
    std::shared_ptr<const IVariable<FP_DATA_TYPE>> aligned_linear_velocity_variable =
            this->get_aligned_linear_velocity_variable();
    std::shared_ptr<const IVariable<FP_DATA_TYPE>> aligned_linear_acceleration_variable =
            this->get_aligned_linear_acceleration_variable();

    std::shared_ptr<IVariable<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_variable(
                new BasicVariable<FP_DATA_TYPE>(
                    this->get_name(), "aligned_linear_velocity", IValuelessVariable::Type::GOAL_VALUE));
    std::shared_ptr<IVariable<temporal::Duration>> aligned_linear_velocity_goal_duration_variable(
                new BasicVariable<temporal::Duration>(
                    this->get_name(), "aligned_linear_velocity", IValuelessVariable::Type::GOAL_DURATION));

    temporal::Duration min_duration_threshold(temporal::DurationRep(1000.0 * MIN_ALIGNED_LINEAR_VELOCITY_CHANGE_DURATION_THRESHOLD));

    temporal::Duration time_step = temporal::Duration(temporal::DurationRep(1000.0 / OPERATING_FRAMERATE));
    temporal::Time current_time;
    temporal::Time action_start_time = this->get_min_temporal_limit();
    FP_DATA_TYPE action_start_aligned_linear_velocity = std::numeric_limits<FP_DATA_TYPE>::max();
    for (current_time = this->get_min_temporal_limit() + time_step; current_time <= this->get_max_temporal_limit(); current_time += time_step)
    {
        try
        {
            FP_DATA_TYPE current_aligned_linear_acceleration = aligned_linear_acceleration_variable->get_value(current_time);
            FP_DATA_TYPE previous_aligned_linear_acceleration = aligned_linear_acceleration_variable->get_value(current_time - time_step);

            if (previous_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    // STATUS QUO

                    if (current_time + time_step > this->get_max_temporal_limit())
                    {
                        if ((current_time - action_start_time >= min_duration_threshold
                             && std::abs(aligned_linear_velocity_variable->get_value(current_time) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                                || action_start_time == this->get_min_temporal_limit())
                        {
                            std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                                   aligned_linear_velocity_variable->get_value(current_time),
                                                                                                                                   action_start_time));
                            aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                            for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                            {
                                std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                      current_time - current_time_2,
                                                                                                                                                      current_time_2));
                                aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                            }
                        }
                    }
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (((current_time - time_step) - action_start_time >= min_duration_threshold
                         && std::abs(aligned_linear_velocity_variable->get_value(current_time - time_step) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + time_step > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time - time_step),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time - time_step; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  (current_time - time_step) - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }

                    action_start_time = current_time - time_step;
                    action_start_aligned_linear_velocity = aligned_linear_velocity_variable->get_value(current_time - time_step);
                }
                else
                {
                    if ((current_time - action_start_time >= min_duration_threshold
                         && std::abs(aligned_linear_velocity_variable->get_value(current_time) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + time_step > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  current_time - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }
                }
            }
            else if (previous_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (((current_time - time_step) - action_start_time >= min_duration_threshold
                         && std::abs(aligned_linear_velocity_variable->get_value(current_time - time_step) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + time_step > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time - time_step),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time - time_step; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  (current_time - time_step) - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }

                    action_start_time = current_time - time_step;
                    action_start_aligned_linear_velocity = aligned_linear_velocity_variable->get_value(current_time - time_step);
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    // STATUS QUO

                    if (current_time + time_step > this->get_max_temporal_limit())
                    {
                        if ((current_time - action_start_time >= min_duration_threshold
                             && std::abs(aligned_linear_velocity_variable->get_value(current_time) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                                || action_start_time == this->get_min_temporal_limit())
                        {
                            std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                                   aligned_linear_velocity_variable->get_value(current_time),
                                                                                                                                   action_start_time));
                            aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                            for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                            {
                                std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                      current_time - current_time_2,
                                                                                                                                                      current_time_2));
                                aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                            }
                        }
                    }
                }
                else
                {
                    if ((current_time - action_start_time >= min_duration_threshold
                         && std::abs(aligned_linear_velocity_variable->get_value(current_time) - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + time_step > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  current_time - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }
                }
            }
            else
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (action_start_time == this->get_min_temporal_limit())
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time - time_step),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time - time_step; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  (current_time - time_step) - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }

                    action_start_time = current_time - time_step;
                    action_start_aligned_linear_velocity = aligned_linear_velocity_variable->get_value(current_time - time_step);
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (action_start_time == this->get_min_temporal_limit())
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time - time_step),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time - time_step; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  (current_time - time_step) - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }

                    action_start_time = current_time - time_step;
                    action_start_aligned_linear_velocity = aligned_linear_velocity_variable->get_value(current_time - time_step);
                }
                else
                {
                    // STATUS QUO

                    if (current_time + time_step > this->get_max_temporal_limit() && action_start_time == this->get_min_temporal_limit())
                    {
                        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                                               aligned_linear_velocity_variable->get_value(current_time),
                                                                                                                               action_start_time));
                        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                                                  current_time - current_time_2,
                                                                                                                                                  current_time_2));
                            aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
                        }
                    }
                }
            }
        }
        catch (std::out_of_range)
        {
            // Attempt to get value from before variable was initialised
        }
    }

    if (aligned_linear_velocity_goal_value_variable->get_events()->count() == 0 ||
            aligned_linear_velocity_goal_value_variable->get_min_temporal_limit() > this->get_min_temporal_limit())
    {
        std::shared_ptr<IEvent<FP_DATA_TYPE>> aligned_linear_velocity_goal_value_event(new BasicEvent<FP_DATA_TYPE>(aligned_linear_velocity_goal_value_variable->get_full_name(),
                                                                                                               aligned_linear_velocity_variable->get_value(this->get_min_temporal_limit()),
                                                                                                               this->get_min_temporal_limit()));
        aligned_linear_velocity_goal_value_variable->add_event(aligned_linear_velocity_goal_value_event);

        std::shared_ptr<IEvent<temporal::Duration>> aligned_linear_velocity_goal_duration_event(new BasicEvent<temporal::Duration>(aligned_linear_velocity_goal_duration_variable->get_full_name(),
                                                                                                                              temporal::Duration(0),
                                                                                                                              this->get_min_temporal_limit()));
        aligned_linear_velocity_goal_duration_variable->add_event(aligned_linear_velocity_goal_duration_event);
    }

    this->variable_dict.update(aligned_linear_velocity_goal_value_variable->get_full_name(), aligned_linear_velocity_goal_value_variable);
    this->variable_dict.update(aligned_linear_velocity_goal_duration_variable->get_full_name(), aligned_linear_velocity_goal_duration_variable);
}

void DrivingGoalExtractionAgent::extract_lane_change_events(std::shared_ptr<const map::IMap<std::string> > map)
{
    std::shared_ptr<const IVariable<geometry::Vec>> position_variable = this->get_position_variable();

    temporal::Duration min_duration_threshold(temporal::DurationRep(1000.0 * MIN_LANE_CHANGE_DURATION_THRESHOLD));

    std::shared_ptr<IVariable<int32_t>> lane_goal_value_variable(
                new BasicVariable<int32_t>(
                    this->get_name(), "lane", IValuelessVariable::Type::GOAL_VALUE));
    std::shared_ptr<IVariable<temporal::Duration>> lane_goal_duration_variable(
                new BasicVariable<temporal::Duration>(
                    this->get_name(), "lane", IValuelessVariable::Type::GOAL_DURATION));

    temporal::Duration time_step = temporal::Duration(temporal::DurationRep(1000.0 / OPERATING_FRAMERATE));
    temporal::Time current_time;
    temporal::Time action_start_time;
    temporal::Time lane_change_time = temporal::Time::max();
    int32_t lane_change_offset = 0;
    std::shared_ptr<const map::ILaneArray<std::string>> previous_lanes = nullptr;
    for (current_time = this->get_min_temporal_limit(); current_time <= this->get_max_temporal_limit(); current_time += time_step)
    {
        try
        {
            geometry::Vec current_position = position_variable->get_value(current_time);
            std::shared_ptr<const map::ILaneArray<std::string>> current_lanes =
                    map->get_encapsulating_lanes(current_position);

            if (current_lanes->count() == 0)
            {
                // Vehicle is currently not on a lane segment. This can happen quite often due to small gaps inbetween the lane segments. As such we just disregard this timestep.
                continue;
            }

            if (previous_lanes != nullptr)
            {
                std::shared_ptr<structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>> continuing_lanes(
                            new structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>());
                std::shared_ptr<structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>> left_lanes(
                            new structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>());
                std::shared_ptr<structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>> right_lanes(
                            new structures::stl::STLStackArray<std::shared_ptr<const map::ILane<std::string>>>());
                for (size_t j = 0; j < current_lanes->count(); ++j)
                {
                    std::shared_ptr<const map::ILane<std::string>> current_lane = (*current_lanes)[j];
                    for (size_t k = 0; k < previous_lanes->count(); ++k)
                    {
                        std::shared_ptr<const map::ILane<std::string>> previous_lane = (*previous_lanes)[k];
                        if (current_lane == previous_lane || current_lane->get_aft_lanes()->contains(previous_lane))
                        {
                            continuing_lanes->push_back(current_lane);
                        }
                        else if (current_lane->get_right_adjacent_lane() == previous_lane)
                        {
                            left_lanes->push_back(current_lane);
                        }
                        else if (current_lane->get_left_adjacent_lane() == previous_lane)
                        {
                            right_lanes->push_back(current_lane);
                        }
                    }
                }

                if (continuing_lanes->count() == 0)
                {
                    if (left_lanes->count() > 0)
                    {
                        --lane_change_offset;
                        lane_change_time = current_time;
                        previous_lanes = left_lanes;
                    }
                    else if (right_lanes->count() > 0)
                    {
                        ++lane_change_offset;
                        lane_change_time = current_time;
                        previous_lanes = right_lanes;
                    }
                    else
                    {
                        // This should be an exceptionally rare occurance, the vehicle is occupying at least one valid lane, but none of the occupied lanes are adjacent to the previously occupied lanes.
                        // Because this is so rare, and difficult to handle, for now we essentially treat it as though the current lane occupation is a continuation of the previous lane occupation.
                        previous_lanes = current_lanes;
                    }
                }
                else
                {
                    // We've found a valid continuation for the previous lanes, so now update the previous lanes with the lanes found to be valid continuations.
                    previous_lanes = continuing_lanes;
                }
            }
            else
            {
                // There were no previous lanes from which to calculate continuing lanes, so use current lanes for updating the previous lanes.
                previous_lanes = current_lanes;
            }

            if (lane_change_time != temporal::Time::max())
            {
                if (current_time - lane_change_time >= min_duration_threshold)
                {
                    if (lane_change_offset != 0)
                    {
                        std::shared_ptr<IEvent<int32_t>> lane_goal_value_event(new BasicEvent<int32_t>(lane_goal_value_variable->get_full_name(),
                                                                                                  lane_change_offset,
                                                                                                  action_start_time));
                        lane_goal_value_variable->add_event(lane_goal_value_event);

                        for (temporal::Time current_time_2 = action_start_time; current_time_2 <= current_time; current_time_2 += time_step)
                        {
                            std::shared_ptr<IEvent<temporal::Duration>> lane_goal_duration_event(new BasicEvent<temporal::Duration>(lane_goal_duration_variable->get_full_name(),
                                                                                                                               current_time - current_time_2,
                                                                                                                               current_time_2));
                            lane_goal_duration_variable->add_event(lane_goal_duration_event);
                        }
                    }
                    else
                    {
                        // We ended up in the same lane as we started with too little time in any other lane to consider a lane change to have occured.
                        // In other words, do nothing.
                    }

                    lane_change_time = temporal::Time::max();
                }
                else
                {
                    // We haven't been in this lane long enough to consider it an actual lane change. Keep waiting!
                }
            }
            else
            {
                action_start_time = current_time;
            }
        }
        catch (std::out_of_range)
        {
            // Attempt to get value from before variable was initialised
        }
    }

    this->variable_dict.update(lane_goal_value_variable->get_full_name(), lane_goal_value_variable);
    this->variable_dict.update(lane_goal_duration_variable->get_full_name(), lane_goal_duration_variable);
}

DrivingGoalExtractionAgent::DrivingGoalExtractionAgent(std::shared_ptr<const IDrivingAgent> driving_agent)
    : driving_agent(driving_agent)
{
    std::shared_ptr<const structures::IArray<std::shared_ptr<const IValuelessVariable>>> variables =
            driving_agent->get_variable_parameters();
    for(size_t i = 0; i < variables->count(); ++i)
    {
        this->variable_dict.update((*variables)[i]->get_full_name(), (*variables)[i]);
    }

    this->extract_aligned_linear_velocity_change_events();
}

DrivingGoalExtractionAgent::DrivingGoalExtractionAgent(std::shared_ptr<const IDrivingAgent> driving_agent,
                                                       std::shared_ptr<const map::IMap<std::string>> map)
    : DrivingGoalExtractionAgent(driving_agent)
{
    this->extract_lane_change_events(map);
}

std::string DrivingGoalExtractionAgent::get_name() const
{
    return this->driving_agent->get_name();
}

geometry::Vec DrivingGoalExtractionAgent::get_min_spatial_limits() const
{
    return this->driving_agent->get_min_spatial_limits();
}

geometry::Vec DrivingGoalExtractionAgent::get_max_spatial_limits() const
{
    return this->driving_agent->get_max_spatial_limits();
}

temporal::Time DrivingGoalExtractionAgent::get_min_temporal_limit() const
{
    return this->driving_agent->get_min_temporal_limit();
}

temporal::Time DrivingGoalExtractionAgent::get_max_temporal_limit() const
{
    return this->driving_agent->get_max_temporal_limit();
}

std::shared_ptr<structures::IArray<std::shared_ptr<const IValuelessConstant>>> DrivingGoalExtractionAgent::get_constant_parameters() const
{
    return this->driving_agent->get_constant_parameters();
}

std::shared_ptr<const IValuelessConstant> DrivingGoalExtractionAgent::get_constant_parameter(const std::string& constant_name) const
{
    return this->driving_agent->get_constant_parameter(constant_name);
}

std::shared_ptr<structures::IArray<std::shared_ptr<const IValuelessVariable>>> DrivingGoalExtractionAgent::get_variable_parameters() const
{
    return std::shared_ptr<structures::IArray<std::shared_ptr<const IValuelessVariable>>>(
            new structures::stl::STLStackArray<std::shared_ptr<const IValuelessVariable>>(variable_dict.get_values()));
}

std::shared_ptr<const IValuelessVariable> DrivingGoalExtractionAgent::get_variable_parameter(const std::string& variable_name) const
{
    return variable_dict[variable_name];
}

std::shared_ptr<structures::IArray<std::shared_ptr<const IValuelessEvent>>> DrivingGoalExtractionAgent::get_events() const
{
    std::shared_ptr<const structures::IArray<std::string>> variable_names = variable_dict.get_keys();

    std::shared_ptr<structures::stl::STLConcatArray<std::shared_ptr<const IValuelessEvent>>> events(
                new structures::stl::STLConcatArray<std::shared_ptr<const IValuelessEvent>>(variable_names->count()));

    size_t i;
    for(i = 0; i < variable_names->count(); ++i)
    {
        events->get_array(i) = variable_dict[(*variable_names)[i]]->get_valueless_events();
    }

    return events;
}

std::shared_ptr<IDrivingAgent> DrivingGoalExtractionAgent::driving_agent_deep_copy() const
{
    std::shared_ptr<DrivingGoalExtractionAgent> driving_agent(new DrivingGoalExtractionAgent());

    driving_agent->driving_agent = this->driving_agent;

    std::shared_ptr<const structures::IArray<std::string>> variable_names = this->variable_dict.get_keys();
    for(size_t i = 0; i < variable_names->count(); ++i)
    {
        driving_agent->variable_dict.update((*variable_names)[i], variable_dict[(*variable_names)[i]]->valueless_deep_copy());
    }

    return driving_agent;
}

std::shared_ptr<IDrivingAgentState> DrivingGoalExtractionAgent::get_driving_agent_state(temporal::Time time, bool throw_on_out_of_range) const
{
    std::shared_ptr<IDrivingAgentState> driving_agent_state =
            driving_agent->get_driving_agent_state(time, throw_on_out_of_range);

    std::shared_ptr<GoalDrivingAgentState> goal_driving_agent_state(new GoalDrivingAgentState(driving_agent_state));

    try
    {
        if (driving_agent_state->get_id_constant()->get_value() == 1310)
        {
            //std::cerr << "DrivingGoalExtractionAgent: " << this->get_variable_parameter(this->get_name() + ".aligned_linear_velocity.goal_value")->get_min_temporal_limit().time_since_epoch().count() << std::endl;
        }
        goal_driving_agent_state->set_aligned_linear_velocity_goal_value_variable(
                    std::shared_ptr<const IConstant<FP_DATA_TYPE>>(
                        new BasicConstant(
                            this->get_name(),
                            "aligned_linear_velocity.goal_value",
                            std::dynamic_pointer_cast<const IVariable<FP_DATA_TYPE>>(
                                this->get_variable_parameter(
                                    this->get_name() + ".aligned_linear_velocity.goal_value"))->get_value(time))));
        goal_driving_agent_state->set_aligned_linear_velocity_goal_duration_variable(
                    std::shared_ptr<const IConstant<temporal::Duration>>(
                        new BasicConstant(
                            this->get_name(),
                            "aligned_linear_velocity.goal_duration",
                            std::dynamic_pointer_cast<const IVariable<temporal::Duration>>(
                                this->get_variable_parameter(
                                    this->get_name() + ".aligned_linear_velocity.goal_duration"))->get_value(time))));
    }
    catch (std::out_of_range e)
    {
        //std::cerr << "At error time was: " << this->get_variable_parameter(this->get_name() + ".aligned_linear_velocity.goal_value")->get_min_temporal_limit().time_since_epoch().count() << std::endl;
        if (throw_on_out_of_range)
        {
            throw e;
        }
    }

    return goal_driving_agent_state;
}

}
}
}