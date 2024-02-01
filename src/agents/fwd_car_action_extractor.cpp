
#include <ori/simcars/agents/fwd_car_action_extractor.hpp>

#include <ori/simcars/structures/stl/stl_deque_array.hpp>
#include <ori/simcars/map/lane_interface.hpp>
#include <ori/simcars/causal/variable_context.hpp>

namespace ori
{
namespace simcars
{
namespace agents
{

structures::IArray<TimeGoalPair<FP_DATA_TYPE>>* FWDCarActionExtractor::extract_speed_goals(
        agents::FWDCar *fwd_car) const
{
    structures::stl::STLDequeArray<TimeGoalPair<FP_DATA_TYPE>> *speed_goals =
            new structures::stl::STLDequeArray<TimeGoalPair<FP_DATA_TYPE>>;

    temporal::Time start_time = fwd_car->get_min_time();
    temporal::Time end_time = fwd_car->get_max_time();

    simcars::causal::IEndogenousVariable<FP_DATA_TYPE> *lon_lin_acc =
            fwd_car->get_lon_lin_acc_variable();
    simcars::causal::IEndogenousVariable<FP_DATA_TYPE> *lon_lin_vel =
            fwd_car->get_lon_lin_vel_variable();

    temporal::Time action_start_time = start_time;
    FP_DATA_TYPE action_start_lon_lin_vel = std::numeric_limits<FP_DATA_TYPE>::max();
    for (temporal::Time current_time = start_time +
         simcars::causal::VariableContext::get_time_step_size();
         current_time <= end_time;
         current_time += simcars::causal::VariableContext::get_time_step_size())
    {
        simcars::causal::VariableContext::set_current_time(current_time);

        FP_DATA_TYPE curr_lon_lin_acc;
        if (!lon_lin_acc->get_value(curr_lon_lin_acc))
        {
            continue;
        }
        FP_DATA_TYPE curr_lon_lin_vel;
        if (!lon_lin_vel->get_value(curr_lon_lin_vel))
        {
            continue;
        }

        simcars::causal::VariableContext::set_current_time(
                    current_time -
                    simcars::causal::VariableContext::get_time_step_size());

        FP_DATA_TYPE prev_lon_lin_acc;
        if (!lon_lin_acc->get_value(prev_lon_lin_acc))
        {
            continue;
        }
        FP_DATA_TYPE prev_lon_lin_vel;
        if (!lon_lin_vel->get_value(prev_lon_lin_vel))
        {
            continue;
        }


        if (prev_lon_lin_acc >= action_min_lon_lin_acc)
        {
            if (curr_lon_lin_acc >= action_min_lon_lin_acc)
            {
                // STATUS QUO

                if (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time)
                {
                    if ((current_time - action_start_time >= action_min_duration
                         && std::abs(curr_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                            || action_start_time == start_time)
                    {
                        Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time);
                        speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                          speed_goal));
                    }
                }
            }
            else if (curr_lon_lin_acc <= -action_min_lon_lin_acc)
            {
                if (((current_time - simcars::causal::VariableContext::get_time_step_size()) - action_start_time >= action_min_duration
                     && std::abs(prev_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                        || (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time
                            && action_start_time == start_time))
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time -
                                                  simcars::causal::VariableContext::get_time_step_size());
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }

                action_start_time = current_time - simcars::causal::VariableContext::get_time_step_size();
                action_start_lon_lin_vel = prev_lon_lin_vel;
            }
            else
            {
                if ((current_time - action_start_time >= action_min_duration
                     && std::abs(curr_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                        || (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time
                            && action_start_time == start_time))
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time);
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }
            }
        }
        else if (prev_lon_lin_acc <= -action_min_lon_lin_acc)
        {
            if (curr_lon_lin_acc >= action_min_lon_lin_acc)
            {
                if (((current_time - simcars::causal::VariableContext::get_time_step_size()) - action_start_time >= action_min_duration
                     && std::abs(prev_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                        || (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time
                            && action_start_time == start_time))
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time -
                                                  simcars::causal::VariableContext::get_time_step_size());
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }

                action_start_time = current_time - simcars::causal::VariableContext::get_time_step_size();
                action_start_lon_lin_vel = prev_lon_lin_vel;
            }
            else if (curr_lon_lin_acc <= -action_min_lon_lin_acc)
            {
                // STATUS QUO

                if (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time)
                {
                    if ((current_time - action_start_time >= action_min_duration
                         && std::abs(curr_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                            || action_start_time == start_time)
                    {
                        Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time);
                        speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                          speed_goal));
                    }
                }
            }
            else
            {
                if ((current_time - action_start_time >= action_min_duration
                     && std::abs(curr_lon_lin_vel - action_start_lon_lin_vel) >= action_min_lon_lin_vel_diff)
                        || (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time
                            && action_start_time == start_time))
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time);
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }
            }
        }
        else
        {
            if (curr_lon_lin_acc >= action_min_lon_lin_acc)
            {
                if (action_start_time == start_time)
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time -
                                                  simcars::causal::VariableContext::get_time_step_size());
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }

