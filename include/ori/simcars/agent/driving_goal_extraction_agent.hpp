#pragma once

#include <ori/simcars/structures/stl/stl_concat_array.hpp>
#include <ori/simcars/structures/stl/stl_dictionary.hpp>
#include <ori/simcars/map/map_interface.hpp>
#include <ori/simcars/map/living_lane_stack_array.hpp>
#include <ori/simcars/agent/defines.hpp>
#include <ori/simcars/agent/driving_agent_abstract.hpp>
#include <ori/simcars/agent/goal.hpp>
#include <ori/simcars/agent/basic_constant.hpp>
#include <ori/simcars/agent/basic_event.hpp>
#include <ori/simcars/agent/basic_variable.hpp>
#include <ori/simcars/agent/basic_goal_driving_agent_state.hpp>

namespace ori
{
namespace simcars
{
namespace agent
{

template <typename T_map_id>
class DrivingGoalExtractionAgent : public virtual ADrivingAgent
{
    IDrivingAgent *driving_agent;

    IVariable<Goal<FP_DATA_TYPE>> *aligned_linear_velocity_goal_variable = nullptr;
    IVariable<Goal<int32_t>> *lane_goal_variable = nullptr;

    IDrivingScene const *driving_scene;

    void extract_aligned_linear_velocity_change_events()
    {
        IVariable<FP_DATA_TYPE> const *aligned_linear_velocity_variable =
                this->get_aligned_linear_velocity_variable();
        IVariable<FP_DATA_TYPE> const *aligned_linear_acceleration_variable =
                this->get_aligned_linear_acceleration_variable();

        aligned_linear_velocity_goal_variable =
                    new BasicVariable<Goal<FP_DATA_TYPE>>(
                        this->get_name(), "aligned_linear_velocity", IValuelessVariable::Type::GOAL, driving_scene->get_time_step());

        temporal::Duration min_duration_threshold(temporal::DurationRep(1000.0 * MIN_ALIGNED_LINEAR_VELOCITY_CHANGE_DURATION_THRESHOLD));

        temporal::Time current_time;
        temporal::Time action_start_time = this->get_min_temporal_limit();
        FP_DATA_TYPE action_start_aligned_linear_velocity = std::numeric_limits<FP_DATA_TYPE>::max();
        for (current_time = this->get_min_temporal_limit() + this->get_scene()->get_time_step(); current_time <= this->get_max_temporal_limit(); current_time += this->get_scene()->get_time_step())
        {
            FP_DATA_TYPE current_aligned_linear_acceleration;
            if (!aligned_linear_acceleration_variable->get_value(current_time, current_aligned_linear_acceleration))
            {
                continue;
            }
            FP_DATA_TYPE current_aligned_linear_velocity;
            if (!aligned_linear_velocity_variable->get_value(current_time, current_aligned_linear_velocity))
            {
                continue;
            }
            FP_DATA_TYPE previous_aligned_linear_acceleration;
            if (!aligned_linear_acceleration_variable->get_value(current_time - this->get_scene()->get_time_step(), previous_aligned_linear_acceleration))
            {
                continue;
            }
            FP_DATA_TYPE previous_aligned_linear_velocity;
            if (!aligned_linear_velocity_variable->get_value(current_time - this->get_scene()->get_time_step(), previous_aligned_linear_velocity))
            {
                continue;
            }

            if (previous_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    // STATUS QUO

                    if (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit())
                    {
                        if ((current_time - action_start_time >= min_duration_threshold
                             && std::abs(current_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                                || action_start_time == this->get_min_temporal_limit())
                        {
                            aligned_linear_velocity_goal_variable->set_value(
                                        action_start_time,
                                        Goal(current_aligned_linear_velocity,
                                             current_time));
                        }
                    }
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (((current_time - this->get_scene()->get_time_step()) - action_start_time >= min_duration_threshold
                         && std::abs(previous_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time -
                                         this->get_scene()->get_time_step()));
                    }

                    action_start_time = current_time - this->get_scene()->get_time_step();
                    action_start_aligned_linear_velocity = previous_aligned_linear_velocity;
                }
                else
                {
                    if ((current_time - action_start_time >= min_duration_threshold
                         && std::abs(current_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time));
                    }
                }
            }
            else if (previous_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (((current_time - this->get_scene()->get_time_step()) - action_start_time >= min_duration_threshold
                         && std::abs(previous_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time -
                                         this->get_scene()->get_time_step()));
                    }

                    action_start_time = current_time - this->get_scene()->get_time_step();
                    action_start_aligned_linear_velocity = previous_aligned_linear_velocity;
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    // STATUS QUO

                    if (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit())
                    {
                        if ((current_time - action_start_time >= min_duration_threshold
                             && std::abs(current_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                                || action_start_time == this->get_min_temporal_limit())
                        {
                            aligned_linear_velocity_goal_variable->set_value(
                                        action_start_time,
                                        Goal(current_aligned_linear_velocity,
                                             current_time));
                        }
                    }
                }
                else
                {
                    if ((current_time - action_start_time >= min_duration_threshold
                         && std::abs(current_aligned_linear_velocity - action_start_aligned_linear_velocity) >= MIN_ALIGNED_LINEAR_VELOCITY_DIFF_THRESHOLD)
                            || (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit()
                                && action_start_time == this->get_min_temporal_limit()))
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time));
                    }
                }
            }
            else
            {
                if (current_aligned_linear_acceleration >= ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (action_start_time == this->get_min_temporal_limit())
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time -
                                         this->get_scene()->get_time_step()));
                    }

