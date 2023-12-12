
#include <ori/simcars/agents/rect_rigid_body_sim.hpp>

namespace ori
{
namespace simcars
{
namespace agents
{

RectRigidBodySim::RectRigidBodySim(RectRigidBody const *rect_rigid_body,
                                   temporal::Time start_time) :
    RectRigidBody(*rect_rigid_body),

    sim_start_time(start_time),

    sim_lin_acc(&(this->lin_acc_buff), rect_rigid_body->get_lin_acc_variable(), &sim_start_time),
    sim_lin_vel(&(this->lin_vel_buff), rect_rigid_body->get_lin_vel_variable(), &sim_start_time),
    sim_pos(&(this->pos_buff), rect_rigid_body->get_pos_variable(), &sim_start_time),

    sim_ang_acc(&(this->ang_acc_buff), rect_rigid_body->get_ang_acc_variable(), &sim_start_time),
    sim_ang_vel(&(this->ang_vel_buff), rect_rigid_body->get_ang_vel_variable(), &sim_start_time),
    sim_rot(&(this->rot_buff), rect_rigid_body->get_rot_variable(), &sim_start_time)
{
    prev_lin_acc = simcars::causal::VectorPreviousTimeStepVariable(&sim_lin_acc);
    prev_lin_vel = simcars::causal::VectorPreviousTimeStepVariable(&sim_lin_vel);
    prev_pos = simcars::causal::VectorPreviousTimeStepVariable(&sim_pos);

    prev_ang_acc = simcars::causal::ScalarPreviousTimeStepVariable(&sim_ang_acc);
    prev_ang_vel = simcars::causal::ScalarPreviousTimeStepVariable(&sim_ang_vel);
    prev_rot = simcars::causal::ScalarPreviousTimeStepVariable(&sim_rot);
}

}
}
}
