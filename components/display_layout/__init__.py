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
        cv.Optional(const.CONF_WIDGETS, default=[]): cv.All(
            cv.ensure_list(_validate_widget), cv.Length(max=const.MAX_WIDGETS)
        ),
        cv.Optional(const.CONF_GAP_X, default=0): cv.int_,
        cv.Optional(const.CONF_RIGHT_EDGE_X): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config: Dict[str, Any]) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_gap_x(config[const.CONF_GAP_X]))
    if const.CONF_RIGHT_EDGE_X in config:
        cg.add(var.set_right_edge_x(config[const.CONF_RIGHT_EDGE_X]))

    for widget in config.get(const.CONF_WIDGETS, []):
        font_expr = None
        font2_expr = None
        if const.CONF_FONT in widget:
            font_expr = await cg.get_variable(widget[const.CONF_FONT])
        if const.CONF_FONT2 in widget:
            font2_expr = await cg.get_variable(widget[const.CONF_FONT2])
        sources = widget.get(const.CONF_SOURCES, {})
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
            if const.CONF_IMAGE in sources:
                image_expr = await cg.get_variable(sources[const.CONF_IMAGE])
            if const.CONF_COUNT in sources:
                count_expr = await cg.get_variable(sources[const.CONF_COUNT])
            if const.CONF_READY_FLAG in sources:
                ready_flag_expr = await cg.get_variable(sources[const.CONF_READY_FLAG])
            if const.CONF_ROW1 in sources:
                row1_expr = await cg.get_variable(sources[const.CONF_ROW1])
            if const.CONF_ROW2 in sources:
                row2_expr = await cg.get_variable(sources[const.CONF_ROW2])
            if const.CONF_ROW3 in sources:
                row3_expr = await cg.get_variable(sources[const.CONF_ROW3])
            if const.CONF_CHANNEL in sources:
                channel_expr = await cg.get_variable(sources[const.CONF_CHANNEL])
            if const.CONF_RX in sources:
                rx_expr = await cg.get_variable(sources[const.CONF_RX])
            if const.CONF_TX in sources:
                tx_expr = await cg.get_variable(sources[const.CONF_TX])
            if const.CONF_VALUE in sources and widget[CONF_TYPE] == "weather":
                weather_expr = await cg.get_variable(sources[const.CONF_VALUE])
            if const.CONF_TIME in sources:
                time_expr = await cg.get_variable(sources[const.CONF_TIME])
            if const.CONF_HIGH in sources:
                high_expr = await cg.get_variable(sources[const.CONF_HIGH])
            if const.CONF_NOW in sources:
                now_expr = await cg.get_variable(sources[const.CONF_NOW])
            if const.CONF_LOW in sources:
                low_expr = await cg.get_variable(sources[const.CONF_LOW])
            if const.CONF_VALUE in sources and widget[CONF_TYPE] == "ha_updates":
                updates_expr = await cg.get_variable(sources[const.CONF_VALUE])
            if const.CONF_PHIL in sources:
                phil_expr = await cg.get_variable(sources[const.CONF_PHIL])
            if const.CONF_NICK in sources:
                nick_expr = await cg.get_variable(sources[const.CONF_NICK])
        cfg = cg.StructInitializer(
            WidgetConfig,
            ("kind", cg.RawExpression(WIDGET_TYPE_MAP[widget[CONF_TYPE]])),
            ("id", widget[CONF_NAME]),
            ("resource", widget.get(const.CONF_RESOURCE, "")),
            (
                "anchor",
                cg.RawExpression(
                    f"ui::Coord({widget[const.CONF_ANCHOR][const.CONF_X]}, "
                    f"{widget[const.CONF_ANCHOR][const.CONF_Y]})"
                ),
            ),
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