                    action_start_time = current_time - this->get_scene()->get_time_step();
                    action_start_aligned_linear_velocity = previous_aligned_linear_velocity;
                }
                else if (current_aligned_linear_acceleration <= -ACTION_BACKED_ACCELERATION_THRESHOLD)
                {
                    if (action_start_time == this->get_min_temporal_limit())
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time -
                                         this->get_scene()->get_time_step()));
                    }

                    action_start_time = current_time - this->get_scene()->get_time_step();
                    action_start_aligned_linear_velocity = previous_aligned_linear_velocity;
                }
                else
                {
                    // STATUS QUO

                    if (current_time + this->get_scene()->get_time_step() > this->get_max_temporal_limit() && action_start_time == this->get_min_temporal_limit())
                    {
                        aligned_linear_velocity_goal_variable->set_value(
                                    action_start_time,
                                    Goal(current_aligned_linear_velocity,
                                         current_time));
                    }
                }
            }
        }

        structures::IArray<IEvent<Goal<FP_DATA_TYPE>> const*> *events = aligned_linear_velocity_goal_variable->get_events();
        if (events->count() == 0 ||
                aligned_linear_velocity_goal_variable->get_min_temporal_limit() > this->get_min_temporal_limit())
        {
            FP_DATA_TYPE initial_aligned_linear_velocity;
            if (aligned_linear_velocity_variable->get_value(this->get_min_temporal_limit(), initial_aligned_linear_velocity))
            {
                aligned_linear_velocity_goal_variable->set_value(
                            this->get_min_temporal_limit(),
                            Goal(initial_aligned_linear_velocity,
                                 this->get_min_temporal_limit()));
            }
            else
            {
                throw std::runtime_error("Aligned linear velocity is not available at minimum temporal limit of agent");
            }
        }
        delete events;

        aligned_linear_velocity_goal_variable->propogate_events_forward(this->get_max_temporal_limit());
    }
    void extract_lane_change_events(map::IMap<T_map_id> const *map)
    {
        IVariable<geometry::Vec> const *position_variable = this->get_position_variable();

        temporal::Duration min_duration_threshold(temporal::DurationRep(1000.0 * MIN_LANE_CHANGE_DURATION_THRESHOLD));

        lane_goal_variable =
                    new BasicVariable<Goal<int32_t>>(
                        this->get_name(), "lane", IValuelessVariable::Type::GOAL, driving_scene->get_time_step());

        temporal::Time current_time;
        temporal::Time action_start_time;
        temporal::Time lane_change_time = temporal::Time::max();
        int32_t lane_change_offset = 0;
        map::ILaneArray<T_map_id> const *previous_lanes = nullptr;
        for (current_time = this->get_min_temporal_limit(); current_time <= this->get_max_temporal_limit(); current_time += this->get_scene()->get_time_step())
        {
            map::ILaneArray<T_map_id> const *current_lanes = nullptr;
            map::LivingLaneStackArray<T_map_id> *continuing_lanes = nullptr;
            map::LivingLaneStackArray<T_map_id> *left_lanes = nullptr;
            map::LivingLaneStackArray<T_map_id> *right_lanes = nullptr;

            geometry::Vec current_position;
            if (!position_variable->get_value(current_time, current_position))
            {
                continue;
            }

            current_lanes = map->get_encapsulating_lanes(current_position);

            if (current_lanes->count() == 0)
            {
                // Vehicle is currently not on a lane segment. This can happen quite often due to small gaps inbetween the lane segments. As such we just disregard this timestep.
                delete current_lanes;
                continue;
            }

            if (previous_lanes != nullptr)
            {
                continuing_lanes = new map::LivingLaneStackArray<T_map_id>;
                left_lanes = new map::LivingLaneStackArray<T_map_id>;
                right_lanes = new map::LivingLaneStackArray<T_map_id>;
                for (size_t j = 0; j < current_lanes->count(); ++j)
                {
                    map::ILane<T_map_id> const *current_lane = (*current_lanes)[j];
                    map::ILaneArray<T_map_id> const *current_lane_aft_lanes = current_lane->get_aft_lanes();
                    for (size_t k = 0; k < previous_lanes->count(); ++k)
                    {
                        map::ILane<T_map_id> const *previous_lane = (*previous_lanes)[k];
                        if (current_lane == previous_lane || (current_lane_aft_lanes != nullptr && current_lane_aft_lanes->contains(previous_lane)))
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
                        delete previous_lanes;
                        previous_lanes = left_lanes;
                        left_lanes = nullptr;
                    }
                    else if (right_lanes->count() > 0)
                    {
                        ++lane_change_offset;
                        lane_change_time = current_time;
                        delete previous_lanes;
                        previous_lanes = right_lanes;
                        right_lanes = nullptr;
                    }
                    else
                    {
                        // This should be an exceptionally rare occurance, the vehicle is occupying at least one valid lane, but none of the occupied lanes are adjacent to the previously occupied lanes.
                        // Because this is so rare, and difficult to handle, for now we essentially treat it as though the current lane occupation is a continuation of the previous lane occupation.
                        delete previous_lanes;
                        previous_lanes = current_lanes;
                        current_lanes = nullptr;
                    }
                }
                else
                {
                    // We've found a valid continuation for the previous lanes, so now update the previous lanes with the lanes found to be valid continuations.
                    delete previous_lanes;
                    previous_lanes = continuing_lanes;
                    continuing_lanes = nullptr;
                }
            }
            else
            {
                // There were no previous lanes from which to calculate continuing lanes, so use current lanes for updating the previous lanes.
                previous_lanes = current_lanes;
                current_lanes = nullptr;
            }

            if (lane_change_time != temporal::Time::max())
            {
                if (current_time - lane_change_time >= min_duration_threshold)
                {
                    if (lane_change_offset != 0)
                    {
                        lane_goal_variable->set_value(
                                    action_start_time,
                                    Goal(lane_change_offset,
                                         current_time));
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

            delete current_lanes;
            delete continuing_lanes;
            delete left_lanes;
            delete right_lanes;
        }
        delete previous_lanes;

        structures::IArray<IEvent<Goal<int32_t>> const*> *events = lane_goal_variable->get_events();
        if (events->count() == 0 ||
                lane_goal_variable->get_min_temporal_limit() >
                this->get_min_temporal_limit())
        {
            lane_goal_variable->set_value(
                        this->get_min_temporal_limit(),
                        Goal<int32_t>(0, this->get_min_temporal_limit()));
        }
        delete events;

        lane_goal_variable->propogate_events_forward(this->get_max_temporal_limit());
    }

protected:
    DrivingGoalExtractionAgent()
    {
    }

public:
    DrivingGoalExtractionAgent(IDrivingAgent *driving_agent,
                               IDrivingScene const *driving_scene)
        : driving_agent(driving_agent), driving_scene(driving_scene)
    {
        this->extract_aligned_linear_velocity_change_events();
    }
    DrivingGoalExtractionAgent(IDrivingAgent *driving_agent,
                               IDrivingScene const *driving_scene,
                               map::IMap<T_map_id> const *map)
        : DrivingGoalExtractionAgent(driving_agent, driving_scene)
    {
        this->extract_lane_change_events(map);
    }

    ~DrivingGoalExtractionAgent()
    {
        delete aligned_linear_velocity_goal_variable;
        delete lane_goal_variable;
    }

    std::string get_name() const override
    {
        return driving_agent->get_name();
    }

    geometry::Vec get_min_spatial_limits() const override
    {
        return driving_agent->get_min_spatial_limits();
    }
    geometry::Vec get_max_spatial_limits() const override
    {
        return driving_agent->get_max_spatial_limits();
    }

    temporal::Time get_min_temporal_limit() const override
    {
        return driving_agent->get_min_temporal_limit();
    }
    temporal::Time get_max_temporal_limit() const override
    {
        return driving_agent->get_max_temporal_limit();
    }

    structures::IArray<IValuelessConstant const*>* get_constant_parameters() const override
    {
        return driving_agent->get_constant_parameters();
    }
    IValuelessConstant const* get_constant_parameter(std::string const &constant_name) const override
    {
        return driving_agent->get_constant_parameter(constant_name);
    }

    structures::IArray<IValuelessVariable const*>* get_variable_parameters() const override
    {
        structures::stl::STLConcatArray<IValuelessVariable const*> *variables =
                    new structures::stl::STLConcatArray<IValuelessVariable const*>(2);

        structures::IStackArray<IValuelessVariable const*> *goal_extraction_variables =
                new structures::stl::STLStackArray<IValuelessVariable const*>;
        if (aligned_linear_velocity_goal_variable != nullptr)
        {
            goal_extraction_variables->push_back(aligned_linear_velocity_goal_variable);
        }
        if (lane_goal_variable != nullptr)
        {
            goal_extraction_variables->push_back(lane_goal_variable);
        }
        variables->get_array(0) = goal_extraction_variables;

        structures::IArray<IValuelessVariable const*> *unfiltered_other_variables =
                driving_agent->get_variable_parameters();
        structures::IStackArray<IValuelessVariable const*> *filtered_other_variables =
                new structures::stl::STLStackArray<IValuelessVariable const*>;

        bool found_variable;
        size_t i, j;
        for (i = 0; i < unfiltered_other_variables->count(); ++i)
        {
            found_variable = false;

            for (j = 0; j < goal_extraction_variables->count(); ++j)
            {
                if ((*goal_extraction_variables)[j]->get_parameter_name() ==
                        (*unfiltered_other_variables)[i]->get_parameter_name())
                {
                    found_variable = true;
                    break;
                }
            }

            if (!found_variable)
            {
                filtered_other_variables->push_back((*unfiltered_other_variables)[i]);
            }
        }

        variables->get_array(1) = filtered_other_variables;

        delete unfiltered_other_variables;

        return variables;
    }
    IValuelessVariable const* get_variable_parameter(std::string const &variable_name) const override
    {
        if (variable_name == this->get_name() + ".aligned_linear_velocity.goal")
        {
            return aligned_linear_velocity_goal_variable;
        }
        else if (variable_name == this->get_name() + ".lane.goal")
        {
            return lane_goal_variable;
        }
        else
        {
            return driving_agent->get_variable_parameter(variable_name);
        }
    }

    structures::IArray<IValuelessEvent const*>* get_events() const override
    {
        structures::IArray<IValuelessVariable const*> *variables = this->get_variable_parameters();

        structures::stl::STLConcatArray<IValuelessEvent const*> *events =
                    new structures::stl::STLConcatArray<IValuelessEvent const*>(
                        variables->count());

        size_t i;

        for(i = 0; i < variables->count(); ++i)
        {
            events->get_array(i) = (*variables)[i]->get_valueless_events();
        }

        delete variables;

        return events;
    }

    IDrivingAgent* driving_agent_deep_copy(IDrivingScene *driving_scene) const override
    {
        DrivingGoalExtractionAgent *driving_agent = new DrivingGoalExtractionAgent;

        driving_agent->driving_agent = this->driving_agent;

        if (this->aligned_linear_velocity_goal_variable != nullptr)
        {
            driving_agent->aligned_linear_velocity_goal_variable =
                    this->aligned_linear_velocity_goal_variable->variable_deep_copy();
        }
        if (this->lane_goal_variable != nullptr)
        {
            driving_agent->lane_goal_variable =
                    this->lane_goal_variable->variable_deep_copy();
        }

        if (driving_scene == nullptr)
        {
            driving_agent->driving_scene = this->driving_scene;
        }
        else
        {
            driving_agent->driving_scene = driving_scene;
        }

        return driving_agent;
    }

    IDrivingScene const* get_driving_scene() const override
    {
        return this->driving_scene;
    }

    IConstant<uint32_t> const* get_id_constant() const
    {
        return driving_agent->get_id_constant();
    }

    IConstant<bool> const* get_ego_constant() const
    {
        return driving_agent->get_ego_constant();
    }

    IConstant<FP_DATA_TYPE> const* get_bb_length_constant() const
    {
        return driving_agent->get_bb_length_constant();
    }

    IConstant<FP_DATA_TYPE> const* get_bb_width_constant() const
    {
        return driving_agent->get_bb_width_constant();
    }

    IConstant<DrivingAgentClass> const* get_driving_agent_class_constant() const
    {
        return driving_agent->get_driving_agent_class_constant();
    }

    IVariable<geometry::Vec> const* get_position_variable() const
    {
        return driving_agent->get_position_variable();
    }

    IVariable<geometry::Vec> const* get_linear_velocity_variable() const
    {
        return driving_agent->get_linear_velocity_variable();
    }

    IVariable<FP_DATA_TYPE> const* get_aligned_linear_velocity_variable() const
    {
        return driving_agent->get_aligned_linear_velocity_variable();
    }

    IVariable<geometry::Vec> const* get_linear_acceleration_variable() const
    {
        return driving_agent->get_linear_acceleration_variable();
    }

    IVariable<FP_DATA_TYPE> const* get_aligned_linear_acceleration_variable() const
    {
        return driving_agent->get_aligned_linear_acceleration_variable();
    }

    IVariable<geometry::Vec> const* get_external_linear_acceleration_variable() const
    {
        return driving_agent->get_external_linear_acceleration_variable();
    }

    IVariable<FP_DATA_TYPE> const* get_rotation_variable() const
    {
        return driving_agent->get_rotation_variable();
    }

    IVariable<FP_DATA_TYPE> const* get_steer_variable() const
    {
        return driving_agent->get_steer_variable();
    }

    IVariable<FP_DATA_TYPE> const* get_angular_velocity_variable() const
    {
        return driving_agent->get_angular_velocity_variable();
    }

    IVariable<temporal::Duration> const* get_ttc_variable() const
    {
        return driving_agent->get_ttc_variable();
    }

    IVariable<temporal::Duration> const* get_cumilative_collision_time_variable() const
    {
        return driving_agent->get_cumilative_collision_time_variable();
    }


    structures::IArray<IValuelessConstant*>* get_mutable_constant_parameters() override
    {
        return driving_agent->get_mutable_constant_parameters();
    }
    IValuelessConstant* get_mutable_constant_parameter(std::string const &constant_name) override
    {
        return driving_agent->get_mutable_constant_parameter(constant_name);
    }

    structures::IArray<IValuelessVariable*>* get_mutable_variable_parameters() override
    {
        structures::stl::STLConcatArray<IValuelessVariable*> *variables =
                    new structures::stl::STLConcatArray<IValuelessVariable*>(2);

        structures::IStackArray<IValuelessVariable*> *goal_extraction_variables =
                new structures::stl::STLStackArray<IValuelessVariable*>;
        if (aligned_linear_velocity_goal_variable != nullptr)
        {
            goal_extraction_variables->push_back(aligned_linear_velocity_goal_variable);
        }
        if (lane_goal_variable != nullptr)
        {
            goal_extraction_variables->push_back(lane_goal_variable);
        }
        variables->get_array(0) = goal_extraction_variables;

        structures::IArray<IValuelessVariable*> *unfiltered_other_variables =
                driving_agent->get_mutable_variable_parameters();
        structures::IStackArray<IValuelessVariable*> *filtered_other_variables =
                new structures::stl::STLStackArray<IValuelessVariable*>;

        bool found_variable;
        size_t i, j;
        for (i = 0; i < unfiltered_other_variables->count(); ++i)
        {
            found_variable = false;

            for (j = 0; j < goal_extraction_variables->count(); ++j)
            {
                if ((*goal_extraction_variables)[j]->get_parameter_name() ==
                        (*unfiltered_other_variables)[i]->get_parameter_name())
                {
                    found_variable = true;
                    break;
                }
            }

            if (!found_variable)
            {
                filtered_other_variables->push_back((*unfiltered_other_variables)[i]);
            }
        }

        variables->get_array(1) = filtered_other_variables;

        delete unfiltered_other_variables;

        return variables;
    }
    IValuelessVariable* get_mutable_variable_parameter(std::string const &variable_name) override
    {
        if (variable_name == this->get_name() + ".aligned_linear_velocity.goal")
        {
            return aligned_linear_velocity_goal_variable;
        }
        else if (variable_name == this->get_name() + ".lane.goal")
        {
            return lane_goal_variable;
        }
        else
        {
            return driving_agent->get_mutable_variable_parameter(variable_name);
        }
    }

    structures::IArray<IValuelessEvent*>* get_mutable_events() override
    {
        structures::IArray<IValuelessVariable*> *variables = this->get_mutable_variable_parameters();

        structures::stl::STLConcatArray<IValuelessEvent*> *events =
                    new structures::stl::STLConcatArray<IValuelessEvent*>(
                        variables->count());

        size_t i;

        for(i = 0; i < variables->count(); ++i)
        {
            events->get_array(i) = (*variables)[i]->get_mutable_valueless_events();
        }

        delete variables;

        return events;
    }

    IConstant<uint32_t>* get_mutable_id_constant()
    {
        return driving_agent->get_mutable_id_constant();
    }

    IConstant<bool>* get_mutable_ego_constant()
    {
        return driving_agent->get_mutable_ego_constant();
    }

    IConstant<FP_DATA_TYPE>* get_mutable_bb_length_constant()
    {
        return driving_agent->get_mutable_bb_length_constant();
    }

    IConstant<FP_DATA_TYPE>* get_mutable_bb_width_constant()
    {
        return driving_agent->get_mutable_bb_width_constant();
    }

    IConstant<DrivingAgentClass>* get_mutable_driving_agent_class_constant()
    {
        return driving_agent->get_mutable_driving_agent_class_constant();
    }

    IVariable<geometry::Vec>* get_mutable_position_variable()
    {
        return driving_agent->get_mutable_position_variable();
    }

    IVariable<geometry::Vec>* get_mutable_linear_velocity_variable()
    {
        return driving_agent->get_mutable_linear_velocity_variable();
    }

    IVariable<FP_DATA_TYPE>* get_mutable_aligned_linear_velocity_variable()
    {
        return driving_agent->get_mutable_aligned_linear_velocity_variable();
    }

    IVariable<geometry::Vec>* get_mutable_linear_acceleration_variable()
    {
        return driving_agent->get_mutable_linear_acceleration_variable();
    }

    IVariable<FP_DATA_TYPE>* get_mutable_aligned_linear_acceleration_variable()
    {
        return driving_agent->get_mutable_aligned_linear_acceleration_variable();
    }

    IVariable<geometry::Vec>* get_mutable_external_linear_acceleration_variable()
    {
        return driving_agent->get_mutable_external_linear_acceleration_variable();
    }

    IVariable<FP_DATA_TYPE>* get_mutable_rotation_variable()
    {
        return driving_agent->get_mutable_rotation_variable();
    }

    IVariable<FP_DATA_TYPE>* get_mutable_steer_variable()
    {
        return driving_agent->get_mutable_steer_variable();
    }

    IVariable<FP_DATA_TYPE>* get_mutable_angular_velocity_variable()
    {
        return driving_agent->get_mutable_angular_velocity_variable();
    }

    IVariable<temporal::Duration>* get_mutable_ttc_variable()
    {
        return driving_agent->get_mutable_ttc_variable();
    }

    IVariable<temporal::Duration>* get_mutable_cumilative_collision_time_variable()
    {
        return driving_agent->get_mutable_cumilative_collision_time_variable();
    }
};

}
}
}
