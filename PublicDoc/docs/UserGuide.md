<!--
Note that the cinder theme has a maximum of 3 levels for its table of
contents. Instead of using the first level (h1) for the title, create a title
div.
-->

<div class="title">MakerVR User Guide</div>

{% include-markdown "./UserGuide/Overview.md"    rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Interaction.md" rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Models.md"      rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Selection.md"   rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Precision.md"   rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Actions.md"     rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Tools.md"       rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/GuiPanels.md"   rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/TreePanel.md"   rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Quirks.md"      rewrite-relative-urls=false %}

<!-- These define references used in the rest of the User Guide: -->
{% include-markdown "./UserGuide/Links.md"       rewrite-relative-urls=false %}
{% include-markdown "./UserGuide/Images.md"      rewrite-relative-urls=false %}
