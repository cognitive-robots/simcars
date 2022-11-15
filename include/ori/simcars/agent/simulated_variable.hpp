#pragma once

#include <ori/simcars/structures/stl/stl_concat_array.hpp>
#include <ori/simcars/temporal/precedence_temporal_dictionary.hpp>
#include <ori/simcars/agent/simulated_valueless_variable_interface.hpp>
#include <ori/simcars/agent/constant_interface.hpp>
#include <ori/simcars/agent/state_interface.hpp>
#include <ori/simcars/agent/simulation_scene_interface.hpp>
#include <ori/simcars/agent/variable_abstract.hpp>
#include <ori/simcars/agent/basic_event.hpp>

#include <stdexcept>

namespace ori
{
namespace simcars
{
namespace agent
{

template <typename T>
class SimulatedVariable : public virtual AVariable<T>, public virtual ISimulatedValuelessVariable
{
    std::shared_ptr<const IVariable<T>> original_variable;
    std::shared_ptr<const ISimulationScene> simulation_scene;

    temporal::Time simulation_start_time;
    temporal::Time simulation_end_time;

    mutable temporal::PrecedenceTemporalDictionary<std::shared_ptr<const IEvent<T>>> time_event_dict;

    void simulation_check(temporal::Time time) const
    {
        temporal::Time simulation_target_time = std::min(time, simulation_end_time);
        if (time_event_dict.count() == 0)
        {
            if (simulation_target_time > simulation_start_time)
            {
                simulation_scene->simulate_and_propogate(simulation_target_time);
            }
        }
        else if (time_event_dict.get_latest_timestamp() < simulation_target_time)
        {
            simulation_scene->simulate_and_propogate(simulation_target_time);
        }
    }

public:
    SimulatedVariable(std::shared_ptr<const IVariable<T>> original_variable,
                      std::shared_ptr<const ISimulationScene> simulation_scene,
                      temporal::Time simulation_start_time,
                      size_t max_cache_size = 10) :
        SimulatedVariable(original_variable, simulation_scene, simulation_start_time, original_variable->get_max_temporal_limit(),
                          max_cache_size) {}

    SimulatedVariable(std::shared_ptr<const IVariable<T>> original_variable,
                      std::shared_ptr<const ISimulationScene> simulation_scene,
                      temporal::Time simulation_start_time, temporal::Time simulation_end_time,
                      size_t max_cache_size = 10) :
        original_variable(original_variable), simulation_scene(simulation_scene),
        simulation_start_time(simulation_start_time), simulation_end_time(simulation_end_time),
        time_event_dict(max_cache_size)
    {
        if (simulation_start_time < original_variable->get_min_temporal_limit())
        {
            throw std::invalid_argument("Simulation start time is before earliest event for original variable");
        }

        if (simulation_start_time > original_variable->get_max_temporal_limit())
        {
            throw std::invalid_argument("Simulation start time is after latest event for original variable");
        }

        if (simulation_start_time > simulation_end_time)
        {
            throw std::invalid_argument("Simulation start time is after simulation end time");
        }
    }

    std::shared_ptr<IValuelessVariable> valueless_deep_copy() const override
    {
        return deep_copy();
    }

    /*
     * NOTE: This should not be called directly by external code, as the simulation scene will not have a pointer to the
     * resulting copy and thus any new simulation data will not be propogated to the copy.
     */
    std::shared_ptr<IVariable<T>> deep_copy() const override
    {
        std::shared_ptr<SimulatedVariable<T>> variable(
                    new SimulatedVariable<T>(original_variable, simulation_scene, simulation_start_time,
                                             simulation_end_time, time_event_dict.get_max_cache_size()));
        size_t i;

        std::shared_ptr<const structures::IArray<temporal::Time>> times = time_event_dict.get_keys();
        for(i = 0; i < times->count(); ++i)
        {
            variable->time_event_dict.update((*times)[i], time_event_dict[(*times)[i]]->shallow_copy());
        }

        return variable;
    }

    std::shared_ptr<ISimulatedValuelessVariable> simulated_valueless_deep_copy() const override
    {
        return std::dynamic_pointer_cast<ISimulatedValuelessVariable>(deep_copy());
    }

