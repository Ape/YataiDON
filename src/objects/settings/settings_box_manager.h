#pragma once

#include "settings_box.h"
#include <memory>
#include <optional>
#include <vector>

class SettingsBoxManager {
private:
    std::vector<std::unique_ptr<SettingsBox>> boxes;
    int   selected_box_index;
    bool  box_selected;

public:
    explicit SettingsBoxManager(const rapidjson::Document& tmpl);
    ~SettingsBoxManager();

    // Returns true if the exit box was selected, otherwise false
    bool select_box();

    std::optional<Screens> pending_screen_change();

    void move_left();
    void move_right();
    void update(double current_time_ms);
    void draw();
};
