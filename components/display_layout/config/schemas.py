# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT
from typing import Dict, Any
from . import const
from .helpers import _require_font, _require_font_pair
from .maps import WIDGET_TYPE_MAP, MAGNET_MAP

from esphome.const import CONF_NAME, CONF_TYPE
import esphome.config_validation as cv
from esphome.components import font
from esphome.components import globals as globals_component
from esphome.components import online_image
from esphome.components import sensor, text_sensor, time
from esphome.components.homeassistant import text_sensor as ha_text_sensor


# The globals component doesn't expose typed IDs, so accept any globals component
# here and rely on the configured C++ type to be bool.
GLOBALS_BOOL = globals_component.GlobalsComponent


BASE_WIDGET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TYPE): cv.one_of(*WIDGET_TYPE_MAP, lower=True),
        cv.Required(CONF_NAME): cv.string,
        cv.Required(const.CONF_ANCHOR): cv.Schema(
            {cv.Required(const.CONF_X): cv.int_, cv.Required(const.CONF_Y): cv.int_}
        ),
        cv.Optional(const.CONF_PRIORITY, default=0): cv.int_range(min=0, max=255),
        cv.Optional(const.CONF_MAGNET, default="right"): cv.one_of(
            *MAGNET_MAP, lower=True
        ),
        cv.Optional(const.CONF_RESOURCE, default=""): cv.string_strict,
        cv.Optional(const.CONF_FONT): cv.use_id(font.Font),
        cv.Optional(const.CONF_FONT2): cv.use_id(font.Font),
        cv.Optional(const.CONF_PIXELS_PER_CHARACTER): cv.int_range(min=1),
        cv.Optional(const.CONF_ICON_WIDTH): cv.positive_int,
        cv.Optional(const.CONF_ICON_HEIGHT): cv.positive_int,
        cv.Optional(const.CONF_MAX_ICONS): cv.positive_int,
    }
)

WIDGET_SCHEMAS = {
    "twitch_icons": BASE_WIDGET_SCHEMA.extend(
        {
            cv.Required(const.CONF_ICON_WIDTH): cv.positive_int,
            cv.Required(const.CONF_ICON_HEIGHT): cv.positive_int,
            cv.Required(const.CONF_MAX_ICONS): cv.positive_int,
            cv.Optional(const.CONF_SOURCES): cv.Schema(
                {
                    cv.Required(const.CONF_IMAGE): cv.use_id(online_image.OnlineImage),
                    cv.Required(const.CONF_COUNT): cv.use_id(text_sensor.TextSensor),
                    cv.Optional(const.CONF_READY_FLAG): cv.use_id(GLOBALS_BOOL),
                }
            ),
        }
    ),
    "twitch_chat": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(const.CONF_ROW): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Optional(const.CONF_CHANNEL): cv.use_id(
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
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(const.CONF_RX): cv.use_id(sensor.Sensor),
                        cv.Required(const.CONF_TX): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "weather": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(const.CONF_VALUE): cv.use_id(
                            text_sensor.TextSensor
                        ),
                        cv.Required(const.CONF_TIME): cv.use_id(time.RealTimeClock),
                    }
                )
            }
        ),
        _require_font,
    ),
    "temperatures": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(const.CONF_HIGH): cv.use_id(sensor.Sensor),
                        cv.Required(const.CONF_NOW): cv.use_id(sensor.Sensor),
                        cv.Required(const.CONF_LOW): cv.use_id(sensor.Sensor),
                    }
                )
            }
        ),
        _require_font,
    ),
    "date": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {cv.Required(const.CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "time": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {cv.Required(const.CONF_TIME): cv.use_id(time.RealTimeClock)}
                )
            }
        ),
        _require_font_pair,
    ),
    "ha_updates": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {cv.Required(const.CONF_VALUE): cv.use_id(sensor.Sensor)}
                )
            }
        ),
        _require_font,
    ),
    "psn": cv.All(
        BASE_WIDGET_SCHEMA.extend(
            {
                cv.Optional(const.CONF_SOURCES): cv.Schema(
                    {
                        cv.Required(const.CONF_PHIL): cv.use_id(
                            ha_text_sensor.HomeassistantTextSensor
                        ),
                        cv.Required(const.CONF_NICK): cv.use_id(
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