    std::string get_entity_name() const override
    {
        return original_variable->get_entity_name();
    }

    std::string get_parameter_name() const override
    {
        return original_variable->get_parameter_name();
    }

    IValuelessVariable::Type get_type() const override
    {
        return original_variable->get_type();
    }

    temporal::Time get_min_temporal_limit() const override
    {
        return original_variable->get_min_temporal_limit();
    }

    temporal::Time get_max_temporal_limit() const override
    {
        return simulation_end_time;
    }

    T get_value(temporal::Time time) const override
    {
        simulation_check(time);

        if (time < simulation_start_time + simulation_scene->get_time_step())
        {
            return original_variable->get_value(time);
        }
        else
        {
            return time_event_dict[time]->get_value();
        }
    }

    std::shared_ptr<structures::IArray<std::shared_ptr<const IEvent<T>>>> get_events(
            temporal::Time time_window_start,
            temporal::Time time_window_end) const override
    {
        simulation_check(time_window_end);

        std::shared_ptr<structures::stl::STLConcatArray<std::shared_ptr<const IEvent<T>>>> events(
                    new structures::stl::STLConcatArray<std::shared_ptr<const IEvent<T>>>(2));

        events->get_array(0) = original_variable->get_events(time_window_start, time_window_end);

        std::shared_ptr<const structures::IArray<std::shared_ptr<const IEvent<T>>>> unfiltered_simulated_events =
                time_event_dict.get_values();
        std::shared_ptr<structures::IStackArray<std::shared_ptr<const IEvent<T>>>> filtered_simulated_events(
                new structures::stl::STLStackArray<std::shared_ptr<const IEvent<T>>>());

        for (size_t i = 0; i < unfiltered_simulated_events->count(); ++i)
        {
            if ((*unfiltered_simulated_events)[i]->get_time() >= time_window_start
                    && (*unfiltered_simulated_events)[i]->get_time() <= time_window_end)
            {
                filtered_simulated_events->push_back((*unfiltered_simulated_events)[i]);
            }
        }

        events->get_array(1) = filtered_simulated_events;

        return events;
    }

    std::shared_ptr<const IEvent<T>> get_event(temporal::Time time) const override
    {
        simulation_check(time);

        if (time < simulation_start_time + simulation_scene->get_time_step())
        {
            return original_variable->get_event(time);
        }
        else
        {
            std::shared_ptr<const IEvent<T>> prospective_event = time_event_dict[time];
            if (prospective_event->get_time() == time)
            {
                return prospective_event;
            }
            else
            {
                throw std::out_of_range("No event at specified time");
            }
        }
    }

    bool add_event(std::shared_ptr<const IEvent<T>> event) override
    {
        if (event->get_time() <= simulation_start_time
                || event->get_time() > simulation_end_time)
        {
            return false;
        }
        else
        {
            time_event_dict.update(event->get_time(), event);

            return true;
        }
    }

    bool remove_event(std::shared_ptr<const IEvent<T>> event) override
    {
        try
        {
            if (time_event_dict[event->get_time()] == event)
            {
                time_event_dict.erase(event->get_time());
                return true;
            }
            else
            {
                return false;
            }
        }
        catch (std::out_of_range)
        {
            return false;
        }
    }

    bool simulation_update(temporal::Time time, std::shared_ptr<const IState> state) const override
    {
        if (time <= simulation_start_time
                || time > simulation_end_time)
        {
            return false;
        }
        else
        {
            try
            {
                std::shared_ptr<const IValuelessConstant> valueless_parameter_value =
                        state->get_parameter_value(this->get_full_name());

                std::shared_ptr<const IConstant<T>> parameter_value =
                        std::dynamic_pointer_cast<const IConstant<T>>(valueless_parameter_value);
                std::shared_ptr<IEvent<T>> update_event(new BasicEvent(parameter_value, time));

                time_event_dict.update(update_event->get_time(), update_event);
            }
            catch (std::out_of_range)
            {
                return false;
            }

            return true;
        }
    }
};

}
}
}