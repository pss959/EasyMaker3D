---
title:     Welcome to MakerVR
layout:    default
nav_order: 1
---

# Welcome to {%appname%}

![{%appname%} Logo](/images/logo.jpg){:width="50%"}{:style="float: right"}

{%appname%} is a VR-enabled application for creating 3D-printable models.
"VR-enabled" means that it can be used in VR, but also works in a standard
mouse+keyboard configuration. It can also be used in VR without the headset
on. You can use any combination of VR controllers, mouse, and keyboard in that
mode.

Finished models may be saved as STL files that can then be sent to a 3D printer.

{%appname%} is available for Windows and Linux. There is also a Mac version,
but it may or not work in VR due to the [lack of official SteamVR support](
https://store.steampowered.com/news/app/250820/view/2216278054495230717); I was
unable to get SteamVR to display on the HTC Vive from a Macbook, but that might
be due to specific hardware limitations.

## [Quickstart]({% link Quickstart.md %})

## [User Guide]({% link UserGuide.md %})

## Site Variables

```
site.url: {{ site.url }}
site.source: {{ site.source }}
site.destination: {{ site.destination }}
site.remote_theme: {{ site.remote_theme }}
env test: {{ site.env }}
```
