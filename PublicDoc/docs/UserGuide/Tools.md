# Tools

Tools are interactive objects that attach to models in the scene to perform a
specific type of editing.

Tools are divided into two categories:

+ A {{term('general tool')}} can be attached to any model, regardless of its
  type*. Every general tool is a {{term('3D tool')}} that appears around or
  above the model being edited.
+ A {{term('specialized tool')}} can be applied only to a model of a specific
  type. It is used to modify unique editable features of that type of model.
  Some specialized tools are 3D tools, and some are {{term('panel tools')}},
  meaning that they use a special [2D GUI panel][GUIPanels] for interactive
  editing.

*There is actually one exception to the general tools rule. The [Complexity
Tool][ComplexityTool] cannot be used for certain model types.

## Tool Attaching and Switching

MakerVR maintains a {{term('current general tool')}}, which is generally the
last general tool that was used. It is initially the [Translation
Tool][TranslationTool]. You can switch to any other enabled general tool by
using the mouse or laser/pinch to select its icon on the [General Tool
Shelf][GeneralToolShelf].

You can also switch general tools with the
[Switch-to-Previous-Tool][SwitchToPreviousToolAction] or
[Switch-to-Next-Tool][SwitchToNextToolAction] actions. The
[shortcuts][Shortcuts] for these are the `[` and `]` keys or the Left and Right
buttons on either controller's trackpad/joystick.

As mentioned previously, some [types of models][ModelTypes] have corresponding
specialized tools. When a model of one of these types is created, the
corresponding specialized tool is automatically attached to it so you can edit
it. If the new model has no specialized tool, the current general tool will be
attached.

Switching between the current general tool and the specialized tool for a model
(if it has one) is done with the [Specialized-Tool toggle
action][SpecializedToolToggle]. The [shortcut][Shortcuts] for this is the
spacebar on the keyboard or the Center buttons on either controller's
trackpad/joystick. When a specialized tool is available or active, the 3D icon
for the toggle changes to reflect the tool. The 3D icon will be the same as the
3D icon of the action used to create a specialized model of that type (whether
through creation or conversion) with a yellow edit pencil in front of it.

When changing the [selection][Selection], the following rules apply:

+ If the last attached tool was any specialized tool and all selected models
  support the same specialized tool (not necessarily the current one), that
  tool will be attached to the primary selection.
+ In all other cases, the current general tool will be attached to the primary
  selection.

## General Tools

Each of these general tools can be activated by clicking on the corresponding
icon on the [general tool shelf][GeneralToolShelf] or by switching to it as
described above.

### Color Tool
![][ImgColorToolActive]{{rightimage(140)}}

The Color Tool edits the color of all selected models. It is placed above the
primary selection. The disc in the center of the tool shows the current color
of the primary selection, as does the small marker in the outer ring. Clicking
or dragging anywhere in the ring moves the marker and changes the color of all
selected models. Grip-dragging works as well; the relative position of the
controller is used to move the marker around the ring.

Colors have no real effect on the 3D-printed model, but they can be used to
distinguish different parts while editing. Or just to make things look nicer.

### Complexity Tool

The Complexity Tool appears above the primary selection and changes the number
of triangles used to represent all selected models.

![][ImgHighComplexity]{{rightimage(140)}}
![][ImgLowComplexity]{{rightimage(140)}}

Dragging the slider handle on the Complexity Tool updates the number of
triangles used for all affected selected models. Grip-dragging also works; the
relative position of the controller moves the handle. These images show two
positions of the handle applied to a cylinder's complexity.

Some important notes:

+ The Complexity Tool has no effect on certain models and its icon will be
  disabled for those: Box models, Imported models, Combined models, or any
  model converted from one of these.
+ If you create a [Converted model][ConvertedModels] from a model that supports
  complexity changes, the Complexity Tool will actually be modifying the
  complexity of the child model.
+ If you want to change the complexity of parts of a [Combined
  model][CombinedModels], you can [select its child models][SelectInHierarchy]
  to modify their complexity; the Combined model will update appropriately when
  its children are no longer selected.

