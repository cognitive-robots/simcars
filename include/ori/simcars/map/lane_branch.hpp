#pragma once

#include <ori/simcars/structures/stl/stl_deque_array.hpp>
#include <ori/simcars/map/lane_interface.hpp>

namespace ori
{
namespace simcars
{
namespace map
{

class LaneBranch
{
    structures::IArray<uint64_t> *lane_ids;

    IMap const *map;

    mutable ILane const *straightest_lane;
    mutable structures::stl::STLDequeArray<ILane const*> left_curving_lanes;
    mutable structures::stl::STLDequeArray<ILane const*> right_curving_lanes;

    void retrieve_lanes() const;

public:
    LaneBranch(structures::IArray<uint64_t> *ids, IMap const *map);

    virtual ~LaneBranch();

    ILane const* operator [](size_t idx) const;
};

}
}
}