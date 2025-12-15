#pragma once
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_dyntext_string.hpp"

namespace ui {
struct TwitchStringComponents {
    std::string user;
    std::string message;
};
struct MultiColorString {
    virtual void update_colors(esphome::Color &user,
                               esphome::Color &message) = 0;
};
inline TwitchStringComponents split_twitch_string(const std::string &input) {
    TwitchStringComponents result;
    std::size_t pos = input.find(':'); // Find the first ':' character
    if (pos != std::string::npos) {
        result.user = input.substr(0, pos); // Everything before ':' is the user
        std::size_t msg_start =
            pos + 1; // Everything after ':' (skipping a following
                     // space if present) is the message
        if (msg_start < input.size() && input[msg_start] == ' ')
            ++msg_start; // skip leading space after the colon
        result.message = input.substr(msg_start);
    } else {
        result.message =
            input; // No ':' found; treat the whole thing as the message
    }
    return result;
}

template <std::size_t BufSize>
class TwitchStringWidget : public DynStringWidget<BufSize>,
                           public MultiColorString {
  private:
    esphome::Color color_user;
    esphome::Color color_message;
    static constexpr const char *TAG = "ui_widget_twitchstring";

  public:
    void prep(std::string value, const char *fmt) override {
        DynStringWidget<BufSize>::prep(value, fmt);
    }
    void update_colors(esphome::Color &user, esphome::Color &message) {
        this->color_user = user;
        this->color_message = message;
    }

    void write() override {
        TwitchStringComponents abc = split_twitch_string(this->buf.data());
        if (abc.user == "" && abc.message == "")
            return;
        const int y = this->anchor.y - this->trim_pixels_top;
        if (this->right_align) {
            // printf will start drawing at the first pixel of a character,
            // ignoring leading whitespace in buffer.
            const int curr_buf_width = this->bounds(this->buf.data()).w;
            const int x_draw =
                this->anchor.x + (this->width() - curr_buf_width);
            ESP_LOGI(TAG,
                     "[widget=%s] write(): x_draw=%d, y=%d, anchor.x=%d, "
                     "this->width=%d, "
                     "curr_buff_width=%d",
                     this->get_name().c_str(), x_draw, y, this->anchor.x,
                     this->width(), this->bounds(this->buf.data()).w);
            ui::printf_dual(this->it, this->font, x_draw, y,
                            (abc.user + ": ").c_str(), WHITE,
                            abc.message.c_str(), YELLOW, this->prev_box);
        } else {
            ESP_LOGI(
                TAG, "[widget=%s] write(): y=%d, anchor.x=%d, this->width=%d",
                this->get_name().c_str(), y, this->anchor.x, this->width());
            ui::printf_dual(this->it, this->font, this->anchor.x, y,
                            (abc.user + ": ").c_str(), WHITE,
                            abc.message.c_str(), YELLOW, this->prev_box);
        }
    }
};
} // namespace ui
