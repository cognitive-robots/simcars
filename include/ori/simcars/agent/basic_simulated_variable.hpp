#pragma once

#include <ori/simcars/structures/stl/stl_concat_array.hpp>
#include <ori/simcars/temporal/temporal_rounding_dictionary.hpp>
#include <ori/simcars/agent/constant_interface.hpp>
#include <ori/simcars/agent/entity_state_interface.hpp>
#include <ori/simcars/agent/simulation_scene_interface.hpp>
#include <ori/simcars/agent/simulated_variable_abstract.hpp>
#include <ori/simcars/agent/basic_event.hpp>

#include <stdexcept>

namespace ori
{
namespace simcars
{
namespace agent
{

template <typename T>
class BasicSimulatedVariable : public virtual ASimulatedVariable<T>
{
    IVariable<T> const *original_variable;
    ISimulationScene *simulation_scene;

    mutable temporal::Time simulation_start_time;
    temporal::Time simulation_end_time;

    mutable temporal::TemporalRoundingDictionary<IEvent<T>*> time_event_dict;

    void simulation_check(temporal::Time time) const
    {
        temporal::Time simulation_target_time = std::min(time, simulation_end_time);
        if (time_event_dict.count() == 0)
        {
            if (simulation_target_time >= simulation_start_time + simulation_scene->get_time_step())
            {
                simulation_scene->simulate(simulation_target_time);
            }
        }
        else if (time_event_dict.get_latest_timestamp() + simulation_scene->get_time_step() <= simulation_target_time)
        {
            simulation_scene->simulate(simulation_target_time);
        }
    }

public:
    BasicSimulatedVariable(IVariable<T> const *original_variable,
                      ISimulationScene *simulation_scene,
                      temporal::Time simulation_start_time,
                      bool start_simulated,
                      temporal::Duration time_step) :
        BasicSimulatedVariable(original_variable, simulation_scene,
                          simulation_start_time,
                          original_variable->get_max_temporal_limit(),
                          start_simulated,
                          time_step) {}
    BasicSimulatedVariable(IVariable<T> const *original_variable,
                      ISimulationScene *simulation_scene,
                      temporal::Time simulation_start_time,
                      temporal::Time simulation_end_time,
                      bool start_simulated,
                      temporal::Duration time_step) :
        original_variable(original_variable), simulation_scene(simulation_scene),
        simulation_end_time(simulation_end_time),
        time_event_dict(time_step, nullptr)
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

        if (start_simulated)
        {
            this->simulation_start_time = simulation_start_time;
        }
        else
        {
            this->simulation_start_time = simulation_end_time;
        }
    }

    ~BasicSimulatedVariable()
    {
        structures::IArray<IEvent<T>*> const *events = time_event_dict.get_values();

        for (size_t i = 0; i < events->count(); ++i)
        {
            delete (*events)[i];
        }
    }

    /*
     * NOTE: This should not be called directly by external code, as the simulation scene will not have a pointer to the
     * resulting copy and thus any new simulation data will not be propogated to the copy.
     */
    ISimulatedVariable<T>* simulated_variable_deep_copy() const override
    {
        BasicSimulatedVariable<T> *variable =
                new BasicSimulatedVariable<T>(original_variable, simulation_scene, simulation_start_time,
                                         simulation_end_time, simulation_start_time < simulation_end_time,
                                         time_event_dict.get_time_window_step());

        size_t i;

        structures::IArray<temporal::Time> const *times = time_event_dict.get_keys();
        for(i = 0; i < times->count(); ++i)
        {
            IEvent<T> *event = time_event_dict[(*times)[i]];
            if (event != nullptr)
            {
                variable->time_event_dict.update((*times)[i], event->event_shallow_copy());
            }
        }

        variable->propogate_events_forward(this->get_max_temporal_limit());

        return variable;
    }

    std::string get_entity_name() const override
    {
        return original_variable->get_entity_name();
    }

    std::string get_variable_name() const override
    {
        return original_variable->get_variable_name();
    }

    IValuelessVariable::Type get_type() const override
    {
        return original_variable->get_type();
    }

    temporal::Time get_min_temporal_limit() const override
    {
        return original_variable->get_min_temporal_limit();
    }

    temporal::Time get_last_event_time() const override
    {
        if (time_event_dict.count() > 0)
        {
            return time_event_dict.get_latest_timestamp();
        }
        else
        {
            return std::min(original_variable->get_last_event_time(), simulation_end_time);
        }
    }

    temporal::Time get_max_temporal_limit() const override
    {
        return simulation_end_time;
    }

    bool has_event(temporal::Time time) const override
    {
        return time_event_dict.contains(time) || (time <= simulation_start_time && original_variable->has_event(time));
    }

    void propogate_events_forward() const override
    {
        time_event_dict.propogate_values_forward();
    }
    void propogate_events_forward(temporal::Time time_window_end) const override
    {
        time_event_dict.propogate_values_forward(time_window_end);
    }

