Overview
--------

.. incimage:: /images/Logo.jpg 400px right

|appname| helps you create models that you can then send to a 3D printer.  It
is designed to be relatively simple to use, even for someone with little to no
3D modeling experience. If you don't want to deal with the steep learning curve
of 3D modelers like `Blender <https://www.blender.org/>`_, you might find
|appname| useful.

|appname| is inspired somewhat by `OpenSCAD <http://openscad.org/>`_ (my
favorite program for creating 3D models). OpenSCAD is great if you don't mind
typing in numbers for everything and have a reasonable amount of 3D graphics
experience. |appname| is designed to have a similar amount of power while still
being useful to beginners.

|block-image|

.. _ug-workflow:

Typical Workflow
................

Typical use of |appname| to create models for 3D printing involves the
following steps:

 - Create one or more basic models of different types or import existing models
   in `STL format <https://en.wikipedia.org/wiki/STL_(file_format)>`_.
 - Edit the models and/or combine them in different ways to create new models.
 - Export any of the resulting models to STL files to send to a 3D printer.

Because precision is extremely important when 3D printing, |appname| has
several features that allow you to create models with exact dimensions,
orientations, and positions. See the section on :ref:`precision <ug-precision>`
for details.

.. _ug-work-area:

The Work Area
.............

Because |appname| is designed to be used in VR as well as in a conventional
mouse+keyboard environment, it cannot rely on typical menu-based interaction.
Instead, it uses a fully 3D interface (with some :ref:`2D panels
<ug-panel-overview>` embedded in it).

.. incimage:: /images/WorkAreaAnnotated.jpg 1024px center
   :caption:  Elements of the |appname| work area.
   :block:

The work area in |appname| is based conceptually on a virtual workshop
containing these elements:

 - Four walls, a floor, and a ceiling, just to provide context for the room.
 - An exit sign. Clicking on this exits the application. If you have made any
   changes, you will be asked if you want to :ref:`save your session
   <ug-sessions>` first.
 - A large disc in the middle of the floor with a grid on it. This is the
   :newterm:`Stage` on which models are built. It provides :ref:`interaction
   for viewing the models <ug-stage>`.
 - A pole on the right with a :newterm:`Height Slider` that can be used to
   :ref:`change or reset the view height <ug-viewing>`.
 - Several 3D icons organized into shelves. Each icon represents an
   :newterm:`action` that :ref:`makes some change to models or to the session
   state <ug-actions>`.
 - A :newterm:`Precision Control` that shows the current interactive
   :ref:`precision level <ug-precision-level>` with buttons to increase or
   decrease it.
 - The :newterm:`Tree Panel` on the back wall that :ref:`displays all models in
   tree form and allows you to interact with them <ug-tree-panel>`.

.. _ug-panel-overview:

Panels
......

Some interaction requires a more conventional interface; :newterm:`panels` are
used in these circumstances. A panel is a 2D rectangle that appears in the
scene with various text inputs, sliders, and so forth. For example, when the
application starts, the :ref:`Session Panel <ug-session-panel>` appears to let
you choose a session to start or continue. You can interact with panels with
the mouse, keyboard, or VR controllers, just like the rest of the work area.

See :ref:`the section on panels <ug-panels>` for specifics on :ref:`interacting
with panels <ug-panel-interaction>`.

.. _ug-sessions:

Sessions
........

Most applications let you save your work as a data file and restore just the
data when you restart, losing all context of how it was created and edited.
|appname|, on the other hand, stores your :newterm:`session` as a series of
:newterm:`commands` that have been executed along with some other program
state. This means that you can save your session, quit, restart your session,
and be back pretty much where you were. You can undo all of your work back to
the beginning if you want to.

When you start the application, the :ref:`Session Panel <ug-session-panel>`
appears, which offers you the chance to continue the current session (if there
is one), load a different session from a file, or start a brand new session.
The default location of saved sessions is your home directory; this can be
customized in the :ref:`Settings Panel <ug-settings-panel>`.

