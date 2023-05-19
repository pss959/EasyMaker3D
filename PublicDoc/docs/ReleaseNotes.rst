|appname| Release Notes
=======================

Release 1.6.0 (May, 2023)
---------------------------

 - New Features:

   - Added Tapered model and Taper Tool.

   - Added titles to all panel-based tools.

   - Added a new format for calling out important notes in the User Guide.

 - Bug Fixes:

   - Greatly sped up Twist Tool (and Taper Tool) with custom mesh slicing code.

   - Twist Tool angle feedback positioning is now correct for rotated models.

   - Improved spherical rotation when a rotation was already applied.

   - Tab/Shift-Tab to move focus in panels now correctly skips over all
     disabled parts.

   - Fixed a problem where loading a file could cause the Session Panel to
     remain displayed incorrectly.


Release 1.5.0 (May, 2023)
---------------------------

 - New Features:

   - Added Twisted model and Twist Tool.

   - Changed the way converted models work. You can now convert a model again
     to apply another operation of the same type (such as a clipping
     plane). Each converted model has a single type of operation that it edited
     by its specialized tool.

   - Changed the UI for Clip Tool and Mirror Tool to operate on a single plane
     more flexibly, resulting from above change to converted models.

 - Bug Fixes:

   - The Session Panel no longer remains visible incorrectly after loading a
     new session.

   - The Point Target now snaps more correctly to model bounds when
     modified-dragging.

   - Commands using converted models no longer can use incorrect model names.

   - Fixed a couple of places where scaling by 0 could occur, including for
     hovered icons.

Release 1.4.0 (April, 2023)
---------------------------

 - New Features:

   - Added extruded models with a specialized editor.

   - Split the model creation shelf into two parts and tweaked some sizes and
     positions to improve icon placement.

 - Bug Fixes:

   - Fixed some crashes due to Session Panel mismanagement.

Release 1.3.0 (March, 2023)
---------------------------

 - Bug Fixes:

   - The escape key now works as expected in the Session Panel.

   - Text input in panels handles shifted keys correctly again.

   - Interacting in a tool panel no longer causes the panel to be repositioned.

   - Keyboard shortcuts are now ignored while an application panel is in use.

   - Tools are not attached to new models until animated placement is complete.

Release 1.2.0 (March, 2023)
---------------------------

 - New Features:

   - Tooltips are now triggered even for disabled 3D icons.

   - Floating panels are always placed in the line of sight for the current
     view (non-VR only).

   - Inspector mode places the inspected model in the line of sight for the
     current view (non-VR/mouse only).

   - Added processing of custom keyboard shortcut file.

   - Added Delete key as a shortcut for delete action.

   - Crash files now contain more useful information.

 - Bug Fixes:

   - Use proper URLs in Help Panel.

   - Clicking on a point when editing a profile no longer resets the profile.

Release 1.1.0 (March, 2023)
---------------------------

 - New Features:

   - Added snapping to surrounding points to Profile Area for Bevel Tool and
     RevSurf Tool.

   - Added snapping to current precision (with checkbox) to RevSurf Tool.

 - Bug Fixes:

   - Use proper URLs in Help Panel.

   - Stop Color Tool setting color to black in some cases.

   - Fixed case where 3D icons would stop responding to hovering.

   - Prevent Bevel Tool from adding no-op commands to session.

Release 1.0.1 (February, 2023)
------------------------------

 - Added versioned public documentation and release notes.

 - Internal code and documentation cleanup.

Release 1.0.0 (February, 2023)
------------------------------

 - First public release!