    bool get_value(temporal::Time time, T &value) const override
    {
        simulation_check(time);

        if (time < simulation_start_time + simulation_scene->get_time_step())
        {
            return original_variable->get_value(time, value);
        }
        else
        {
            if (time_event_dict.contains(time))
            {
                value = time_event_dict[time]->get_value();
                return true;
            }
            else
            {
                time_event_dict.contains(time);
                return false;
            }
        }
    }

    structures::IArray<IEvent<T> const*>* get_events(
            temporal::Time time_window_start,
            temporal::Time time_window_end) const override
    {
        if (time_window_end < simulation_start_time + simulation_scene->get_time_step())
        {
            return original_variable->get_events(time_window_start, time_window_end);
        }

        if (time_window_start > this->get_max_temporal_limit())
        {
            return new structures::stl::STLStackArray<IEvent<T> const*>;
        }


        structures::IArray<IEvent<T> const*> *non_simulated_events = nullptr;
        if (time_window_start <= simulation_start_time)
        {
            non_simulated_events =
                    original_variable->get_events(time_window_start, time_window_end);
        }


        simulation_check(time_window_end);

        structures::IArray<IEvent<T>*> const *unfiltered_simulated_events =
                time_event_dict.get_values();
        structures::IStackArray<IEvent<T> const*> *filtered_simulated_events =
                new structures::stl::STLStackArray<IEvent<T> const*>;

        for (size_t i = 0; i < unfiltered_simulated_events->count(); ++i)
        {
            if ((*unfiltered_simulated_events)[i]->get_time() >= time_window_start
                    && (*unfiltered_simulated_events)[i]->get_time() <= time_window_end)
            {
                filtered_simulated_events->push_back((*unfiltered_simulated_events)[i]);
            }
        }


        if (non_simulated_events != nullptr)
        {
            structures::stl::STLConcatArray<IEvent<T> const*> *events =
                    new structures::stl::STLConcatArray<IEvent<T> const*>(2);

            events->get_array(0) = non_simulated_events;
            events->get_array(1) = filtered_simulated_events;

            return events;
        }
        else
        {
            return filtered_simulated_events;
        }
    }

    IEvent<T> const* get_event(temporal::Time time, bool exact) const override
    {
        simulation_check(time);

        if (time < simulation_start_time + simulation_scene->get_time_step())
        {
            return original_variable->get_event(time, exact);
        }
        else
        {
            IEvent<T> const *prospective_event = time_event_dict[time];
            if (!exact || prospective_event->get_time() == time)
            {
                return prospective_event;
            }
            else
            {
                return nullptr;
            }
        }
    }

    bool remove_value(temporal::Time time) override
    {
        if (time_event_dict.contains(time, true))
        {
            IEvent<T> *event = time_event_dict[time];
            time_event_dict.erase(time);
            delete event;
            return true;
        }
        else
        {
            return false;
        }
    }

    void set_value(temporal::Time time, T const &value) override
    {
        if (time > simulation_start_time
                && time <= simulation_end_time)
        {
            if (time_event_dict.contains(time))
            {
                IEvent<T> *other_event = time_event_dict[time];
                if (time == other_event->get_time())
                {
                    other_event->set_value(value);
                    return;
                }
            }

            time_event_dict.update(time,
                                   new BasicEvent(this->get_entity_name(),
                                                  this->get_parameter_name(),
                                                  value, time));
        }
    }

    structures::IArray<IEvent<T>*>* get_mutable_events(
            temporal::Time time_window_start,
            temporal::Time time_window_end) override
    {
        if (time_window_end < simulation_start_time + simulation_scene->get_time_step() ||
                time_window_start > this->get_max_temporal_limit())
        {
            return new structures::stl::STLStackArray<IEvent<T>*>;
        }

        simulation_check(time_window_end);

        structures::IArray<IEvent<T>*> const *unfiltered_simulated_events =
                time_event_dict.get_values();
        structures::IStackArray<IEvent<T>*> *filtered_simulated_events =
                new structures::stl::STLStackArray<IEvent<T>*>;

        for (size_t i = 0; i < unfiltered_simulated_events->count(); ++i)
        {
            if ((*unfiltered_simulated_events)[i]->get_time() >= time_window_start
                    && (*unfiltered_simulated_events)[i]->get_time() <= time_window_end)
            {
                filtered_simulated_events->push_back((*unfiltered_simulated_events)[i]);
            }
        }

        return filtered_simulated_events;
    }

    IEvent<T>* get_mutable_event(temporal::Time time, bool exact) override
    {
        simulation_check(time);

        if (time >= simulation_start_time + simulation_scene->get_time_step())
        {
            IEvent<T> *prospective_event = time_event_dict[time];
            if (!exact || prospective_event->get_time() == time)
            {
                return prospective_event;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    void begin_simulation(temporal::Time simulation_start_time) const override
    {
        if (this->simulation_start_time == this->simulation_end_time &&
                simulation_start_time <= original_variable->get_max_temporal_limit())
        {
            this->simulation_start_time = simulation_start_time;
        }
    }
};

}
}
}
