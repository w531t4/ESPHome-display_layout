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

CONF_WIDGETS = "widgets"
CONF_ANCHOR = "anchor"
CONF_X = "x"
CONF_Y = "y"
CONF_PRIORITY = "priority"
CONF_MAGNET = "magnet"
CONF_RESOURCE = "resource"
CONF_FONT = "font"
CONF_FONT2 = "font2"
CONF_PIXELS_PER_CHARACTER = "pixels_per_character"
CONF_ICON_WIDTH = "icon_width"
CONF_ICON_HEIGHT = "icon_height"
CONF_MAX_ICONS = "max_icons"
CONF_GAP_X = "gap_x"
CONF_RIGHT_EDGE_X = "right_edge_x"
CONF_SOURCES = "sources"
CONF_IMAGE = "image"
CONF_COUNT = "count"
CONF_READY_FLAG = "ready_flag"
CONF_ROW1 = "row1"
CONF_ROW2 = "row2"
CONF_ROW3 = "row3"
CONF_CHANNEL = "channel"
CONF_RX = "rx"
CONF_TX = "tx"
CONF_VALUE = "value"
CONF_TIME = "time"
CONF_HIGH = "high"
CONF_NOW = "now"
CONF_LOW = "low"
CONF_PHIL = "phil"
CONF_NICK = "nick"

MAX_WIDGETS = 16

GLOBALS_BOOL = globals_component.GlobalsComponent.template(cg.bool_)

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
        cv.Required(CONF_ANCHOR): cv.Schema(
            {cv.Required(CONF_X): cv.int_, cv.Required(CONF_Y): cv.int_}
        ),
        cv.Optional(CONF_PRIORITY, default=0): cv.int_range(min=0, max=255),
        cv.Optional(CONF_MAGNET, default="right"): cv.one_of(*MAGNET_MAP, lower=True),
        cv.Optional(CONF_RESOURCE, default=""): cv.string_strict,
        cv.Optional(CONF_FONT): cv.use_id(font.Font),
        cv.Optional(CONF_FONT2): cv.use_id(font.Font),
        cv.Optional(CONF_PIXELS_PER_CHARACTER): cv.int_range(min=1),
        cv.Optional(CONF_ICON_WIDTH): cv.positive_int,
        cv.Optional(CONF_ICON_HEIGHT): cv.positive_int,
        cv.Optional(CONF_MAX_ICONS): cv.positive_int,
    }
)


def _require_font(value: Dict[str, Any]) -> Dict[str, Any]:
    if CONF_FONT not in value:
        raise cv.Invalid("font is required for this widget type")
    return value


def _require_font_pair(value: Dict[str, Any]) -> Dict[str, Any]:
    value = _require_font(value)
    if CONF_FONT2 not in value:
        raise cv.Invalid("font2 is required for this widget type")
    return value


WIDGET_SCHEMAS = {
    "twitch_icons": BASE_WIDGET_SCHEMA.extend(
        {
            cv.Required(CONF_ICON_WIDTH): cv.positive_int,
            cv.Required(CONF_ICON_HEIGHT): cv.positive_int,
            cv.Required(CONF_MAX_ICONS): cv.positive_int,
            cv.Optional(CONF_SOURCES): cv.Schema(
                {
                    cv.Required(CONF_IMAGE): cv.use_id(image_component.Image_),
                    cv.Required(CONF_COUNT): cv.use_id(text_sensor.TextSensor),
                    cv.Optional(CONF_READY_FLAG): cv.use_id(GLOBALS_BOOL),
                }
            ),
        }
    ),
    "twitch_chat": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(CONF_ROW1): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(CONF_ROW2): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(CONF_ROW3): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Optional(CONF_CHANNEL): cv.use_id(
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
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(CONF_RX): cv.use_id(sensor.Sensor),
                        cv.Required(CONF_TX): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "weather": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(CONF_VALUE): cv.use_id(text_sensor.TextSensor),
                        cv.Required(CONF_TIME): cv.use_id(time.RealTimeClock),
                    }
                )
            }
        ),
        _require_font,
    ),
    "temperatures": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(CONF_HIGH): cv.use_id(sensor.Sensor),
                        cv.Required(CONF_NOW): cv.use_id(sensor.Sensor),
                        cv.Required(CONF_LOW): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "date": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {cv.Required(CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "time": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {cv.Required(CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "ha_updates": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {cv.Required(CONF_VALUE): cv.use_id(sensor.Sensor)}
                )
            }
        ),
        _require_font,
    ),
    "psn": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(CONF_PHIL): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(CONF_NICK): cv.use_id(
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
    value = BASE_WIDGET_SCHEMA(value)
    schema = WIDGET_SCHEMAS[value[CONF_TYPE]]
    return schema(value)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DisplayLayout),
        cv.Optional(CONF_WIDGETS, default=[]): cv.All(
            cv.ensure_list(_validate_widget), cv.Length(max=MAX_WIDGETS)
        ),
        cv.Optional(CONF_GAP_X, default=0): cv.int_,
        cv.Optional(CONF_RIGHT_EDGE_X): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


