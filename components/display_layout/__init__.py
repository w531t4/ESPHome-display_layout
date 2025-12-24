# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import font
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

MAX_WIDGETS = 16

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
        cv.Optional(CONF_MAGNET, default="right"): cv.one_of(
            *MAGNET_MAP, lower=True
        ),
        cv.Optional(CONF_RESOURCE, default=""): cv.string_strict,
        cv.Optional(CONF_FONT): cv.use_id(font.Font),
        cv.Optional(CONF_FONT2): cv.use_id(font.Font),
        cv.Optional(CONF_PIXELS_PER_CHARACTER): cv.int_range(min=1),
        cv.Optional(CONF_ICON_WIDTH): cv.positive_int,
        cv.Optional(CONF_ICON_HEIGHT): cv.positive_int,
        cv.Optional(CONF_MAX_ICONS): cv.positive_int,
    }
)


def _require_font(value):
    if CONF_FONT not in value:
        raise cv.Invalid("font is required for this widget type")
    return value


def _require_font_pair(value):
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
        }
    ),
    "twitch_chat": cv.All(BASE_WIDGET_SCHEMA, _require_font),
    "pixel_motion": BASE_WIDGET_SCHEMA,
    "network_tput": cv.All(BASE_WIDGET_SCHEMA, _require_font),
    "weather": cv.All(BASE_WIDGET_SCHEMA, _require_font),
    "temperatures": cv.All(BASE_WIDGET_SCHEMA, _require_font),
    "date": cv.All(BASE_WIDGET_SCHEMA, _require_font_pair),
    "time": cv.All(BASE_WIDGET_SCHEMA, _require_font_pair),
    "ha_updates": cv.All(BASE_WIDGET_SCHEMA, _require_font),
    "psn": cv.All(BASE_WIDGET_SCHEMA, _require_font),
}


def _validate_widget(value):
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


def _opt(value):
    return value if value is not None else cg.RawExpression("std::nullopt")


async def to_code(config):
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
        )
        cg.add(var.add_widget_config(cfg))
