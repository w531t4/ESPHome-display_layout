<!--
SPDX-FileCopyrightText: 2026 Aaron White <w531t4@gmail.com>
SPDX-License-Identifier: MIT
-->

# Current State

Using 80Mhz SPI with 64x32 (x12 in series) and RGB24

- Healthy output using 10ms refresh rate
- roughly 75fps
- takes roughly 13ms to write a full frame over SPI @ above speeds
- rendering content currently takes ~0.75ms

# Metrics captured while switching display_layout to be event-driven

## Before using callbacks

```
render avg us: total=1223.6 post=551.0 motion=15.6 update=259.9 relayout=395.4 over 141 frames
post avg us (sorted 1/2): network_tput=117.7 weather=73.0 psn=65.4 time=60.3 temperatures=51.5
post avg us (sorted 2/2): twitch_chat=46.3 date=42.9 ha_updates=18.7 twitch_icons=1.7
post detail avg us: weather=49.7 (clock=17.1 state=0.4 post=47.4) chat=0.0 (hist=0.0 post=0.0)
post detail avg us: net=106.8 (state=3.9 post=98.8) psn=59.0 (pre=0.8 post=58.1)
```

## After net tput (watching twitch channel)

```
render avg us: total=1423.6 post=646.0 motion=20.8 update=305.5 relayout=449.6 over 161 frames
post avg us (sorted 1/2): psn=170.0 network_tput=122.8 twitch_chat=99.4 weather=98.9 date=66.3
post avg us (sorted 2/2): temperatures=54.1 time=53.3 ha_updates=46.2 twitch_icons=1.7
post total us (sorted 1/2): psn=27364(161) twitch_chat=15997(161) weather=15921(161) date=10676
post total us (sorted 2/2): time=8585(161) ha_updates=7439(161) network_tput=491(4) twitch_icons=279
post detail avg us: weather=80.7 (clock=19.5 state=0.7 post=74.4) chat=0.0 (hist=0.0 post=0.0)
post detail avg us: net=122.8 (state=0.5 post=122.2) calls=4 rate=1.99/s psn=155.7 (pre=9.1 post=146.6) calls=161 rate=80.12/s
```

## after PSN? (watching twitch channel)

```
render avg us: total=1256.3 post=592.9 motion=18.4 update=266.9 relayout=376.4 over 148 frames
post avg us (sorted 1/2): twitch_chat=202.5 network_tput=135.5 weather=93.4 time=84.1 date=73.2
post avg us (sorted 2/2): temperatures=72.7 ha_updates=20.1 twitch_icons=9.9 pixel_motion=0.8 psn=0.0
post total us (sorted 1/2): twitch_chat=29977(148) weather=13827(148) time=12446(148) date=10839(148) temperatures=10756(148)
post total us (sorted 2/2): ha_updates=2981(148) twitch_icons=1464(148) network_tput=542(4) pixel_motion=114(148) psn=0(0)
post detail avg us: weather=65.0 (clock=17.6 state=3.9 post=56.1) chat=188.3 (hist=7.9 post=117.4) date=71.6
post detail avg us: net=135.5 (state=0.8 post=134.8) calls=4 rate=1.99/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after twitch chat (watching twitch channel)

```
render avg us: total=1429.2 post=429.6 motion=18.8 update=586.1 relayout=393.8 over 151 frames
post avg us (sorted 1/2): twitch_chat=203.0 network_tput=130.8 date=97.4 weather=84.1 time=67.6
post avg us (sorted 2/2): temperatures=59.3 twitch_icons=54.5 ha_updates=15.5 pixel_motion=0.9 psn=0.0
post total us (sorted 1/2): date=14703(151) weather=12704(151) time=10205(151) temperatures=8958(151) twitch_icons=8233(151)
post total us (sorted 2/2): ha_updates=2341(151) network_tput=523(4) twitch_chat=203(1) pixel_motion=129(151) psn=0(0)
post detail avg us: weather=65.4 (clock=24.0 state=0.4 post=62.9) chat=203.0 (hist=15.0 post=133.0) date=89.5
post detail avg us: net=130.8 (state=4.0 post=126.8) calls=4 rate=1.99/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## After weather (watching twitch channel)

