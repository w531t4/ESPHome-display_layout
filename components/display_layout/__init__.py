# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT
from typing import Any, Dict

from .config import const
from .config.maps import MAGNET_MAP, WIDGET_TYPE_MAP
from .config.helpers import _opt
from .config.schemas import _validate_widget

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME, CONF_TYPE

display_layout_ns = cg.esphome_ns.namespace("display_layout")
DisplayLayout = display_layout_ns.class_("DisplayLayout", cg.Component)
WidgetConfig = display_layout_ns.struct("WidgetConfig")

ui_ns = cg.global_ns.namespace("ui")
Coord = ui_ns.struct("Coord")

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DisplayLayout),
        cv.Optional(const.CONF_WIDGETS): cv.All(cv.ensure_list(_validate_widget)),
        cv.Optional(const.CONF_GAP_X): cv.int_,
        cv.Optional(const.CONF_RIGHT_EDGE_X): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config: Dict[str, Any]) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    async def _maybe_get(mapping: Dict[str, Any], key: str):
        if key in mapping:
            return await cg.get_variable(mapping[key])
        return None

    # Let C++ know the exact widget count for the registry size.
    widget_list = config.get(const.CONF_WIDGETS, [])
    max_widgets = max(1, len(widget_list))
    cg.add_define("DISPLAY_LAYOUT_MAX_WIDGETS", max_widgets)

    cg.add(var.set_gap_x(config.get(const.CONF_GAP_X, 0)))
    if const.CONF_RIGHT_EDGE_X in config:
        cg.add(var.set_right_edge_x(config[const.CONF_RIGHT_EDGE_X]))

    for widget in widget_list:
        anchor = widget.get(const.CONF_ANCHOR, {})
        anchor_x = anchor.get(const.CONF_X, 0)
        anchor_y = anchor.get(const.CONF_Y, 0)
        anchor_expr = cg.RawExpression(f"ui::Coord({anchor_x}, {anchor_y})")
        font_expr = None
        font2_expr = None
        if const.CONF_FONT in widget:
            font_expr = await cg.get_variable(widget[const.CONF_FONT])
        if const.CONF_FONT2 in widget:
            font2_expr = await cg.get_variable(widget[const.CONF_FONT2])
        sources = widget.get(const.CONF_SOURCES, {})
        image_expr = await _maybe_get(sources, const.CONF_IMAGE)
        count_expr = await _maybe_get(sources, const.CONF_COUNT)
        ready_flag_expr = await _maybe_get(sources, const.CONF_READY_FLAG)
        row_expr = await _maybe_get(sources, const.CONF_ROW)
        channel_expr = await _maybe_get(sources, const.CONF_CHANNEL)
        rx_expr = await _maybe_get(sources, const.CONF_RX)
        tx_expr = await _maybe_get(sources, const.CONF_TX)
        time_expr = await _maybe_get(sources, const.CONF_TIME)
        high_expr = await _maybe_get(sources, const.CONF_HIGH)
        now_expr = await _maybe_get(sources, const.CONF_NOW)
        low_expr = await _maybe_get(sources, const.CONF_LOW)
        weather_expr = (
            await _maybe_get(sources, const.CONF_VALUE)
            if widget[CONF_TYPE] == "weather"
            else None
        )
        updates_expr = (
            await _maybe_get(sources, const.CONF_VALUE)
            if widget[CONF_TYPE] == "ha_updates"
            else None
        )
        phil_expr = await _maybe_get(sources, const.CONF_PHIL)
        nick_expr = await _maybe_get(sources, const.CONF_NICK)
        cfg = cg.StructInitializer(
            WidgetConfig,
            ("kind", cg.RawExpression(WIDGET_TYPE_MAP[widget[CONF_TYPE]])),
            ("id", widget[CONF_NAME]),
            ("anchor", anchor_expr),
            ("priority", widget[const.CONF_PRIORITY]),
            ("magnet", cg.RawExpression(MAGNET_MAP[widget[const.CONF_MAGNET]])),
            ("font", _opt(font_expr)),
            ("font2", _opt(font2_expr)),
            ("pixels_per_character", _opt(widget.get(const.CONF_PIXELS_PER_CHARACTER))),
            ("icon_width", _opt(widget.get(const.CONF_ICON_WIDTH))),
            ("icon_height", _opt(widget.get(const.CONF_ICON_HEIGHT))),
            ("max_icons", _opt(widget.get(const.CONF_MAX_ICONS))),
            ("source_image", _opt(image_expr)),
            ("source_count", _opt(count_expr)),
            ("source_ready_flag", _opt(ready_flag_expr)),
            ("source_chat_row", _opt(row_expr)),
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
