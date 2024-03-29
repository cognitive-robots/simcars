
#include <ori/simcars/structures/stl/stl_stack_array.hpp>
#include <ori/simcars/agent/defines.hpp>
#include <ori/simcars/agent/basic_constant.hpp>
#include <ori/simcars/agent/basic_event.hpp>
#include <ori/simcars/agent/basic_variable.hpp>
#include <ori/simcars/agent/csv/csv_scene.hpp>

#include <iostream>

namespace ori
{
namespace simcars
{
namespace agent
{
namespace csv
{

CSVScene* CSVScene::construct_from(IScene *scene)
{
    CSVScene *new_scene = new CSVScene();

    new_scene->min_spatial_limits = scene->get_min_spatial_limits();
    new_scene->max_spatial_limits = scene->get_max_spatial_limits();
    new_scene->time_step = scene->get_time_step();
    new_scene->min_temporal_limit = scene->get_min_temporal_limit();
    new_scene->max_temporal_limit = scene->get_max_temporal_limit();

    structures::IArray<IEntity*> *entities = scene->get_mutable_entities();

    size_t i;
    for(i = 0; i < entities->count(); ++i)
    {
        new_scene->entity_dict.update((*entities)[i]->get_name(),
                                      (*entities)[i]->entity_deep_copy());
    }

    delete entities;

    return new_scene;
}

IScene* CSVScene::scene_deep_copy() const
{
    CSVScene *new_scene = new CSVScene();

    new_scene->min_spatial_limits = this->min_spatial_limits;
    new_scene->max_spatial_limits = this->max_spatial_limits;
    new_scene->time_step = this->time_step;
    new_scene->min_temporal_limit = this->min_temporal_limit;
    new_scene->max_temporal_limit = this->max_temporal_limit;

    structures::IArray<IEntity*> const *entities = this->entity_dict.get_values();

    size_t i;
    for(i = 0; i < entities->count(); ++i)
    {
        new_scene->entity_dict.update((*entities)[i]->get_name(),
                                      (*entities)[i]->entity_deep_copy());
    }

    return new_scene;
}

void CSVScene::save_virt(std::ofstream &output_filestream) const
{
    structures::IArray<IEntity const*> *entities = this->get_entities();

    for (size_t i = 0; i < entities->count(); ++i)
    {
        structures::IArray<IValuelessConstant const*> *constants = (*entities)[i]->get_constant_parameters();
        structures::IArray<IValuelessVariable const*> *variables = (*entities)[i]->get_variable_parameters();

        size_t j;
        for (j = 0; j < variables->count(); ++j)
        {
            std::string variable_name = (*variables)[j]->get_full_name();
            output_filestream << variable_name << ";";
        }
        for (j = 0; j < constants->count(); ++j)
        {
            std::string constant_name = (*constants)[j]->get_full_name();
            output_filestream << constant_name << ";";
        }
        output_filestream << std::endl;

        temporal::Time current_time;
        for (current_time = min_temporal_limit; current_time <= max_temporal_limit; current_time += this->get_time_step())
        {
            for (j = 0; j < variables->count(); ++j)
            {
                std::string value_as_string;
                if ((*variables)[j]->get_value_as_string(current_time, value_as_string))
                {
                    output_filestream << value_as_string << ";";
                }
                else
                {
                    // Not the best solution, since it's not impossible a variable could take the value "NA"
                    output_filestream << "NA;";
                }
            }
            for (j = 0; j < constants->count(); ++j)
            {
                std::string value_as_string = (*constants)[j]->get_value_as_string();
                output_filestream << value_as_string << ";";
            }
            output_filestream << std::endl;
        }

        delete constants;
        delete variables;
    }

    delete entities;
}

void CSVScene::load_virt(std::ifstream &input_filestream, structures::ISet<std::string>* agent_names)
{
    throw utils::NotImplementedException();
}

geometry::Vec CSVScene::get_min_spatial_limits() const
{
    return this->min_spatial_limits;
}

geometry::Vec CSVScene::get_max_spatial_limits() const
{
    return this->max_spatial_limits;
}

temporal::Duration CSVScene::get_time_step() const
{
    return this->time_step;
}

temporal::Time CSVScene::get_min_temporal_limit() const
{
    return this->min_temporal_limit;
}

temporal::Time CSVScene::get_max_temporal_limit() const
{
    return this->max_temporal_limit;
}

structures::IArray<IEntity const*>* CSVScene::get_entities() const
{
    structures::stl::STLStackArray<IEntity const*> *entities =
            new structures::stl::STLStackArray<IEntity const*>(entity_dict.count());
    cast_array(*entity_dict.get_values(), *entities);
    return entities;
}

IEntity const* CSVScene::get_entity(std::string const &entity_name) const
{
    return entity_dict[entity_name];
}

ISceneState const* CSVScene::get_state(temporal::Time time) const
{
    throw utils::NotImplementedException();
}

structures::IArray<IEntity*>* CSVScene::get_mutable_entities()
{
    structures::stl::STLStackArray<IEntity*> *entities =
            new structures::stl::STLStackArray<IEntity*>(entity_dict.count());
    entity_dict.get_values(entities);
    return entities;
}

IEntity* CSVScene::get_mutable_entity(std::string const &entity_name)
{
    return entity_dict[entity_name];
}

ISceneState* CSVScene::get_mutable_state(temporal::Time time)
{
    throw utils::NotImplementedException();
}

}
}
}
}