```
render avg us: total=1373.5 post=283.6 motion=6.9 update=631.4 relayout=450.7 over 150 frames
post avg us (sorted 1/2): twitch_chat=252.0 network_tput=166.5 temperatures=95.3 date=61.4 time=54.6
post avg us (sorted 2/2): ha_updates=14.0 twitch_icons=6.0 pixel_motion=0.6 weather=0.0 psn=0.0
post total us (sorted 1/2): temperatures=14290(150) date=9207(150) time=8183(150) ha_updates=2107(150) twitch_icons=898(150)
post total us (sorted 2/2): twitch_chat=756(3) network_tput=666(4) pixel_motion=95(150) weather=0(0) psn=0(0)
post detail avg us: weather=0.0 (clock=0.0 state=0.0 post=0.0) chat=252.0 (hist=33.7 post=160.7) date=59.4
post detail avg us: net=166.5 (state=0.5 post=166.0) calls=4 rate=1.99/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after temp (not watching twitch channel)

```
render avg us: total=903.3 post=190.7 motion=5.8 update=272.2 relayout=434.1 over 150 frames
post avg us (sorted 1/2): network_tput=140.2 date=78.1 time=55.9 twitch_icons=6.4 ha_updates=2.5
post avg us (sorted 2/2): pixel_motion=0.4 twitch_chat=0.0 weather=0.0 temperatures=0.0 psn=0.0
post total us (sorted 1/2): date=11712(150) time=8389(150) twitch_icons=961(150) network_tput=561(4) ha_updates=369(150)
post total us (sorted 2/2): pixel_motion=64(150) twitch_chat=0(0) weather=0(0) temperatures=0(0) psn=0(0)
post detail avg us: weather=0.0 (clock=0.0 state=0.0 post=0.0) chat=0.0 (hist=0.0 post=0.0) date=75.4
post detail avg us: net=140.2 (state=6.2 post=134.0) calls=4 rate=2.00/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after time (not watching twitch channel)

```
render avg us: total=738.3 post=151.1 motion=11.0 update=250.4 relayout=324.8 over 153 frames
post avg us (sorted 1/2): time=600.5 network_tput=147.5 date=63.3 ha_updates=17.7 twitch_icons=16.8
post avg us (sorted 2/2): pixel_motion=0.9 twitch_chat=0.0 weather=0.0 temperatures=0.0 psn=0.0
post total us (sorted 1/2): date=9688(153) ha_updates=2704(153) twitch_icons=2564(153) time=1201(2) network_tput=590(4)
post total us (sorted 2/2): pixel_motion=143(153) twitch_chat=0(0) weather=0(0) temperatures=0(0) psn=0(0)
post detail avg us: weather=0.0 (clock=0.0 state=0.0 post=0.0) chat=0.0 (hist=0.0 post=0.0) date=61.5
post detail avg us: net=147.5 (state=5.5 post=142.0) calls=4 rate=2.00/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after date (not watching twitch channel)

```
render avg us: total=754.5 post=71.5 motion=7.2 update=327.5 relayout=347.7 over 153 frames
post avg us (sorted 1/2): time=824.0 network_tput=142.8 ha_updates=34.2 twitch_icons=2.6 pixel_motion=0.6
post avg us (sorted 2/2): twitch_chat=0.0 weather=0.0 temperatures=0.0 date=0.0 psn=0.0
post total us (sorted 1/2): ha_updates=5230(153) time=1648(2) network_tput=571(4) twitch_icons=395(153) pixel_motion=99(153)
post total us (sorted 2/2): twitch_chat=0(0) weather=0(0) temperatures=0(0) date=0(0) psn=0(0)
post detail avg us: net=142.8 (state=1.0 post=141.8) calls=4 rate=1.99/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after ha_updates (not watching twitch channel)

```
render avg us: total=763.8 post=52.7 motion=13.2 update=352.6 relayout=344.3 over 150 frames
post avg us (sorted 1/2): time=659.5 network_tput=163.0 twitch_icons=5.2 pixel_motion=0.6 twitch_chat=0.0
post avg us (sorted 2/2): weather=0.0 temperatures=0.0 date=0.0 ha_updates=0.0 psn=0.0
post total us (sorted 1/2): time=1319(2) twitch_icons=776(150) network_tput=652(4) pixel_motion=86(150) twitch_chat=0(0)
post total us (sorted 2/2): weather=0(0) temperatures=0(0) date=0(0) ha_updates=0(0) psn=0(0)
post detail avg us: net=163.0 (state=2.5 post=160.5) calls=4 rate=2.00/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```

## after twitch_icons (not watching twitch channel)

```
render avg us: total=749.6 post=36.4 motion=22.3 update=331.9 relayout=358.0 over 154 frames
post avg us (sorted 1/2): time=878.0 network_tput=152.0 pixel_motion=1.4 twitch_icons=0.0 twitch_chat=0.0
post avg us (sorted 2/2): weather=0.0 temperatures=0.0 date=0.0 ha_updates=0.0 psn=0.0
post total us (sorted 1/2): time=1756(2) network_tput=608(4) pixel_motion=209(154) twitch_icons=0(0) twitch_chat=0(0)
post total us (sorted 2/2): weather=0(0) temperatures=0(0) date=0(0) ha_updates=0(0) psn=0(0)
post detail avg us: net=152.0 (state=1.0 post=151.0) calls=4 rate=2.00/s psn=0.0 (pre=0.0 post=0.0) calls=0 rate=0.00/s
```
