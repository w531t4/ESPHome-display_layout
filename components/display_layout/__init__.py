# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT
from typing import Any, Dict, Optional

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import font
from esphome.components import globals as globals_component
from esphome.components import image as image_component
from esphome.components import sensor, text_sensor, time
from esphome.components.homeassistant import text_sensor as ha_text_sensor
from esphome.const import CONF_ID, CONF_NAME, CONF_TYPE

display_layout_ns = cg.esphome_ns.namespace("display_layout")
DisplayLayout = display_layout_ns.class_("DisplayLayout", cg.Component)
WidgetConfig = display_layout_ns.struct("WidgetConfig")

ui_ns = cg.global_ns.namespace("ui")
Coord = ui_ns.struct("Coord")

from .config import vars as v

MAX_WIDGETS = 16

# The globals component doesn't expose typed IDs, so accept any globals component
# here and rely on the configured C++ type to be bool.
GLOBALS_BOOL = globals_component.GlobalsComponent

MAGNET_MAP = {
    "left": "Magnet::LEFT",
    "right": "Magnet::RIGHT",
    "auto": "Magnet::AUTO",
}

WIDGET_TYPE_MAP = {
    "twitch_icons": "display_layout::WidgetKind::TWITCH_ICONS",
    "twitch_chat": "display_layout::WidgetKind::TWITCH_CHAT",
    "pixel_motion": "display_layout::WidgetKind::PIXEL_MOTION",
    "network_tput": "display_layout::WidgetKind::NETWORK_TPUT",
    "weather": "display_layout::WidgetKind::WEATHER",
    "temperatures": "display_layout::WidgetKind::TEMPERATURES",
    "date": "display_layout::WidgetKind::DATE",
    "time": "display_layout::WidgetKind::TIME",
    "ha_updates": "display_layout::WidgetKind::HA_UPDATES",
    "psn": "display_layout::WidgetKind::PSN",
}

BASE_WIDGET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TYPE): cv.one_of(*WIDGET_TYPE_MAP, lower=True),
        cv.Required(CONF_NAME): cv.string,
        cv.Required(v.CONF_ANCHOR): cv.Schema(
            {cv.Required(v.CONF_X): cv.int_, cv.Required(v.CONF_Y): cv.int_}
        ),
        cv.Optional(v.CONF_PRIORITY, default=0): cv.int_range(min=0, max=255),
        cv.Optional(v.CONF_MAGNET, default="right"): cv.one_of(*MAGNET_MAP, lower=True),
        cv.Optional(v.CONF_RESOURCE, default=""): cv.string_strict,
        cv.Optional(v.CONF_FONT): cv.use_id(font.Font),
        cv.Optional(v.CONF_FONT2): cv.use_id(font.Font),
        cv.Optional(v.CONF_PIXELS_PER_CHARACTER): cv.int_range(min=1),
        cv.Optional(v.CONF_ICON_WIDTH): cv.positive_int,
        cv.Optional(v.CONF_ICON_HEIGHT): cv.positive_int,
        cv.Optional(v.CONF_MAX_ICONS): cv.positive_int,
    }
)


def _require_font(value: Dict[str, Any]) -> Dict[str, Any]:
    if v.CONF_FONT not in value:
        raise cv.Invalid("font is required for this widget type")
    return value


def _require_font_pair(value: Dict[str, Any]) -> Dict[str, Any]:
    value = _require_font(value)
    if v.CONF_FONT2 not in value:
        raise cv.Invalid("font2 is required for this widget type")
    return value