def _opt(value: Optional[Any]) -> Any:
    return value if value is not None else cg.RawExpression("std::nullopt")


async def to_code(config: Dict[str, Any]) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_gap_x(config[CONF_GAP_X]))
    if CONF_RIGHT_EDGE_X in config:
        cg.add(var.set_right_edge_x(config[CONF_RIGHT_EDGE_X]))

    for widget in config.get(CONF_WIDGETS, []):
        font_expr = None
        font2_expr = None
        if CONF_FONT in widget:
            font_expr = await cg.get_variable(widget[CONF_FONT])
        if CONF_FONT2 in widget:
            font2_expr = await cg.get_variable(widget[CONF_FONT2])
        sources = widget.get(CONF_SOURCES, {})
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
            if CONF_IMAGE in sources:
                image_expr = await cg.get_variable(sources[CONF_IMAGE])
            if CONF_COUNT in sources:
                count_expr = await cg.get_variable(sources[CONF_COUNT])
            if CONF_READY_FLAG in sources:
                ready_flag_expr = await cg.get_variable(sources[CONF_READY_FLAG])
            if CONF_ROW1 in sources:
                row1_expr = await cg.get_variable(sources[CONF_ROW1])
            if CONF_ROW2 in sources:
                row2_expr = await cg.get_variable(sources[CONF_ROW2])
            if CONF_ROW3 in sources:
                row3_expr = await cg.get_variable(sources[CONF_ROW3])
            if CONF_CHANNEL in sources:
                channel_expr = await cg.get_variable(sources[CONF_CHANNEL])
            if CONF_RX in sources:
                rx_expr = await cg.get_variable(sources[CONF_RX])
            if CONF_TX in sources:
                tx_expr = await cg.get_variable(sources[CONF_TX])
            if CONF_VALUE in sources and widget[CONF_TYPE] == "weather":
                weather_expr = await cg.get_variable(sources[CONF_VALUE])
            if CONF_TIME in sources:
                time_expr = await cg.get_variable(sources[CONF_TIME])
            if CONF_HIGH in sources:
                high_expr = await cg.get_variable(sources[CONF_HIGH])
            if CONF_NOW in sources:
                now_expr = await cg.get_variable(sources[CONF_NOW])
            if CONF_LOW in sources:
                low_expr = await cg.get_variable(sources[CONF_LOW])
            if CONF_VALUE in sources and widget[CONF_TYPE] == "ha_updates":
                updates_expr = await cg.get_variable(sources[CONF_VALUE])
            if CONF_PHIL in sources:
                phil_expr = await cg.get_variable(sources[CONF_PHIL])
            if CONF_NICK in sources:
                nick_expr = await cg.get_variable(sources[CONF_NICK])
        cfg = cg.StructInitializer(
            WidgetConfig,
            ("kind", cg.RawExpression(WIDGET_TYPE_MAP[widget[CONF_TYPE]])),
            ("id", widget[CONF_NAME]),
            ("resource", widget.get(CONF_RESOURCE, "")),
            (
                "anchor",
                cg.RawExpression(
                    f"ui::Coord({widget[CONF_ANCHOR][CONF_X]}, "
                    f"{widget[CONF_ANCHOR][CONF_Y]})"
                ),
            ),
            ("priority", widget[CONF_PRIORITY]),
            ("magnet", cg.RawExpression(MAGNET_MAP[widget[CONF_MAGNET]])),
            ("font", _opt(font_expr)),
            ("font2", _opt(font2_expr)),
            ("pixels_per_character", _opt(widget.get(CONF_PIXELS_PER_CHARACTER))),
            ("icon_width", _opt(widget.get(CONF_ICON_WIDTH))),
            ("icon_height", _opt(widget.get(CONF_ICON_HEIGHT))),
            ("max_icons", _opt(widget.get(CONF_MAX_ICONS))),
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
