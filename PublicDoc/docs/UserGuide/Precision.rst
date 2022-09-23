Precision
---------

|appname| offers several features to help you create 3D models with precise
dimensions and placement.

.. _ug-precision-level:

Precision Level
...............

|appname| provides a :newterm:`variable precision level` to help you use exact
values during drag operations. There are three precision levels, each affecting
both linear operations (such as scaling and translation) and angular operations
(such as rotation).

  -   1 unit / 15\ |degree|
  -  .1 unit /  5\ |degree|
  - .01 unit /  1\ |degree|

Assuming :ref:`centimeter units <ug-units>`, the linear precision levels are
then 1cm, 1mm, and .1mm, respectively.

.. incimage:: /images/PrecisionControl.jpg 200px right

Here is how to change the current precision level:

  - Click on one of the the two buttons on the Precision Control on the back
    wall of the :ref:`work area <ug-work-area>`. The top button increases to
    finer precision, and the bottom button decreases to coarser precision.
  - Use :shortcut:`x` and :shortcut:`z` :ref:`keyboard shortcuts
    <ug-keyboard-shortcuts>` to increase and decrease levels, respectively.

  .. admonition:: VR Only

     You can also click the Up or Down buttons on either VR controller trackpad
     or joystick to increase or decrease the current precision.

Note that the keyboard and controller shortcuts work even during drag and VR
grip drag operations. Modifying the precision setting during a drag has the
following effects:

  - Updates the text and buttons on the Precision Control.
  - Changes the precision of the current operation (position, size, etc.).
  - Changes the precision of the :ref:`visual feedback <ug-feedback>` for the
    current operation.
  - :strong:`Changes the relative amount of drag motion.` This may be
    startling at first, but makes sense when you try it out. For example, if
    you want to move an object exactly 5.45 units to the right, you start out
    with 1 unit precision and drag it 5 units. While still dragging, increase
    precision to .1 unit and drag it .4 units more. This uses the same amount
    of drag motion that 4 units would at 1 unit precision, so it's much
    easier. Finally, increase precision again to .01 unit and drag another .05
    units to the right, again with the corresponding amount of drag motion.

Once you get used to this, it becomes relatively easy to achieve precise
dimensions and distances interactively.

.. _ug-feedback:

Visual Feedback
...............

.. incimage:: /images/AngularFeedback.jpg 200px right
.. incimage:: /images/LinearFeedback.jpg 200px right


Most interactive drag operations display some sort of visual feedback.
Operations that are essentially linear show the current dimension of that
change along a line, while rotations show the current angle. Multi-dimensional
changes such as uniform scaling and spherical rotation show feedback in all
affected dimensions. The precision of the feedback matches the current
:ref:`precision level <ug-precision-level>`.

Feedback is colored according to the :ref:`coordinate system convention
<ug-coordinate-system>` for the appropriate dimension(s). When a drag operation
is snapped to a :ref:`target <ug-targets>`, feedback is displayed in the active
target color.

.. _ug-targets:

Targets
.......

.. incimage:: /images/EdgeTarget.jpg   200px right
.. incimage:: /images/PointTarget.jpg  200px right

Precise interactive placement can be tedious, so |appname| offers extra help
for some operations using :newterm:`targets`. A target is a 3D object that you
can place in the scene to affect future operations. Targets can be used to
transfer information from one object to another, such as position, orientation,
or size. This can be very useful, for example, to align objects precisely.

There are two targets available in the application:

  - The :newterm:`Point Target` represents a :emphasis:`position` (the base
    sphere) and a :emphasis:`direction` (the arrow).
  - The :newterm:`Edge Target` represents a :emphasis:`length` and a
    :emphasis:`direction`.

The ring on the Point Target is used for :ref:`radial layout
<ug-radial-layout>`.

Targets must be active (visible) to have any effect. To activate or deactivate
the Point Target, click on the :ref:`Toggle Point Target
<ug-toggle-point-target>` icon.  To activate or deactivate the Edge Target,
click on the :ref:`Toggle Edge Target <ug-toggle-edge-target>` icon.

Each target appears initially in the center of the :ref:`stage <ug-stage>`. If
a target is obscured by models, you can use the :ref:`Tree Panel
<ug-tree-panel>` to hide those models temporarily.

Positioning Targets
,,,,,,,,,,,,,,,,,,,

Targets are positioned by dragging or :ref:`modified-dragging
<ug-modified-mode>` them; they change to the active target color while the drag
is in process. Targets can be placed either on the :ref:`stage <ug-stage>` or
any model in the scene. When dragging a target on the stage, the target will
snap to grid locations based on the current :ref:`precision level
<ug-precision-level>`.

.. incimage:: /images/EdgeTargetSnapped.jpg  200px right
.. incimage:: /images/PointTargetSnapped.jpg 240px right

