<!--
Note that the cinder theme has a maximum of 3 levels for its table of
contents. Instead of using the first level (h1) for the title, create a title
div.
-->

<div class="title">MakerVR Release Notes</div>

# Release 0.2.1 (July 18, 2021)

+ Implemented double-click dragging for Alt-dragging.

+ Fixed some tool-related crashes.

# Release 0.2.0 (July 06, 2021)

+ Redesigned the tool handling user experience.  Separated general tools from
  specialized tools and made it easy to switch between them. Changed the user
  workflow to first create or convert models of a specific type, then use
  specialized tool to edit them. Also allow editing with a specialized tool
  when all models of the correct type are selected, for all model
  types. Removed several special cases where specialized tools had to convert
  models first as an extra step.

+ Better handling of grip interaction with panels.

+ Added RevSurf (surface of revolution) creation and editing.

+ New User Guide organization and format.

+ New documentation formatting and style.

# Release 0.1.0 (May 23, 2021)

+ Added a Mac version again. Unfortunately, it has to be missing
  features. (Thanks again to Bob Brown for the loaner MacBook.)

+ The mouse scroll wheel now scales the stage.

+ Added a file format dropdown for model export (text vs. binary STL).

+ Added Settings Panel dropdowns to convert to and from alternate units for
  export and import.

+ Added the ability to show the build volume for a 3D printer. There is an
  entry in the Settings Panel for the size and a new icon to toggle display of
  the volume on and off. When the volume is visible, parts of models outside
  the volume are rendered using a special color. The stage grid also resizes
  when the build volume size is changed.

+ Radial menus are now available to use when in non-VR and hybrid modes.

+ The Scale Tool now allows scaling in 1, 2, or 3 dimensions.

+ The scene name displayed in the TreePanel now indicates whether the scene has
  been modified.

+ Models display tooltips containing their names.

+ The Precision display is simplified to just show the current precision as
  text, and now has buttons to increase or decrease precision.

+ Reimplemented device and event handling to provide more consistent behavior.

+ Completely revised and improved much of the application appearance:
  materials, colors, sizes, feedback.

+ Revised the GUI implementation. All GUI panels use the same infrastructure,
  allowing optional moving and resizing. Built new custom GUI elements that
  respond to resizing better, handle keyboard navigation better, and provide an
  easier and more consistent VR interface.

+ Implemented better synchronization between the keyboard and the VRKeys
  virtual keyboard for text input.

+ Added a grip bar to help show that the virtual keyboard can be dragged.

+ Fixed some issues with triangulating complex polygons, such as text.

# Release 0.0.5 (November 09, 2020)

+ Removed the Mac version from the Release 0.0.4 distribution. Unity enables
  Metal by default, but Unity/Metal does not support geometry shaders, so the
  MakerVR custom faceted shader does not work. Disabling Metal caused all kinds
  of horrible crashes. We will have to wait for something to be fixed.

+ Added 3D text via the Text Tool.

+ Now using CGAL for face triangulation. This fixed a lot of problems with
  complex concave polygons.

+ Invalid meshes are now indicated with a bright red color. This allows users
  to tell when the Profile Tool is causing problems. When an invalid mesh is
  selected, several different tools are now disabled.

+ Added a setting to the Settings Panel for tooltip delay, in seconds. Setting
  this to 0 disables tooltips completely.

+ Exporting as STL now operates on the current selection rather than all
  existing top-level models.

+ Many other bug fixes, especially those involving geometric inconsistencies.

# Release 0.0.4 (September 26, 2020)

+ Added a Mac version. (Thanks to Bob Brown for the loaner MacBook.)

+ Added the Info Panel, along with new tool icons to open the Info Panel and
  the Help Panel.

+ Added the Clip Tool.

+ Added a slider to the Profile Tool to allow the maximum edge angle to be
  specified; edges forming smaller angles will not be profiled. Fixed a lot of
  Profile Tool math.

+ Fixed a bug where TreePanel interaction could occur even when it was occluded.

+ All duplicate vertices in meshes should now be merged properly.

+ Changed the way parent models are handled relative to their children; the
  parent is always defined initially in axis-aligned Cartesian coordinates
  rather than sometimes trying to copy the child's rotation.

# Release 0.0.3 (August 9, 2020)

+ Added HelpPanel with keyboard shortcuts and button on SessionPanel.

+ Session state (settings of all toggles) is now saved with MVR files. Older
  MVR files will still be able to be loaded, but will reset the state to
  default settings. InspectSelection is no longer considered a toggle, since it
  is really transient and cannot persist across sessions.

+ Fixed a bug where STL output of models did not transform them correctly.

+ Improved size and placement of tooltips.

+ Implemented alt-dragging for point and edge targets to drag over bounds of
  intersected model, snapping to edges and centers. Also added a snapping
  feedback indicator to the point target.

+ Sessions can be saved after changing any of the toggles.

+ Added a button for creating a convex hull of one or more models.

+ Added a Profile Tool to change the profiles of all edges of a model to create
  bevels, chamfers, etc.

+ The Color Tool now responds to clicks in the color ring in addition to drags.

+ Fixed some bugs in STL import.

+ Fixed some transformation bugs in the Inspector.

+ Added versioning and version switching to documentation; retroactively added
  to previous versions.

# Release 0.0.2 (June 21, 2020)

+ Sped up CSG operations on Linux.

+ Slow CSG operations now run in a separate thread and show an hourglass cursor
  and model while they are running.

+ Fixed STL importing to uniquify vertices so that models have a better chance
  of being watertight.

+ Improved checking of imported models for validity and solidity.

+ Added option of exporting STL files as text or binary.

# Release 0.0.1 (June 17, 2020)

Initial release to a very small number of people for testing.