                action_start_time = current_time - simcars::causal::VariableContext::get_time_step_size();
                action_start_lon_lin_vel = prev_lon_lin_vel;
            }
            else if (curr_lon_lin_acc <= -action_min_lon_lin_acc)
            {
                if (action_start_time == start_time)
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time -
                                                  simcars::causal::VariableContext::get_time_step_size());
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }

                action_start_time = current_time - simcars::causal::VariableContext::get_time_step_size();
                action_start_lon_lin_vel = prev_lon_lin_vel;
            }
            else
            {
                // STATUS QUO

                if (current_time + simcars::causal::VariableContext::get_time_step_size() > end_time && action_start_time == start_time)
                {
                    Goal<FP_DATA_TYPE> speed_goal(curr_lon_lin_vel, current_time);
                    speed_goals->push_back(TimeGoalPair<FP_DATA_TYPE>(action_start_time,
                                                                      speed_goal));
                }
            }
        }

        if (speed_goals->count() == 0 || (*speed_goals)[0].first > start_time)
        {
            simcars::causal::VariableContext::set_current_time(start_time);

            FP_DATA_TYPE init_lon_lin_vel;
            if (lon_lin_vel->get_value(init_lon_lin_vel))
            {
                Goal<FP_DATA_TYPE> speed_goal(init_lon_lin_vel, start_time);
                speed_goals->push_front(TimeGoalPair<FP_DATA_TYPE>(start_time,
                                                                   speed_goal));
            }
            else
            {
                throw std::runtime_error("Longitudinal linear velocity is not available at start "
                                         "time of agent");
            }
        }
    }

    return speed_goals;
}

structures::IArray<TimeGoalPair<uint64_t>>* FWDCarActionExtractor::extract_lane_goals(
        agents::FWDCar *fwd_car) const
{
    structures::stl::STLDequeArray<TimeGoalPair<uint64_t>> *lane_goals =
            new structures::stl::STLDequeArray<TimeGoalPair<uint64_t>>;

    temporal::Time start_time = fwd_car->get_min_time();
    temporal::Time end_time = fwd_car->get_max_time();

    simcars::causal::IEndogenousVariable<geometry::Vec> *pos =
            fwd_car->get_pos_variable();

    temporal::Time lane_change_time = start_time;
    structures::IArray<map::ILane const*> *prev_lanes = nullptr;
    for (temporal::Time current_time = start_time +
         simcars::causal::VariableContext::get_time_step_size();
         current_time <= end_time;
         current_time += simcars::causal::VariableContext::get_time_step_size())
    {
        geometry::Vec curr_pos;
        if (!pos->get_value(curr_pos))
        {
            continue;
        }

        structures::IArray<map::ILane const*> *curr_lanes = map->get_encapsulating_lanes(curr_pos);

        if (curr_lanes->count() == 0)
        {
            delete curr_lanes;
            continue;
        }

        structures::stl::STLStackArray<map::ILane const*> *cont_lanes = nullptr;
        structures::stl::STLStackArray<map::ILane const*> *left_lanes = nullptr;
        structures::stl::STLStackArray<map::ILane const*> *right_lanes = nullptr;
        if (prev_lanes != nullptr)
        {
            cont_lanes = new structures::stl::STLStackArray<map::ILane const*>;
            left_lanes = new structures::stl::STLStackArray<map::ILane const*>;
            right_lanes = new structures::stl::STLStackArray<map::ILane const*>;
            for (size_t j = 0; j < curr_lanes->count(); ++j)
            {
                map::ILane const *curr_lane = (*curr_lanes)[j];
                map::LaneBranch const *curr_lane_aft_lanes =
                        curr_lane->get_aft_lane_branch();
                for (size_t k = 0; k < prev_lanes->count(); ++k)
                {
                    map::ILane const *prev_lane = (*prev_lanes)[k];
                    if (curr_lane == prev_lane || curr_lane_aft_lanes->contains(prev_lane))
                    {
                        cont_lanes->push_back(curr_lane);
                    }
                    else if (curr_lane->get_right_adjacent_lane() == prev_lane)
                    {
                        left_lanes->push_back(curr_lane);
                    }
                    else if (curr_lane->get_left_adjacent_lane() == prev_lane)
                    {
                        right_lanes->push_back(curr_lane);
                    }
                }
            }

            if (cont_lanes->count() == 0)
            {
                if (left_lanes->count() > 0)
                {
                    lane_change_time = current_time;
                    delete prev_lanes;
                    prev_lanes = left_lanes;
                    left_lanes = nullptr;
                }
                else if (right_lanes->count() > 0)
                {
                    lane_change_time = current_time;
                    delete prev_lanes;
                    prev_lanes = right_lanes;
                    right_lanes = nullptr;
                }
                else
                {
                    // This should be an exceptionally rare occurance, the vehicle is occupying at
                    // least one valid lane, but none of the occupied lanes are adjacent to the
                    // previously occupied lanes.
                    // Because this is so rare, and difficult to handle, for now we essentially
                    // treat it as though the current lane occupation is a continuation of the
                    // previous lane occupation.
                    delete prev_lanes;
                    prev_lanes = curr_lanes;
                    curr_lanes = nullptr;
                }
            }
            else
            {
                // We've found a valid continuation for the previous lanes, so now update the
                // previous lanes with the lanes found to be valid continuations.
                lane_change_time = current_time;
                delete prev_lanes;
                prev_lanes = cont_lanes;
                cont_lanes = nullptr;
            }
        }
        else
        {
            // There were no previous lanes from which to calculate continuing lanes, so use
            // current lanes for updating the previous lanes.
            prev_lanes = curr_lanes;
            curr_lanes = nullptr;
        }

        if (lane_change_time != temporal::Time::max())
        {
            if (lane_goals->count() == 0)
            {
                // WARNING: Takes first lane as the lane goal
                Goal<uint64_t> lane_goal((*curr_lanes)[0]->get_id(), lane_change_time);
                lane_goals->push_back(TimeGoalPair<uint64_t>(
                                          start_time,
                                          lane_goal
                                          ));
            }
            else if (current_time - lane_change_time >= lane_min_duration &&
                     lane_change_time - action_min_duration >
                     (*lane_goals)[lane_goals->count() - 1].first)
            {
                if ((*lane_goals)[lane_goals->count() - 1].second.val != (*curr_lanes)[0]->get_id())
                {
                    // WARNING: Takes first lane as the lane goal
                    Goal<uint64_t> lane_goal((*curr_lanes)[0]->get_id(), lane_change_time);
                    lane_goals->push_back(TimeGoalPair<uint64_t>(
                                              lane_change_time - action_min_duration,
                                              lane_goal
                                              ));
                }
                else
                {
                    // We ended up in the same lane as we started with too little time in any other
                    // lane to consider a lane change to have occured. In other words, do nothing.
                }

                lane_change_time = temporal::Time::max();
            }
            else
            {
                // We haven't been in this lane long enough to consider it an actual lane change.
                // Keep waiting!
            }
        }

        delete curr_lanes;
        if (cont_lanes != nullptr) delete cont_lanes;
        if (left_lanes != nullptr) delete left_lanes;
        if (right_lanes != nullptr) delete right_lanes;
    }
    delete prev_lanes;

    if (lane_goals->count() == 0)
    {
        throw std::runtime_error("Lane is not available at start time of agent");
    }

    return lane_goals;
}

