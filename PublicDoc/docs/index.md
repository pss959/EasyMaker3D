---
title:     "Welcome to MakerVR"
nav_order: 1
---

# Welcome to MakerVR

![MakerVR Logo](/images/logo.jpg){:width="50%"}

MakerVR is a VR-enabled application for creating 3D-printable models.
"VR-enabled" means that it can be used in VR, but also works in a standard
mouse+keyboard configuration. In fact, it can also be used in VR without the
headset on. You can use any combination of VR controllers, mouse, and keyboard
in that mode.

Finished models may be saved as STL files that can then be sent to a 3D printer.

MakerVR is available for Windows and Linux. There is also a Mac version, but it
may or not work in VR due to the
[lack of official SteamVR support](
https://store.steampowered.com/news/app/250820/view/2216278054495230717);
I was unable to get SteamVR to display on the HTC Vive from a Macbook, but that
might be due to specific hardware limitations.

## [Quickstart]({% link Quickstart.md %})

## This Should Include Page1.md

{% include Page1.md %}

## This Should Include Page2.md

{% include Page2.md %}

## Contents of the site variables:

<br>site.url: {{ site.url }}
<br>site.source: {{ site.source }}
<br>site.destination: {{ site.destination }}
<br>site.remote_theme: {{ site.remote_theme }}