The Point Target can be dragged by any part (except the ring). When dragging
over a model's surface, the target location will snap to vertices of the
model's triangular mesh when close enough, based on the current :ref:`precision
level <ug-precision-level>`. When this occurs, a :newterm:`snap indicator`
(sphere at the tip of the target's arrow) will appear.

Similarly, the Edge Target snaps to the closest edge of the model's mesh,
matching the length of that edge. (You may find it helpful to :ref:`show model
edges <ug-show-edges>`.) If the Edge Target is dragged from a model to the
:ref:`stage <ug-stage>`, it retains the length of the last edge it snapped to.
Clicking on the Edge Target reverses its direction, which can be useful when
performing :ref:`linear layout <ug-linear-layout>`.

.. incimage:: /images/EdgeTargetModSnapped.jpg   200px right
.. incimage:: /images/PointTargetModSnapped2.jpg 200px right
.. incimage:: /images/PointTargetModSnapped.jpg  240px right

:ref:`Modified-dragging <ug-modified-mode>` either target over a model uses the
rectangular bounds of the model instead of its mesh. The Point Target will snap
to the minimum, maximum, and center values of the bounds in each dimension. You
can use this feature to more easily attach the point target to important points
on objects with asymmetric meshes. Similarly, the Edge Target snaps to edges of
the of the model instead of its mesh; the target will snap to the nearest edge
of the bounds. You can use this feature to easily set the edge target length to
any dimension of a model's bounds, such as the width of the cylinder shown
here.

When modified-dragging the Point Target, the snap indicator sphere will appear
when any snapping occurs, and is :ref:`color-coded by dimension
<ug-coordinate-system>`. If snapping occurs in two or three dimensions, the
color will be the sum of the respective dimensions' colors. In the above left
image, the Point Target is snapped to the center of the front-right edge of the
cylinder's bounds. In the center image, the :ref:`precision level
<ug-precision-level>` has been increased to keep the target from always
snapping to a bounds edge, and the target is on the center line of the front
face of the bounds, but is not snapped in the Z (up) dimension.

.. todo::
   Ended here.

Effects
,,,,,,,

When the Point Target is active, most position-based drag operations (such as
translation) will snap to the target's position in all affected dimensions when
the .
When snapping occurs during one of these operations, both the [visual
feedback][Feedback] and target change to the active target color.

The point target can also be used for instantly positioning a model, and it has
other features that let it be used be used for radial layout; see [Radial
Layout][RadialLayout] below for details.

#### Positioning on Models or the Stage






To activate or deactivate a target, click on the corresponding toggle icon, as
described below. When activated, a visual representation of the target appears,
initially in the center of the [stage][Stage]. If the target is obscured by
models, you can use the [Tree Panel][TreePanel] to hide those models
temporarily.

Targets can be [dragged][ClickDrag] to other locations, as described
below. While a target is being dragged, its color changes from the standard
target color (cyan) to the active target color (orange).

### Edge Target

![The edge target][ImgEdgeTarget]{{rightimage(200)}}

The edge target indicates a length and orientation in the scene. To activate or
deactivate the edge target, click on the [Edge Target
Toggle][EdgeTargetToggle].

#### Effects

When the edge target is active, dragging during a size-based operation (such as
scaling a model or changing a cylinder or torus diameter) snaps to the target
length. When snapping occurs during one of these operations, both the
[feedback][Feedback] and the target change to the active target color.

The edge target can also be used for immediate linear layout. See [Linear
Layout][LinearLayout] below for details.

#### Positioning on Models or the Stage

## Layout

The point and edge targets can also be used to perform radial and linear
layout, respectively.

### Radial Layout

The point target can also be used to lay out [selected models][Selection] along
a circle or circular arc. All of the parameters of the layout are specified
using the features of the target. When they are all set, click on the [Radial
Layout icon][RadialLayoutAction] to lay out the selected models.

If exactly one model is selected and the Radial Layout icon is clicked, the
bottom center of the model will be moved to the point target's location and the
model's "up" (+Z) direction will be aligned with the point target's direction.

![][ImgPointTargetRadius]{{rightimage(140)}}

If two or more models are selected, clicking on the Radial Layout icon will lay
them out along a circle or arc. To set this up, you can use some extra features
of the point target. To use them, drag the ring of the target to make it
larger. As you do this, you will see [feedback][Feedback] above the arrow
showing the diameter of the ring, which will be the diameter of the circle or
arc for your layout. The radius follows the current [precision
level][PrecisionLevel].

![][ImgPointTargetRadiusActive]{{rightimage(140)}}

Intersecting the ring are two spokes, one green and one red. These are used to
modify the starting and ending angles for layout, respectively. If you drag the
green (starting) spoke, both spokes rotate around the circle. If you drag the
red (ending) spoke, only the red spoke moves. The arc connecting the spokes
shows the direction of the layout (green to red). If you want to reverse the
direction, drag the red spoke past the green one in the desired direction. As
you drag either spoke, color-coded feedback shows the angle of each spoke and
the subtended layout angle.  Note that the spoke angles also follow the current
[precision level][PrecisionLevel].

Once you have set up the point target with the correct position, orientation,
radius, and angles, clicking on the [Radial Layout icon][RadialLayoutAction]
will position all selected models with their bottom centers along the circle or
arc (without changing their orientations). The selection order determines the
order in which they are laid out: the [primary selection][Selection] will be at
the start angle, and the last-selected model will be at the end angle.

The point target retains the radial settings (diameter and angles) until you
reset it by dragging the ring back to its tiny state.

### Linear Layout

The edge target can be used to lay out two or more models along a line. The
direction and length of the target determine the offset between models. The
[primary selection][Selection] will not move. The center of the first secondary
selection will be placed at the offset from the primary model's center, and so
on for subsequent selected models, in selection order.

It may be useful to create a temporary model with the correct size as a layout
aid. For example, suppose you want to lay out 3 models along the X direction
with 6 units between their centers:

+ Create a box and scale it so that it is 6 units in size in X (using 1 unit
  [precision][PrecisionLevel].
+ Activate the edge target and drag it onto one of the box edges in the X
  direction. The target will then be exactly 6 units in length and lie along
  the X axis. Click on the edge target to flip it if it is pointing in the
  wrong direction.
+ [Delete][DeleteAction] the box if you no longer need it.
+ Select the primary selection - the one that will not move.
+ [Alt-select][Selection] the other models you want to lay out, in order.
+ Click on the [Linear Layout icon][LinearLayoutAction] to lay them out.
