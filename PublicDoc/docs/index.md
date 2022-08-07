---
title:  "This is the title in index.md"
---

# Index

This is index.md. My home is {{ site.env.BLAH }}. My url is `{{site.url}}`.

## This Should Include Page1.md

{% include Page1.md %}

## This Should Include Page2.md

{% include Page2.md %}

## Contents of the site variables:

site.source: {{ site.source }}

site.destination: {{ site.destination }}

site.remote_theme: {{ site.remote_theme }}

site.CONFIGURATION_DATA: {{ site.[CONFIGURATION_DATA] }}
