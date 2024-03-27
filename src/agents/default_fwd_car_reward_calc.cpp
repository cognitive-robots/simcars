
#include <ori/simcars/agents/default_fwd_car_reward_calc.hpp>

#include <cmath>

namespace ori
{
namespace simcars
{
namespace agents
{

FP_DATA_TYPE DefaultFWDCarRewardCalc::calc_reward(const FWDCarOutcome *outcome,
                                                  const FWDCarRewardParameters *parameters) const
{
    FWDCarRewards rewards = calc_rewards(outcome, parameters);

    return parameters->lane_transitions_weight * rewards.lane_transitions_reward +
            parameters->caution_weight * rewards.caution_reward +
            parameters->speed_limit_excess_weight * rewards.speed_limit_excess_reward +
            parameters->max_env_force_mag_weight * rewards.max_env_force_mag_reward +
            parameters->bias_weight;
}

FWDCarRewards DefaultFWDCarRewardCalc::calc_rewards(FWDCarOutcome const *outcome,
                                                    FWDCarRewardParameters const *parameters) const
{
    FWDCarRewards rewards;

    FP_DATA_TYPE lane_transitions_exp = std::exp(outcome->lane_transitions);
    rewards.lane_transitions_reward = lane_transitions_exp / (lane_transitions_exp + 1.0);

    rewards.caution_reward = std::exp(-0.1 * outcome->final_speed);

    rewards.speed_limit_excess_reward = std::exp(0.1 * (outcome->final_speed -
                                                        parameters->speed_limit));

    //rewards.final_speed_reward =
    //        std::exp(-8.0 * std::pow(outcome->final_speed - parameters->speed_limit, 2) /
    //                 std::pow(parameters->speed_limit, 2));

    //FP_DATA_TYPE max_env_force_mag_exp = std::exp(outcome->max_env_force_mag -
    //                                              parameters->env_force_mag_limit);
    //rewards.max_env_force_mag_reward = max_env_force_mag_exp / (max_env_force_mag_exp + 1.0);
    rewards.max_env_force_mag_reward =
            outcome->max_env_force_mag > parameters->env_force_mag_limit ? 0.0 : 1.0;

    return rewards;
}

}
}
}
