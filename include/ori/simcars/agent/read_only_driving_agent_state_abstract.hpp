#pragma once

#include <ori/simcars/agent/read_only_driving_agent_state_interface.hpp>

namespace ori
{
namespace simcars
{
namespace agent
{

class AReadOnlyDrivingAgentState : public virtual IReadOnlyDrivingAgentState
{
public:
    IConstant<uint32_t> const* get_id_constant() const override;
    IConstant<bool> const* get_ego_constant() const override;
    IConstant<FP_DATA_TYPE> const* get_bb_length_constant() const override;
    IConstant<FP_DATA_TYPE> const* get_bb_width_constant() const override;
    IConstant<DrivingAgentClass> const* get_driving_agent_class_constant() const override;
    IConstant<geometry::Vec> const* get_position_variable() const override;
    IConstant<geometry::Vec> const* get_linear_velocity_variable() const override;
    IConstant<FP_DATA_TYPE> const* get_aligned_linear_velocity_variable() const override;
    IConstant<geometry::Vec> const* get_linear_acceleration_variable() const override;
    IConstant<FP_DATA_TYPE> const* get_aligned_linear_acceleration_variable() const override;
    IConstant<geometry::Vec> const* get_external_linear_acceleration_variable() const override;
    IConstant<FP_DATA_TYPE> const* get_rotation_variable() const override;
    IConstant<FP_DATA_TYPE> const* get_steer_variable() const override;
    IConstant<FP_DATA_TYPE> const* get_angular_velocity_variable() const override;
    IConstant<temporal::Duration> const* get_ttc_variable() const override;
    IConstant<temporal::Duration> const* get_cumilative_collision_time_variable() const override;
};

}
}
}