Certain other program state (such as whether :ref:`targets <ug-targets>` and
:ref:`model edges <ug-toggle-show-edges>` are visible) is saved along with the
session and will be restored when a session is loaded.

The name of the current session file, if any, is displayed at the top of the
:ref:`Tree Panel <ug-tree-panel>` (without the "|suffix|" suffix). The name is
followed by :ref:`a string indicating whether the scene or settings have been
modified <ug-session-name>`.

.. _ug-coordinate-system:
.. _ug-dimension-colors:
.. _ug-units:

Coordinate System, Units, and Dimension Colors
..............................................

.. incimage:: /images/static/CoordSystem.jpg 200px right

|appname| uses the same coordinate system that most 3D printing software uses:
right-handed, with +X to the right, +Y away from the viewer, and +Z up. The top
surface of the :ref:`stage <ug-stage>` is at Z=0.

All dimensions in |appname| are essentially unitless, so you can consider them
to be whatever is most convenient. The only times actual units are considered
are:

 - When models are imported or exported from or to STL files.
 - When the 3D printer :ref:`build volume <ug-build-volume>` is defined to
   show the extents of your 3D printer.

The :ref:`Settings Panel <ug-settings-panel>` allows you to specify :ref:`how
to convert units <ug-unit-conversion>` when importing and exporting, and also
how large the build volume is in whatever units you would like to work in. For
example, if you consider |appname| units to be inches and need to convert to
millimeters for export (which is the standard for STL files), you can set that.

The default settings assume that |appname| units are centimeters and convert to
and from millimeters (STL standard) on export and import.

Note that the grid on the stage has thin lines every 1 unit, with thicker lines
every 5 units. The grid grows or shrinks :ref:`when the stage is scaled
<ug-stage>` so that you can always discern the absolute sizes of models on
it. Also, the X and Y axes are colored red and green on the grid for reference.

Most everything in the application that aligns with the coordinate axes uses a
standard RGB color scheme: :raw-html:`<font color="red">X is red</font>`,
:raw-html:`<font color="green">Y is green</font>`, and :raw-html:`<font
color="blue">Z is blue</font>` as in the diagram. This is true for axis-aligned
parts of :ref:`Tools <ug-tools>` and :ref:`interactive visual feedback
<ug-feedback>`.

.. _ug-build-volume:

The Build Volume
................

.. incimage:: /images/BuildVolume.jpg 200px right

If you want to make sure that your models will fit within your 3D printer's
build volume, you can choose to display the build volume on the stage by
clicking on the :ref:`Build Volume Toggle <ug-toggle-build-volume>` icon on the
:ref:`Viewing Shelf <ug-work-area>`. The build volume is rendered as a
translucent box. You can specify the size of the build volume for your specific
printer in the ref:`Settings Panel <ug-settings-panel>`.

When the build volume is visible, parts of models that lie outside it are
rendered in :ref:`a special color <ug-model-colors>` to warn you of potential
printing problems.

|block-image|

.. _ug-vr-modes:

VR and Non-VR Modes
...................

Since most people do not have a VR setup, |appname| can operate without one,
using the mouse and keyboard exclusively. This is referred to below as
:newterm:`conventional mode`.

If you do happen to have a VR headset, you can set it up as explained in the
:ref:`Quickstart <qs-vr>`. There are two ways to use it with the application:

 - :newterm:`VR mode` refers to using the VR system with the headset on and
   controllers in your hand.
 - :newterm:`Hybrid mode` is halfway between the other two: you have the VR
   system connected, but you are not wearing the headset. In this mode you can
   still use one or both controllers for interaction in the conventional window
   view.

Some VR experiences allow you or force you to walk around while using them.
That wouldn't make sense for this application, so it is designed to make
everything useful from one spot. If you're planning to use the VR mode, set it
up for a comfortable, fixed standing or seated position. Note that the scene
view is set up for sitting, so you may need to adjust your VR positioning for a
different height if you prefer to stand.

.. admonition:: VR Only

   Note: any information in the rest of this guide that applies only to either
   of the two VR modes will be formatted like this paragraph. If you are not
   using a VR system, feel free to skip over these.
