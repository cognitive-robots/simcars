#pragma once

#include <ori/simcars/structures/set_interface.hpp>
#include <ori/simcars/structures/stl/stl_stack_array.hpp>

#include <unordered_set>

namespace ori
{
namespace simcars
{
namespace structures
{
namespace stl
{

template <typename T>
class STLSet : public virtual ISet<T>
{
    std::unordered_set<T> data;

    mutable IStackArray<T> *values_cache;

public:
    STLSet(size_t bin_count = 10000) : data(bin_count), values_cache(nullptr) {}
    STLSet(std::initializer_list<T> init_list, size_t bin_count = 10000) :
        data(init_list, bin_count), values_cache(nullptr) {}
    STLSet(STLSet const &stl_set) : data(stl_set.data), values_cache(nullptr) {}
    STLSet(ISet<T> const *set, size_t bin_count = 10000) : data(bin_count), values_cache(nullptr)
    {
        IArray<T> const *array = set->get_array();
        for (size_t i; i < array->count(); ++i)
        {
            data.insert((*array)[i]);
        }
    }

    ~STLSet() override
    {
        delete values_cache;
    }

    size_t count() const override
    {
        return data.size();
    }
    bool contains(T const &val) const override
    {
        return data.count(val) > 0;
    }

    IArray<T> const* get_array() const override
    {
        if (values_cache != nullptr)
        {
            return values_cache;
        }
        else
        {
            values_cache = new STLStackArray<T>(count());
            get_array(values_cache);
            return values_cache;
        }
    }
    void get_array(IStackArray<T> *array) const override
    {
        size_t i = 0;
        for (T const &val : data)
        {
            if (i >= array->count())
            {
                array->push_back(val);
                ++i;
            }
            else
            {
                (*array)[i] = val;
                ++i;
            }
        }
    }

    void union_with(ISet<T> const *set) override
    {
        IArray<T> const *array = set->get_array();
        size_t i;
        for (i = 0; i < array->count(); ++i)
        {
            this->insert((*array)[i]);
        }
    }
    void intersect_with(ISet<T> const *set) override
    {
        IArray<T> const *array = this->get_array();
        size_t i;
        for (i = 0; i < array->count(); ++i)
        {
            if (!set->contains((*array)[i]))
            {
                this->erase((*array)[i]);
            }
        }
    }
    void difference_with(ISet<T> const *set) override
    {
        IArray<T> const *array;
        size_t i;
        if (this->count() <= set->count())
        {
            array = this->get_array();
            for (i = 0; i < array->count(); ++i)
            {
                if (set->contains((*array)[i]))
                {
                    this->erase((*array)[i]);
                }
            }
        }
        else
        {
            array = set->get_array();
            for (i = 0; i < array->count(); ++i)
            {
                this->erase((*array)[i]);
            }
        }
    }
    void insert(T const &val) override
    {
        data.insert(val);

        if (values_cache != nullptr)
        {
            values_cache->push_back(val);
        }
    }
    void erase(T const &val) override
    {
        data.erase(val);

        delete values_cache;
        values_cache = nullptr;
    }
};

}
}
}
}
