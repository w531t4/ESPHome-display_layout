# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT
from typing import Dict, Any, Optional
import esphome.codegen as cg
import esphome.config_validation as cv
from . import vars as v


def _require_font(value: Dict[str, Any]) -> Dict[str, Any]:
    if v.CONF_FONT not in value:
        raise cv.Invalid("font is required for this widget type")
    return value


def _require_font_pair(value: Dict[str, Any]) -> Dict[str, Any]:
    value = _require_font(value)
    if v.CONF_FONT2 not in value:
        raise cv.Invalid("font2 is required for this widget type")
    return value


def _opt(value: Optional[Any]) -> Any:
    return value if value is not None else cg.RawExpression("std::nullopt")