WIDGET_SCHEMAS = {
    "twitch_icons": BASE_WIDGET_SCHEMA.extend(
        {
            cv.Required(v.CONF_ICON_WIDTH): cv.positive_int,
            cv.Required(v.CONF_ICON_HEIGHT): cv.positive_int,
            cv.Required(v.CONF_MAX_ICONS): cv.positive_int,
            cv.Optional(v.CONF_SOURCES): cv.Schema(
                {
                    cv.Required(v.CONF_IMAGE): cv.use_id(image_component.Image_),
                    cv.Required(v.CONF_COUNT): cv.use_id(text_sensor.TextSensor),
                    cv.Optional(v.CONF_READY_FLAG): cv.use_id(GLOBALS_BOOL),
                }
            ),
        }
    ),
    "twitch_chat": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(v.CONF_ROW1): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(v.CONF_ROW2): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(v.CONF_ROW3): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Optional(v.CONF_CHANNEL): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                    }
                )
            }
        ),
        _require_font,
    ),
    "pixel_motion": BASE_WIDGET_SCHEMA,
    "network_tput": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(v.CONF_RX): cv.use_id(sensor.Sensor),
                        cv.Required(v.CONF_TX): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "weather": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(v.CONF_VALUE): cv.use_id(text_sensor.TextSensor),
                        cv.Required(v.CONF_TIME): cv.use_id(time.RealTimeClock),
                    }
                )
            }
        ),
        _require_font,
    ),
    "temperatures": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(v.CONF_HIGH): cv.use_id(sensor.Sensor),
                        cv.Required(v.CONF_NOW): cv.use_id(sensor.Sensor),
                        cv.Required(v.CONF_LOW): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "date": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {cv.Required(v.CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "time": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {cv.Required(v.CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "ha_updates": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {cv.Required(v.CONF_VALUE): cv.use_id(sensor.Sensor)}
                )
            }
        ),
        _require_font,
    ),
    "psn": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(v.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(v.CONF_PHIL): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(v.CONF_NICK): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                    }
                )
            }
        ),
        _require_font,
    ),
}


def _validate_widget(value: Dict[str, Any]) -> Dict[str, Any]:
    # Validate only the discriminator first, so we can select the right schema
    widget_type = cv.one_of(*WIDGET_TYPE_MAP, lower=True)(value.get(CONF_TYPE))
    schema = WIDGET_SCHEMAS[widget_type]
    return schema(value)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DisplayLayout),
        cv.Optional(v.CONF_WIDGETS, default=[]): cv.All(
            cv.ensure_list(_validate_widget), cv.Length(max=MAX_WIDGETS)
        ),
        cv.Optional(v.CONF_GAP_X, default=0): cv.int_,
        cv.Optional(v.CONF_RIGHT_EDGE_X): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


def _opt(value: Optional[Any]) -> Any:
    return value if value is not None else cg.RawExpression("std::nullopt")