Note that you can use complexity to create different primitive models. The
lowest complexity Cylinder model is a triangular prism, and the lowest
complexity Sphere model is an octohedron. You can [show the polygonal
edges][ShowEdges] to see the triangles more clearly as in the above images.

### Rotation Tool

The Rotation Tool lets you rotate selected models. It has 3 color-coded rings
for rotating about the principal axes and a central sphere for doing free
spherical rotation. The rotation angle follows the current [precision
level][PrecisionLevel] setting and snaps to the current [point target
direction][PointTarget] if the target is active.

Color-coded [feedback][Feedback] shows the current rotation angle or angles
during an interactive drag.

![][ImgRotationToolActive]{{rightimage(140)}}

The [primary selection][Selection] is always rotated about its center point.
If multiple models are selected, secondary selections are rotated around the
same point, meaning that they revolve around the primary selection. However,
[alt-dragging][AltMode] the Rotation Tool causes all models to be rotated in
place about their own centers.

The [Axis-Aligned toggle][AxisAlignedToggle] affects how the Rotation Tool is
attached to the primary selection. If the toggle is active, then the rings will
always be aligned with the principal axes of the [stage][Stage], rather than
with the principal axes of the primary selection's local coordinates.

![][ImgRotationToolActiveVR]{{rightimage(180)}}

Grip-dragging uses the orientation of the VR controller to define the
rotation. A [hover guide][ClickDrag] attached to each controller hand indicates
what action will be performed. Note that the hover guide for rotation points
forward from the hand, unlike most other hover guides.  When the ring on the
guide is close to aligning with any of the axis rings on the Rotation Tool, the
guide will show a [color-coded connection][Colors] to that ring. (In the
picture here, the ring is close to aligning with the red X-axis rotation ring
of the Rotation Tool.) In any other orientation, the guide will show a
neutral-color connection to the center sphere. If a ring is hovered, pressing
the grip button and rotating the controller ring around the guide axis will
rotate the model(s) the same way. When no ring is selected, the model(s) will
be free rotated to match the controller orientation.

### Scale Tool

![][ImgScaleToolActive]{{rightimage(160)}}

The Scale Tool lets you change the size of selected models. It has several
double-ended sliders:

+ Three sliders aligned with the coordinate axes of the model. These scale
  nonuniformly in one dimension.
+ Eight sliders passing diagonally through edge centers. These scale
  nonuniformly in two dimensions.
+ Four sliders passing diagonally through the corners of the model's
  bounds. These scale uniformly in all three dimensions.

Any dimension being scaled follows the current [precision
level][PrecisionLevel] and snaps to the current [edge target
length][EdgeTarget] if the edge target is active. Color-coded
[feedback][Feedback] shows the current relevant dimension(s) of the primary
selection during a drag.

There are two scaling modes:

+ {{term('Asymmetric scaling')}} resizes the model about the opposite
  point. That is, when you drag a slider handle, the handle on the other end
  stays fixed. This is the default scaling mode when dragging a handle.
+ {{term('Symmetric scaling')}} resizes the model about its center. When
  dragging a slider handle in this mode, the handle on the opposite side moves
  the same amount in the other direction. [Alt-dragging][AltMode] a slider
  handle performs symmetric scaling.

If multiple models are selected, all secondary selections are scaled in place
in their local orientations by corresponding ratios. Note that the
[Axis-Aligned toggle][AxisAlignedToggle] has no effect on the Scale Tool, since
using different axes might cause confusing shearing to occur.

![][ImgScaleToolActiveVR]{{rightimage(200)}}

Grip-dragging uses the orientation of a VR controller to select a slider handle
to activate. A [hover guide][ClickDrag] attached to each controller hand shows
the direction that needs to align with the slider to activate its handle. When
the guide is close enough to being aligned with a slider, the guide will show a
[color-coded connection][Colors] (neutral if not a 1-dimensional slider) to the
closest handle of that slider. When a handle is highlighted in this manner,
pressing the grip button and moving the controller along the slider direction
scales the model(s).

