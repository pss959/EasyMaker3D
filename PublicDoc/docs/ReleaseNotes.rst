|appname| Release Notes
=======================

Release 1.8.0 (November, 2023)
------------------------------

 - New Features:

   - The Info Panel now shows an approximate volume for the selected models.
     This can be useful for estimating 3D print times and material use.

   - Scrolling with a mouse scroll wheel is now localized by mouse cursor
     position, meaning that it can be used in scrolling areas and dropdown
     lists in panels. Scrolling to change the stage size still works when the
     cursor is over the stage or any model or tool on it.

   - Added OFF file format export.

 - Bug Fixes:

   - Improved focus management in panels

   - VR controller positioning feedback is now better for different VR
     systems. (There is an inconsistency in the way OpenVR treats them.)

   - Selecting models in a hierarchy with more than two levels now works
     correctly.

   - Drag-selecting in text input fields in panels now works correctly.

   - Resizing a Bevel Tool Panel now works properly: the profile area is
     resized.

   - Many other potential bugs were fixed after extensive unit testing.

Release 1.7.0 (June, 2023)
---------------------------

 - New Features:

   - Added *Duplicate* action, icon, and shortcut.

   - Added Bend Tool.

   - Changed Twist Tool interaction to be consistent with Bend Tool and added a
     way to offset twisted models.

   - Changed Clip Tool and Mirror Tool to be more consistent with Twist Tool
     and Bend Tool; use the arrow for rotation and the plane for translation.
     Improved plane translation feedback as well.

   - Pasted models now have random colors assigned to help distinguish them
     from originals.

   - Resized and rearranged some items in the work area to fit shelves better.

 - Bug Fixes:

   - Fixed an issue that caused the program to freeze.

   - Fixed problems with Point Target placement on models.

   - Fixed some interactive rotation problems.

   - Fixed a problem with dragging cone-shaped interactive parts.


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
