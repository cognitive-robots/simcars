#pragma once

#include <ori/simcars/structures/dictionary_interface.hpp>
#include <ori/simcars/structures/stl/stl_stack_array.hpp>

#include <unordered_map>
#include <memory>

namespace ori
{
namespace simcars
{
namespace structures
{
namespace stl
{

template <typename K, typename V, class K_hash = std::hash<K>, class K_equal = std::equal_to<K>>
class STLDictionary : public virtual IDictionary<K, V>
{
protected:
    std::unordered_map<K, V, K_hash, K_equal> data;

    mutable std::shared_ptr<IStackArray<K>> keys_cache;
    mutable std::shared_ptr<IStackArray<V>> values_cache;

public:
    STLDictionary(size_t bin_count = 10000) : data(bin_count), keys_cache(nullptr), values_cache(nullptr) {}
    STLDictionary(const STLDictionary<K, V, K_hash, K_equal>& stl_dictionary) : data(stl_dictionary.data) {}

    size_t count() const override
    {
        return data.size();
    }
    bool contains(const K& key) const override
    {
        return data.count(key) > 0;
    }

    const V& operator [](const K& key) const override
    {
        return data.at(key);
    }
    const bool contains_value(const V& val) const override
    {
        for (const auto& entry : data)
        {
            if (entry.second == val)
            {
                return true;
            }
        }

        return false;
    }
    std::shared_ptr<const IArray<K>> get_keys() const override
    {
        if (keys_cache)
        {
            return keys_cache;
        }
        else
        {
            keys_cache.reset(new STLStackArray<K>(count()));

            size_t i = 0;
            for (const auto& entry : data)
            {
                (*keys_cache)[i] = entry.first;
                ++i;
            }

            return keys_cache;
        }
    }
    std::shared_ptr<const IArray<V>> get_values() const override
    {
        if (values_cache)
        {
            return values_cache;
        }
        else
        {
            values_cache.reset(new STLStackArray<V>(count()));

            size_t i = 0;
            for (const auto& entry : data)
            {
                (*values_cache)[i] = entry.second;
                ++i;
            }

            return values_cache;
        }
    }

    void update(const K& key, const V& val) override
    {
        data[key] = val;

        keys_cache.reset();
        values_cache.reset();
    }
    void erase(const K& key) override
    {
        data.erase(key);

        keys_cache.reset();
        values_cache.reset();
    }
};

}
}
}
}
