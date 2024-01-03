
#include <ori/simcars/agents/greedy_plan_fwd_car.hpp>

#include <ori/simcars/agents/fwd_car.hpp>
#include <ori/simcars/agents/control_fwd_car.hpp>

namespace ori
{
namespace simcars
{
namespace agents
{

void GreedyPlanFWDCar::init_links()
{
    pos.set_parent(control_fwd_car->get_fwd_car()->get_pos_variable());

    speed_val_goal.set_parent(&best_action_speed_goal_val);
    speed_time_goal.set_parent(&best_action_speed_goal_time);
    lane_val_goal.set_parent(&best_action_lane_goal_val);
    lane_time_goal.set_parent(&best_action_lane_goal_time);
}

GreedyPlanFWDCar::GreedyPlanFWDCar(map::IMap const *map,
                                   IFWDCarOutcomeSim const *fwd_car_outcome_sim,
                                   FWDCarSimParameters fwd_car_sim_parameters,
                                   IFWDCarRewardCalc const *fwd_car_reward_calc,
                                   FWDCarRewardParameters fwd_car_reward_parameters,
                                   FP_DATA_TYPE speed_min_value, FP_DATA_TYPE speed_max_value,
                                   FP_DATA_TYPE speed_interval_value,
                                   FP_DATA_TYPE time_horizon_value,
                                   FP_DATA_TYPE time_interval_value) :
    time_horizon(time_horizon_value),
    time_horizon_proxy(&time_horizon),
    time_interval(time_interval_value),
    time_options(&time_horizon_proxy, &time_interval),

    speed_min(speed_min_value),
    speed_min_proxy(&speed_min),
    speed_max(speed_max_value),
    speed_max_proxy(&speed_max),
    speed_interval(speed_interval_value),
    speed_options(&speed_min_proxy, &speed_max_proxy, &speed_interval),

    pos(),
    lane_options(&pos, map),

    actions(&time_options, &speed_options, &lane_options),

    sim_params(fwd_car_sim_parameters),
    sim_action_outcomes(&actions, &sim_params, fwd_car_outcome_sim),

    reward_params(fwd_car_reward_parameters),
    action_outcome_rewards(&sim_action_outcomes, &reward_params, fwd_car_reward_calc),

    best_action(&action_outcome_rewards),

    best_action_speed_goal(&best_action),
    best_action_speed_goal_val(&best_action_speed_goal),
    best_action_speed_goal_time(&best_action_speed_goal),

    best_action_lane_goal(&best_action),
    best_action_lane_goal_val(&best_action_lane_goal),
    best_action_lane_goal_time(&best_action_lane_goal)
{
}

}
}
}