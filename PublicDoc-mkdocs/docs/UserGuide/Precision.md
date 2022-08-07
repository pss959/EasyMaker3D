# Precision

MakerVR offers several features to help you create 3D models with precise
dimensions and placement.

## Precision Level

![The precision control][ImgPrecisionControl]{{rightimage(80)}}

MakerVR provides a {{term('variable precision level')}} to help you attain
exact values during drag operations. There are three levels, each affecting
both linear operations (such as scaling and translation) and angular operations
(such as rotation).

+ 1 unit / 15&deg;
+ .1 unit / 5&deg;
+ .01 unit / 1&deg;

Assuming [centimeter units][Units], the linear precision levels are 1cm, 1mm,
and .1mm, respectively.

{{tiny('If you need higher precision than the finest values here, you should
probably use a different application.')}}

There are three ways to change the current precision level:

+ Clicking on one of the the two buttons on the precision control. (Top button
  to increase to finer precision; bottom button to decrease to coarser
  precision.)
+ Using `X` and `Z` [keyboard shortcuts][Shortcuts] to increase and decrease
  levels, respectively.
+ Clicking the Up or Down buttons on the VR controller trackpad/joystick.

Note that the keyboard and controller shortcuts work even during drag and grip
drag operations. Modifying the precision setting during a drag has the
following effects:

+ Updates the precision control text.
+ Changes the precision of the result of the current operation (position, size,
  etc.).
+ Changes the precision of the [visual feedback][Feedback].
+ *Changes the relative amount of drag motion.* This may be startling at first,
  but makes sense when you try it out. For example, if you want to move an
  object exactly 5.45 units to the right, you start out with 1 unit precision
  and drag it 5 units. While still dragging, increase precision to .1 unit and
  drag it .4 units more. This uses the same amount of drag motion that 4 units
  would at 1 unit precision, so it's much easier. Finally, increase precision
  again to .01 unit and drag another .05 units to the right, again with the
  corresponding amount of drag motion.

Once you get used to this, it becomes relatively easy to achieve precise
dimensions and distances interactively.

## Visual Feedback

![Angular feedback][ImgAngularFeedback]{{rightimage(200)}}
![Linear feedback][ImgLinearFeedback]{{rightimage(200)}}

Most interactive drag operations display some sort of visual feedback.
Operations that are essentially linear show the current dimension of that
change along a line, while rotations show the current angle. Multi-dimensional
changes such as uniform scaling and spherical rotation show feedback in all
affected dimensions. The precision of the feedback matches the current
[precision level][PrecisionLevel].

Feedback is colored according to the [coordinate system convention][Colors] for
the appropriate dimension(s). When a drag is snapped to a [target][Targets],
feedback is displayed in the active target color.

## Targets

Interactive placement can sometimes be tedious, so MakerVR offers extra help
for some operations using {{term('targets')}}. There are two targets available
in the application, the {{term('point target')}} and {{term('edge target')}}.

To activate or deactivate a target, click on the corresponding toggle icon, as
described below. When activated, a visual representation of the target appears,
initially in the center of the [stage][Stage]. If the target is obscured by
models, you can use the [Tree Panel][TreePanel] to hide those models
temporarily.

Targets can be [dragged][ClickDrag] to other locations, as described
below. While a target is being dragged, its color changes from the standard
target color (cyan) to the active target color (orange).

When dragging a target on the stage, the target will snap to grid locations
based on the current [precision level][PrecisionLevel].

### Point Target

![The point target][ImgPointTarget]{{rightimage(200)}}

The point target indicates a position (shown by the sphere at the base) and
orientation (shown by the arrow) in the scene. To activate or deactivate the
point target, click on the [Point Target Toggle][PointTargetToggle].

#### Effects

When the point target is active, most position-based drag operations (such as
translation) will snap to the target's position in all affected dimensions.
When snapping occurs during one of these operations, both the [visual
feedback][Feedback] and target change to the active target color.

The point target can also be used for instantly positioning a model, and it has
other features that let it be used be used for radial layout; see [Radial
Layout][RadialLayout] below for details.

#### Positioning on Models or the Stage

The point target can be dragged by any part (except the ring) to any point on a
model or on the [stage][Stage]. When dragging over a model's surface, the
target location will snap to vertices of the model's triangular mesh when close
enough to them, based on the current [precision level][PrecisionLevel]. When
this occurs, a {{term('snap indicator')}} (sphere at the tip of the target's
arrow) will appear.

[Alt-dragging][AltMode] the point target on a model uses the rectangular bounds
of the model instead of its mesh. The target will snap to the minimum, maximum,
and center values of the bounds in each dimension. You can use this feature to
more easily attach the point target to important points on objects with
asymmetric meshes.

When alt-dragging the point target, the snap indicator sphere will appear when
any snapping occurs, and is [color-coded by dimension][Colors]. If snapping
occurs in two dimensions, the color will be the sum of the two dimensions'
color. For example, if alt-dragging the point target on the front (-Y) face of
a model's bounds, the indicator will be red (X) when snapping to the left edge,
center, or right edge of the face. It will be blue (Z) when snapping to the
bottom edge, center, or top edge of the face. It will be magenta (red + blue)
when snapping to both at the same time.

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

When dragged over the surface of a model, the edge target snaps to the closest
edge of the model's mesh. (To see model edges more clearly, you can [show the
edges][ShowEdges].) If the edge target is dragged from a model to the
[stage][Stage], it retains the length of the last edge it snapped to.

[Alt-dragging][AltMode] the edge target on a model uses the rectangular bounds
of the model instead of its mesh; the target will snap to the nearest edge of
the bounds. You can use this feature to easily set the edge target length to
any dimension of a model's bounds.

Clicking on the edge target reverses its direction, which is useful when
performing [linear layout][LinearLayout].

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