### Translation Tool

![][ImgTranslationToolActive]{{rightimage(160)}}

The Translation Tool lets you change the position of selected models. It adds a
double-ended slider along each of the principal axes. Dragging the handle at
either end translates all selected models along the corresponding axis.
Translation offsets follow the current [precision level][PrecisionLevel]
setting. It also snaps to the current [point target location][PointTarget] if
the target is active; snapping is done when the minimum, center, or maximum
value of the primary model's bounds in the translated dimension is aligned with
the target's position. Color-coded [feedback][Feedback] shows the current
translation amount during a drag.

The [Axis-Aligned toggle][AxisAlignedToggle] affects how the Translation Tool
is attached to the primary selection. If the toggle is active, then the sliders
will always be aligned with the principal axes of the [stage][Stage], rather
than with the local coordinates of the primary selection.

![][ImgTranslationToolActiveVR]{{rightimage(200)}}

Grip-dragging uses the orientation of the VR controller to select a slider
handle to activate. A [hover guide][ClickDrag] attached to each controller hand
shows the direction that needs to align with the slider to activate its
handle. When the guide is close enough to being aligned with a slider, the
guide will show a [color-coded][Colors] connection to the closest handle of
that slider. When a handle is highlighted in this manner, pressing the grip
button and moving the controller along the slider direction translates the
model(s) along the axis in either direction (pull or push).

## Specialized Tools

The following sections describe the various specialized tools. The 3D
specialized tools are listed first; the tools that use [2D GUI
panels][GUIPanels] for interaction are grouped together at the end of this
section.

### Clip Tool

The Clip Tool that allows you to remove parts of models with one or more
clipping planes. It is enabled when all of the selected models are Clipped
models. You can convert any model to a Clipped model with the
[Convert-to-Clipped action][ConvertToClippedAction].

This tool has three interactive parts:

![][ImgClipToolInactive]{{rightimage(180)}}

+ An arrow indicating the normal to the clipping plane. The part of the
  selected model(s) on the side of the plane with the normal is what will be
  clipped away when the plane is clicked. Dragging the arrow lets you
  reposition the plane along the normal.
+ A translucent sphere that can be rotated to change the orientation of the
  clipping plane.
+ A translucent quadrilateral representing the clipping plane. Clicking this
  quadrilateral adds a clipping plane to all selected Clipped models.

![][ImgClipToolClipped]{{rightimage(180)}}

Translating the plane by dragging the arrow is limited by the extents of the
primary model; it will not let you move the plane completely off this model.
The plane will snap to the [point target location][PointTarget] if the target
is active or to the center of the primary selection. The plane will change
color to the target color when it is snapped to either point.
[Alt-dragging][AltMode] the arrow deactivates any snapping.

When rotating the plane by dragging the sphere, the plane normal will snap to
the [point target direction][PointTarget] if the target is active or to any of
the principal axes. If the [Axis-Aligned toggle][AxisAlignedToggle] is active,
the principal axes of the [stage][Stage] are used; otherwise, the local axes of
the primary model are used. The plane will change color when snapped to either
the target color or the [color corresponding to a principal axis][Colors].
[Alt-dragging][AltMode] the sphere deactivates any snapping.

During interaction (translating or rotating), all selected objects will be
clipped in real time to show what will happen if the plane is clicked. The
real-time clipping stops when interaction ends.

Grip-dragging works for both translation and rotation. If the controller is
oriented so that the [hover guide][ClickDrag] attached to the controller is
nearly parallel to the plane normal arrow, grip-dragging will translate the
plane along the normal based on the controller position. Otherwise,
grip-dragging will rotate the sphere and plane based on the controller
orientation.  Snapping occurs as above unless [alt-dragging][AltMode].

Any number of clipping planes can be applied to the same clipped model. When
the Clip Tool is attached to a clipped model, it aligns itself with the most
recent clipping plane applied to that model, if any.

### Cylinder Tool