async def to_code(config: Dict[str, Any]) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_gap_x(config[v.CONF_GAP_X]))
    if v.CONF_RIGHT_EDGE_X in config:
        cg.add(var.set_right_edge_x(config[v.CONF_RIGHT_EDGE_X]))

    for widget in config.get(v.CONF_WIDGETS, []):
        font_expr = None
        font2_expr = None
        if v.CONF_FONT in widget:
            font_expr = await cg.get_variable(widget[v.CONF_FONT])
        if v.CONF_FONT2 in widget:
            font2_expr = await cg.get_variable(widget[v.CONF_FONT2])
        sources = widget.get(v.CONF_SOURCES, {})
        image_expr = None
        count_expr = None
        ready_flag_expr = None
        row1_expr = None
        row2_expr = None
        row3_expr = None
        channel_expr = None
        rx_expr = None
        tx_expr = None
        weather_expr = None
        time_expr = None
        high_expr = None
        now_expr = None
        low_expr = None
        updates_expr = None
        phil_expr = None
        nick_expr = None
        if sources:
            if v.CONF_IMAGE in sources:
                image_expr = await cg.get_variable(sources[v.CONF_IMAGE])
            if v.CONF_COUNT in sources:
                count_expr = await cg.get_variable(sources[v.CONF_COUNT])
            if v.CONF_READY_FLAG in sources:
                ready_flag_expr = await cg.get_variable(sources[v.CONF_READY_FLAG])
            if v.CONF_ROW1 in sources:
                row1_expr = await cg.get_variable(sources[v.CONF_ROW1])
            if v.CONF_ROW2 in sources:
                row2_expr = await cg.get_variable(sources[v.CONF_ROW2])
            if v.CONF_ROW3 in sources:
                row3_expr = await cg.get_variable(sources[v.CONF_ROW3])
            if v.CONF_CHANNEL in sources:
                channel_expr = await cg.get_variable(sources[v.CONF_CHANNEL])
            if v.CONF_RX in sources:
                rx_expr = await cg.get_variable(sources[v.CONF_RX])
            if v.CONF_TX in sources:
                tx_expr = await cg.get_variable(sources[v.CONF_TX])
            if v.CONF_VALUE in sources and widget[CONF_TYPE] == "weather":
                weather_expr = await cg.get_variable(sources[v.CONF_VALUE])
            if v.CONF_TIME in sources:
                time_expr = await cg.get_variable(sources[v.CONF_TIME])
            if v.CONF_HIGH in sources:
                high_expr = await cg.get_variable(sources[v.CONF_HIGH])
            if v.CONF_NOW in sources:
                now_expr = await cg.get_variable(sources[v.CONF_NOW])
            if v.CONF_LOW in sources:
                low_expr = await cg.get_variable(sources[v.CONF_LOW])
            if v.CONF_VALUE in sources and widget[CONF_TYPE] == "ha_updates":
                updates_expr = await cg.get_variable(sources[v.CONF_VALUE])
            if v.CONF_PHIL in sources:
                phil_expr = await cg.get_variable(sources[v.CONF_PHIL])
            if v.CONF_NICK in sources:
                nick_expr = await cg.get_variable(sources[v.CONF_NICK])
        cfg = cg.StructInitializer(
            WidgetConfig,
            ("kind", cg.RawExpression(WIDGET_TYPE_MAP[widget[CONF_TYPE]])),
            ("id", widget[CONF_NAME]),
            ("resource", widget.get(v.CONF_RESOURCE, "")),
            (
                "anchor",
                cg.RawExpression(
                    f"ui::Coord({widget[v.CONF_ANCHOR][v.CONF_X]}, "
                    f"{widget[v.CONF_ANCHOR][v.CONF_Y]})"
                ),
            ),
            ("priority", widget[v.CONF_PRIORITY]),
            ("magnet", cg.RawExpression(MAGNET_MAP[widget[v.CONF_MAGNET]])),
            ("font", _opt(font_expr)),
            ("font2", _opt(font2_expr)),
            ("pixels_per_character", _opt(widget.get(v.CONF_PIXELS_PER_CHARACTER))),
            ("icon_width", _opt(widget.get(v.CONF_ICON_WIDTH))),
            ("icon_height", _opt(widget.get(v.CONF_ICON_HEIGHT))),
            ("max_icons", _opt(widget.get(v.CONF_MAX_ICONS))),
            ("source_image", _opt(image_expr)),
            ("source_count", _opt(count_expr)),
            ("source_ready_flag", _opt(ready_flag_expr)),
            ("source_chat_row1", _opt(row1_expr)),
            ("source_chat_row2", _opt(row2_expr)),
            ("source_chat_row3", _opt(row3_expr)),
            ("source_chat_channel", _opt(channel_expr)),
            ("source_rx", _opt(rx_expr)),
            ("source_tx", _opt(tx_expr)),
            ("source_weather", _opt(weather_expr)),
            ("source_time", _opt(time_expr)),
            ("source_temp_high", _opt(high_expr)),
            ("source_temp_now", _opt(now_expr)),
            ("source_temp_low", _opt(low_expr)),
            ("source_updates", _opt(updates_expr)),
            ("source_psn_phil", _opt(phil_expr)),
            ("source_psn_nick", _opt(nick_expr)),
        )
        cg.add(var.add_widget_config(cfg))