FWDCarActionExtractor::FWDCarActionExtractor(map::IMap *map,
                                             temporal::Duration action_min_duration,
                                             FP_DATA_TYPE action_min_lon_lin_acc,
                                             FP_DATA_TYPE action_min_lon_lin_vel_diff,
                                             temporal::Duration lane_min_duration) :
    map(map), action_min_duration(action_min_duration),
    action_min_lon_lin_acc(action_min_lon_lin_acc),
    action_min_lon_lin_vel_diff(action_min_lon_lin_vel_diff), lane_min_duration(lane_min_duration)
{}

structures::IArray<TimeFWDCarActionPair>* FWDCarActionExtractor::extract_actions(
        agents::FWDCar *fwd_car) const
{
    structures::stl::STLStackArray<TimeFWDCarActionPair> *actions =
            new structures::stl::STLStackArray<TimeFWDCarActionPair>;

    structures::IArray<TimeGoalPair<FP_DATA_TYPE>> *speed_goals = extract_speed_goals(fwd_car);
    structures::IArray<TimeGoalPair<uint64_t>> *lane_goals = extract_lane_goals(fwd_car);

    size_t i = 0, j = 0;
    do
    {
        FWDCarAction action((*speed_goals)[i].second, (*lane_goals)[j].second);
        temporal::Time start_time = std::max((*speed_goals)[i].first, (*lane_goals)[j].first);
        actions->push_back(TimeFWDCarActionPair(start_time, action));

        if (i == speed_goals->count() - 1)
        {
            ++j;
        }
        else if (j == lane_goals->count() - 1)
        {
            ++i;
        }
        else
        {
            if ((*speed_goals)[i + 1].first == (*lane_goals)[j + 1].first)
            {
                ++i;
                ++j;
            }
            else if ((*speed_goals)[i + 1].first < (*lane_goals)[j + 1].first)
            {
                ++i;
            }
            else
            {
                ++j;
            }
        }
    }
    while (i < speed_goals->count() - 1 || j < lane_goals->count() - 1);

    delete speed_goals;
    delete lane_goals;

    return actions;
}

}
}
}