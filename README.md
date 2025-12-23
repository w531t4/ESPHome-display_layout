<!--
SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
SPDX-License-Identifier: MIT
-->
# ESPHome-display_layout
An organizer of content to be displayed on an LED matrix

A 'widget' represents something that is to be presented upon a esphome display component.

This components is an **organizer** of 'widgets'; It solves the following problems:
- I have a widget which only should be displayed when it fails to match value 'x'
- I want my widgets to re-oragnize themselves if one becomes hidden
- I want some widgets to align themselves to the left/right
- I want a single widget to span any leftover space between the left/right-aligned widgets
- I want to **minimize** and **constrain** writes to only locations on the display that need writing to

The library includes a number of different widgets:
- Twitch.tv Chat
- Arbitrary Image (for twitch streamer online icons)
- H:M:S time
- Month/Date
- High/Medium/Low Temperatures
- Weather status
- PSN Friend Status
- Network TX/RX State
- Framerate visualization

# Getting Started
[![Open in Dev Container](https://img.shields.io/badge/Open-Dev%20Container-blue?logo=visualstudiocode)](
https://vscode.dev/redirect?url=vscode://ms-vscode-remote.remote-containers/cloneInVolume?url=https://github.com/w531t4/ESPHome-display_layout
)
[![Open in Dev Container (SSH)](https://img.shields.io/badge/Open-Dev%20Container%20SSH-blue?logo=visualstudiocode)](
https://vscode.dev/redirect?url=vscode://ms-vscode-remote.remote-containers/cloneInVolume?url=ssh%3A%2F%2Fgit%40github.com%2Fw531t4%2FESPHome-display_layout.git
)

1. Add the repostitory as an external_component
    ```
    external_components:
      - source: github://w531t4/ESPHome-display_layout@main
        refresh: 1s
    ```
1. Add the following to your esphome yaml to make the headerfiles accessible
    ```
    display_layout:
      id: my_layout
    ```

# To-do
There's a bunch of glue that currently lives in the esphome yaml that still needs to be brought into this component and exposed as arguments to `display_layout:`

