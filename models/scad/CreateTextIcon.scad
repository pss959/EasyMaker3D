//=============================================================================
// Icon used for the Text model: a 3D letter 'T'.
//=============================================================================

module TextIcon() {
     linear_extrude(height = 6) {
          text(text = "T", font="Liberation Serif:style=Bold",
               size = 20, halign = "center", valign = "center");
     }
}

TextIcon();
