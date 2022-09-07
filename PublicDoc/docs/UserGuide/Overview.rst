Overview
--------

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

Jump Right In?
..............

|appname| is designed to be reasonably intuitive to use, but there are a few
things that may need explanation before you can dive into using it. Here are
some sections you may want to refer to:

 * :ref:`workflow`: describes basic use of the application to create models for
   3D printing.
 * :ref:`the-work-area`: describes the various interactive parts of the work
   area and what they do.

 * :ref:`vr-modes`: describes how to use the application with VR equipment if
   you have some.

.. _workflow:

Typical Workflow
................

Typical use of |appname| to create models for 3D printing involves the following
steps:

 * Create one or more basic models of different types or import existing models
   in `STL format <https://en.wikipedia.org/wiki/STL_(file_format)>`_.
 * Edit the models and/or combine them in different ways to create new models.
 * Export any of the resulting models to STL files to send to a 3D printer.

Because precision is extremely important when 3D printing, |appname| has several
features that allow you to create models with exact dimensions, orientations,
and positions. See the section on [precision][Precision] for details.

.. _the-work-area:

The Work Area
.............

.. incimage:: /images/WorkArea.jpg 600px center
   :caption:  The (empty) |appname| work area.
   :block:

The work area in |appname| is based conceptually on a virtual workshop. You may
notice the following elements:

* Four walls, a floor, and a ceiling. They just provide scene context.
* An exit sign. Clicking on this exits the application. If you haven't saved
  your session, you will be asked to do so.
* A large disc in the middle of the floor with a grid on it. This is the
  [stage][Stage] on which models are built.
* A pole on the right with a ring around it. The ring is a {{term('height
  slider')}} that can be used to [change the view height][Viewing].
* Several shelves with 3D icons on them. Each icon represents an
  [action][Actions] that makes a change to the scene or program state.
* A caliper to the right of the exit sign with text that shows the current
  [precision level][Precision], along with buttons to increase or decrease it.
* A large framed panel on the right of the back wall. This is the [Tree
  Panel][TreePanel] that shows your models in tree form and provides various
  ways to interact with them.

Sessions
........

Unlike applications that save your work as a data file, |appname| stores your
{{term('session')}} as a series of commands that have been executed. This means
that you can save your session, quit, restart your session, and be back pretty
much where you were. You can undo all of your work back to the beginning if you
feel like it.

When you start the application, the first thing you see is the [Session
Panel][SessionPanel]. It offers you the chance to continue the current session
(if there is one), load a different session, or start a new session.  The
location of saved sessions is customizable in the [Settings
Panel][SettingsPanel].

The state of various settings (such as whether [targets][Targets] and
[edges][ShowEdges] are visible) is saved along with the session and is restored
when a session is loaded.

The name of the current session file is displayed at the top of the [Tree
Panel][TreePanel]. The name is followed by a string indicating [whether the
scene or settings have been modified][SessionName].

Coordinate System and Units
...........................

![The |appname| coordinate system][ImgCoordSys]{{rightimage(200)}}

|appname| uses the same coordinate system that most 3D printing software uses:
right-handed, with +X to the right, +Y away from the viewer, and +Z up. The
surface of the [stage][Stage] is at Z=0.

{{tiny('Don\'t let the fact that this application is built on Unity with its
ridiculous left-handed coordinate system bother you.')}}

|appname| is essentially unit-independent. The only places where units are
considered are:

* When models are imported or exported from or to STL files.
* When the 3D printer [build volume][BuildVolume] is shown.

The [Settings Panel][SettingsPanel] allows you to specify [how to convert
units][UnitConversion] when importing and exporting, and also how large the
[build volume][BuildVolume] is in whatever units you would like to work in.

The application assumes by default that everything is expressed in centimeter
units. If your 3D printing software expects STL files in millimeter units, you
can convert from centimeters to millimeters on export. If you are importing an
STL file that is in millimeters, you can convert it from millimeters to
centimeters on import.

If you want for some reason to work in inches, you can just consider the
working units to be inches, and use the same conversion facilities to convert
to and from saner units.

Note that the grid on the stage has thin lines every 1 unit, with thicker lines
every 5 units. The grid grows or shrinks [when the stage is scaled][Stage]
so that you can always measure absolute sizes of models. The X and Y axes are
colored red and green on the grid for reference.

The Build Volume
................

![][ImgBuildVolume]{{rightimage(140)}}

If you want to make sure that your models will fit within your 3D printer's
build volume, you can choose to display the build volume on the stage by
clicking on the [Build Volume Toggle][BuildVolumeToggle] icon. The build volume
is rendered as a translucent box. You can specify the size of your build volume
in the [Settings Panel][BuildVolumeSize].

When the build volume is visible, parts of models that lie outside it are
rendered in [a special color][ModelColors] to warn you of potential printing
errors.

Colors
......

Most everything in the application that aligns with the coordinate axes uses
the standard RGB color scheme: X is {{color('red', 'red')}}, Y is
{{color('green', 'green')}}, Z is {{color('blue', 'blue')}}, as shown
above. This is true for [tool][Tools] parts, [hover guides][ClickDrag], and
[interactive visual feedback][Feedback].