The Cylinder Tool is enabled when all selected models are Cylinder models. It
allows the top and bottom radii of all selected Cylinder models to be changed
to create cones or truncated cones.

![][ImgCylinderToolActive]{{rightimage(180)}}

The tool consists of two double-ended sliders, one at the top and one at the
bottom. The pair of handles on each slider always work symmetrically. The
radius being modified follows the current [precision level][PrecisionLevel]
setting and also snaps to the current [edge target length][EdgeTarget] if the
target is active.  [Feedback][Feedback] shows the length of the current radius
during a drag.

Grip-dragging also works with the radius sliders. A [hover guide][ClickDrag]
attached to each controller shows which slider handle will be activated when
the grip button is pressed. The relative vertical position of the controller
chooses the top or bottom radius.

### Mirror Tool

The Mirror Tool is enabled when all selected models are [Mirrored
models][ConvertedModels]. It mirrors the models across one or more principal
planes. When the tool is active, it adds 3 color-coded orthogonal planes to the
primary selection.  Clicking on any of the planes mirrors the model across it.

![][ImgMirrorToolActive]{{rightimage(180)}}

The mirroring planes always pass through the center of the primary selection.
If multiple Mirrored models are selected, all secondary selections are mirrored
across the same planes, meaning that they will move to the opposite side of the
plane if they are not also bisected by the plane, in addition to being
mirrored. However, [alt-clicking][AltMode] on a mirroring plane causes each
Mirrored model to be mirrored in place as if the plane passed through its local
center.

The [Axis-Aligned toggle][AxisAlignedToggle] affects how the Mirror Tool is
attached to the primary selection when the primary selection has been
rotated. If the toggle is active, then the mirroring planes will always be
aligned with the principal planes of the [stage][Stage] rather than with the
local coordinates of the primary selection.

### Torus Tool

The Torus Tool is enabled when all selected models are Torus models. It allows
the inner and outer diameters of the selected Torus models to be changed.

![][ImgTorusToolActive]{{rightimage(180)}}

The tool consists of two double-ended sliders, one horizontal and one
vertical. The horizontal slider changes the outer diameter, and the vertical
slider changes the inner diameter. The handles always operate symmetrically.
The diameter being modified follows the current [precision
level][PrecisionLevel] setting and also snaps to the current [edge target
length][EdgeTarget] if the target is active. [Feedback][Feedback] shows the
length of both diameters during a drag of either slider.

Note that if the inner diameter is increased too much, the outer diameter will
be increased as well to keep the torus from intersecting itself. Reducing the
inner diameter during the same drag operation will reduce the outer diameter as
well up to its previous size.

Grip-dragging also may be used to modify either diameter. A [hover
guide][ClickDrag] attached to each controller shows which slider handle will be
activated when the grip button is pressed. The relative orientation (closer to
horizontal or vertical) of the controller and guide chooses the outer or inner
diameter slider.

### GUI Panel Tools

Each of these specialized tools is a [2D GUI Panel][GUIPanels] that appears in
the scene above the primary selection (although it may be moved). The details
of all panel interactions are described in the GUI Panel section.

The following table shows the model type each of these specialized tools is
associated with; the tool is enabled only when all selected models are of that
type. The third column names the 2D GUI Panel used to implement its
interaction. Sharp readers will notice a pattern in the table.

<div class="tool-table" markdown="1">
Tool Name | Model Type | 2D GUI Panel
--------- | ---------- | ------------
{{anchor('bevel-tool')}}Bevel Tool | Beveled model | [Bevel Tool Panel][BevelToolPanel]
{{anchor('csg-tool')}}CSG Tool | CSG model | [CSG Tool Panel][CSGToolPanel]
{{anchor('import-tool')}}Import Tool | Imported model | [Import Tool Panel][ImportToolPanel]
{{anchor('rev-surf-tool')}}RevSurf Tool | RevSurf model | [RevSurf Tool Panel][RevSurfToolPanel]
{{anchor('text-tool')}}Text Tool | Text model | [Text Tool Panel][TextToolPanel]
</div>
