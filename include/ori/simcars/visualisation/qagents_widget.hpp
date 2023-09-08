#pragma once

#include <ori/simcars/agents/fwd_car.hpp>
#include <ori/simcars/visualisation/qsfml_canvas_abstract.hpp>

#include <mutex>

namespace ori
{
namespace simcars
{
namespace visualisation
{

class QAgentsWidget : public AQSFMLCanvas, public structures::stl::STLSet<agents::FWDCar*>
{
public:
    enum class FocusMode
    {
        FIXED = 0,
        ALL_AGENTS = 1,
        FOCAL_AGENTS = 2
    };

private:
    //bool text_enabled;
    //sf::Font text_font;

    std::chrono::milliseconds frame_interval;
    FP_DATA_TYPE realtime_factor;
    FP_DATA_TYPE pixels_per_metre;

    FocusMode focus_mode;
    geometry::Vec focal_position;
    size_t focal_agent_count;
    //structures::IArray<std::string> const *focal_entities;

    temporal::Time start_time;
    temporal::Time end_time;
    temporal::Time current_time;

    bool update_required;

    std::chrono::time_point<std::chrono::steady_clock> last_realtime;

    structures::stl::STLStackArray<sf::Drawable const*> render_stack;

    void add_agent_to_render_stack(agents::FWDCar *agent);

protected:

    void on_init() final;
    void on_update() final;

    void add_to_render_stack(sf::Drawable const *drawable);
    void add_agents_to_render_stack();

    virtual void populate_render_stack();

public:
    QAgentsWidget(QWidget *parent, QPoint const &position, QSize const &size,
                  temporal::Time start_time, temporal::Time end_time,
                  std::chrono::milliseconds frame_interval = std::chrono::milliseconds(100),
                  FP_DATA_TYPE realtime_factor = 1.0f, FP_DATA_TYPE pixels_per_metre = 10.0,
                  FocusMode focus_mode = FocusMode::ALL_AGENTS);

    ~QAgentsWidget();

    FP_DATA_TYPE get_realtime_factor() const;
    FP_DATA_TYPE get_pixels_per_metre() const;
    FocusMode get_focus_mode() const;
    geometry::Vec const& get_focal_position() const;
    //structures::IArray<std::string> const* get_focal_entities() const;
    temporal::Time get_time() const;

    void set_realtime_factor(FP_DATA_TYPE realtime_factor);
    void set_pixels_per_metre(FP_DATA_TYPE pixels_per_metre);
    void set_focus_mode(FocusMode focus_mode);
    void set_focal_position(geometry::Vec const &focal_position);
    //void set_focal_entities(structures::IArray<std::string> const *focal_entities);
    void set_time(temporal::Time time);

public slots:
    void tick_forwards();
    void tick_backwards();
};

}
}
}