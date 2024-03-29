
#include <ori/simcars/geometry/rect.hpp>

#include <cmath>
#include <cassert>

namespace ori
{
namespace simcars
{
namespace geometry
{

Rect::Rect() : Rect(0.0f, 0.0f, 0.0f, 0.0f) {}

Rect::Rect(Vec origin, FP_DATA_TYPE width, FP_DATA_TYPE height)
    : origin(origin), half_width(0.5f * width), half_height(0.5f * height),
      half_span(0.5f * std::sqrt(std::pow(width, 2.0f) + std::pow(height, 2.0f))),
      calc_bounds_flag(true) {}

Rect::Rect(FP_DATA_TYPE min_x, FP_DATA_TYPE min_y, FP_DATA_TYPE max_x, FP_DATA_TYPE max_y)
    : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y), calc_bounds_flag(false)
{
    assert(max_x >= min_x);
    assert(max_y >= min_y);

    origin << (min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f;
    half_width = 0.5f * (max_x - min_x);
    half_height = 0.5f * (max_y - min_y);
    half_span = std::sqrt(std::pow(half_width, 2.0f) +
                          std::pow(half_height, 2.0f));
}

Rect::Rect(Vecs points)
    : Rect(points.row(0).minCoeff(), points.row(1).minCoeff(), points.row(0).maxCoeff(), points.row(1).maxCoeff()) {}

Rect::Rect(Rect const &rect)
    : origin(rect.origin), half_width(rect.half_width), half_height(rect.half_height),
      half_span(rect.half_span), min_x(rect.min_x), min_y(rect.min_y),
      max_x(rect.max_x), max_y(rect.max_y), calc_bounds_flag(rect.calc_bounds_flag) {}

Rect::Rect(Rect const &rect_1, Rect const &rect_2)
    : Rect(fmin(rect_1.get_min_x(), rect_2.get_min_x()),
           fmin(rect_1.get_min_y(), rect_2.get_min_y()),
           fmax(rect_1.get_max_x(), rect_2.get_max_x()),
           fmax(rect_1.get_max_y(), rect_2.get_max_y())) {}

FP_DATA_TYPE Rect::get_half_width() const
{
    return half_width;
}

FP_DATA_TYPE Rect::get_half_height() const
{
    return half_height;
}

void Rect::set_min_x(FP_DATA_TYPE min_x) const
{
    this->min_x = min_x;
}

void Rect::set_min_y(FP_DATA_TYPE min_y) const
{
    this->min_y = min_y;
}

void Rect::set_max_x(FP_DATA_TYPE max_x) const
{
    this->max_x = max_x;
}

void Rect::set_max_y(FP_DATA_TYPE max_y) const
{
    this->max_y = max_y;
}

void Rect::set_calc_bounds_flag() const
{
    calc_bounds_flag = true;
}

void Rect::calc_bounds() const
{
    calc_bounds_virt();
    calc_bounds_flag = false;
}

bool Rect::check_bounds(Vec const &point) const
{
    if ((point - origin).norm() <= half_span)
    {
        if (this->calc_bounds_flag) calc_bounds();
        return !(this->min_x > point.x() || this->max_x < point.x()
                 || this->min_y > point.y() || this->max_y < point.y());
    }
    else
    {
        return false;
    }
}

bool Rect::check_bounds(Rect const &rect) const
{
    if ((rect.origin - this->origin).norm() <= (this->half_span + rect.half_span))
    {
        if (this->calc_bounds_flag) this->calc_bounds();
        if (rect.calc_bounds_flag) rect.calc_bounds();
        return !(this->min_x > rect.max_x || this->max_x < rect.min_x
                 || this->min_y > rect.max_y || this->max_y < rect.min_y);
    }
    else
    {
        return false;
    }
}

bool Rect::check_collision_virt(Rect const &rect) const
{
    return this->check_bounds(rect);
}

void Rect::calc_bounds_virt() const
{
    min_x = origin.x() - half_width;
    min_y = origin.y() - half_height;
    max_x = origin.x() + half_width;
    max_y = origin.y() + half_height;
}

bool Rect::operator ==(Rect const &rect) const
{
    return this->origin == rect.origin && this->half_width == rect.half_width && this->half_height == rect.half_height;
}

Vec const& Rect::get_origin() const
{
    return origin;
}

FP_DATA_TYPE Rect::get_width() const
{
    return 2.0f * half_width;
}

FP_DATA_TYPE Rect::get_height() const
{
    return 2.0f * half_height;
}

FP_DATA_TYPE Rect::get_span() const
{
    return 2.0f * half_span;
}

FP_DATA_TYPE Rect::get_min_x() const
{
    return min_x;
}

FP_DATA_TYPE Rect::get_min_y() const
{
    return min_y;
}

FP_DATA_TYPE Rect::get_max_x() const
{
    return max_x;
}

FP_DATA_TYPE Rect::get_max_y() const
{
    return max_y;
}

bool Rect::check_collision(Rect const &rect) const
{
    return this->check_collision_virt(rect) && rect.check_collision_virt(*this);
}

void Rect::set_origin(Vec const &origin)
{
    this->origin = origin;
    set_calc_bounds_flag();
}

void Rect::set_width(FP_DATA_TYPE width)
{
    half_width = 0.5f * width;
    set_calc_bounds_flag();
}

void Rect::set_height(FP_DATA_TYPE height)
{
    half_height = 0.5f * height;
    set_calc_bounds_flag();
}

void Rect::translate(Vec translation)
{
    origin += translation;
    set_calc_bounds_flag();
}

bool Rect::check_encapsulation(Vec const &point) const
{
    return check_bounds(point);
}

}
}
}
