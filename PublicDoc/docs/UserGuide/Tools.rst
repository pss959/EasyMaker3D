.. _ug-tools:

Tools
-----

:newterm:`Tools` attach to :ref:`selected models <ug-selection>` in the scene,
adding interaction to perform a specific type of editing.

Tools are divided into two categories:

  - A :newterm:`general tool` can be attached to any model, regardless of its
    type (except for the :ref:`Complexity Tool <ug-complexity-tool>`).
  - A :newterm:`specialized tool` can be applied only to a model of a specific
    type. It is used to modify unique editable features of that type of model.

Some tools add interactive 3D handles around or above the primary selection;
others use 2D :ref:`panels <ug-panels>` for editing.

Attaching and Switching Tools
.............................

|appname| maintains a :newterm:`current general tool`, which is usually the
last general tool that was used. The :ref:`Translation Tool
<ug-translation-tool>` is the default current general tool when the application
starts. You can switch to any other enabled general tool by applying the
corresponding :ref:`tool action <ug-tool-actions>`. The :action:`Switch To Next
Tool` and :action:`Switch To Previous Tool` actions also allow you to rotate
through the available general tools.

If a specialized tool is available for the current selection, you can
:ref:`toggle between the current general tool and that specialized tool
<ug-toggle-specialized-tool>` with the :action:`Toggle Specialized Tool`
action.

As mentioned previously, :ref:`creating a new model <ug-model-creation>` that
has a corresponding specialized tool automatically attaches that tool so you
can edit the model. If the new model has no specialized tool, the current
general tool will be attached.

When changing the :ref:`current selection <ug-selection>`, the following rules
apply:

  - If the last attached tool was a specialized tool and all selected models
    support the same specialized tool (not necessarily the current one), that
    tool will be attached to the primary selection.
  - In all other cases, the current general tool will be attached to the
    primary selection.

.. _ug-general-tools:

General Tools
.............

With the exception of the :ref:`Complexity Tool <ug-complexity-tool>`, any of
the following general tools can be attached to all selected models.

.. _ug-color-tool:

Color Tool
,,,,,,,,,,

.. incimage:: /images/ColorTool.jpg 200px right

The :newterm:`Color Tool` changes the color of all selected models. The
interface for the tool is placed above the primary selection as shown in this
image. The disc in the center of the tool shows the current color of the
primary selection, as does the position of the small marker in the outer
ring. Clicking or dragging anywhere in the ring moves the marker and changes
the color of all selected models.

Colors have no real effect on the 3D-printed model, but they can be used to
distinguish different parts while editing. Or just to make things look nicer.

.. admonition:: VR Only

   :ref:`Grip dragging <ug-grip-dragging>` can also be used to change the
   color. The relative position of the controller is used to move the marker
   around the ring.

.. _ug-complexity-tool:

Complexity Tool
,,,,,,,,,,,,,,,

.. incimage:: /images/ComplexityToolHigh.jpg 200px right
.. incimage:: /images/ComplexityToolLow.jpg  200px right

The :newterm:`Complexity Tool` appears above the primary selection and can be
used to change the number of triangles used in the meshes that represent all
selected models. For models with curved surfaces, changing the complexity will
obviously affect the smoothness of the model.

The tool consists of a single horizontal 3D slider. Dragging the slider handle
changes the number of triangles used for all affected selected models. These
images show two positions of the handle applied to a Cylinder model's
complexity.

Some notes about this tool:

  - The Complexity Tool has effect only on the following :ref:`model types
    <ug-model-types>`: Cylinder, Sphere, Torus, RevSurf (surface of
    revolution), and Text. The tool is disabled if the selection contains only
    models of other types.
  - If you want to change the complexity of a child model of a
    :ref:`Converted model <ug-converted-models>` or :ref:`Combined model
    <ug-combined-models>` of any type you can :ref:`select the child or
    children <ug-select-hierarchy>` and apply the complexity change. The parent
    model will update appropriately when its children are no longer selected.
  - :ref:`Showing model edges <ug-show-edges>` can help you see the triangles
    more clearly as in these images.
  - You can use complexity to create models with certain basic shapes. The
    lowest complexity Cylinder model is a triangular prism, and the lowest
    complexity Sphere model is an octohedron.

.. admonition:: VR Only

   :ref:`Grip dragging <ug-grip-dragging>` also works; the relative position of
   the controller moves the handle.

.. _ug-name-tool:

Name Tool
,,,,,,,,,

.. incimage:: /images/NameTool.jpg 200px right

The :newterm:`Name Tool` lets you edit the name of the model that is the
primary selection. A model's name can help ypu distinguish its purpose or role,
and is used as the default file name for :ref:`STL export <ug-export>`.  Model
names can be seen in the :ref:`Tree Panel <ug-tree-panel>`.

The Name Tool is a panel-based tool that appears above the model. It has a text
input field that responds to the mouse and keyboard to edit the name along with
an "Apply" button that applies the changed name to the model. It will not let
you apply an invalid name (empty or with leading or trailing whitespace) or a
name that is already used by another model.

.. admonition:: VR Only

   If you are using the application in VR with the headset on, activating the
   text input field will bring up :ref:`a panel with the virtual keyboard
   <ug-virtual-keyboard-panel>` to allow text to be edited with the
   controllers.

.. _ug-rotation-tool:

Rotation Tool
,,,,,,,,,,,,,

.. incimage:: /images/RotationTool.jpg 200px right

The :newterm:`Rotation Tool` lets you rotate selected models. It has three
:ref:`color-coded <ug-dimension-colors>` rings for rotating about the principal
axes and a translucent central sphere for applying free spherical rotation.

Color-coded :ref:`angular feedback <ug-feedback>` shows the current rotation
angle or angles during an interactive drag with the tool.  The rotation angle
is snapped to the current :ref:`precision level setting <ug-precision-level>`
and snaps to the current :ref:`Point Target direction <ug-targets>` if the
target is visible.

The :ref:`primary selection <ug-selection>` is always rotated about its center
point. If multiple models are selected, secondary selections are also rotated
around the the center of the primary selection. However,
:ref:`modified-dragging <ug-modified-mode>` the Rotation Tool causes each model
to be rotated in place about its own center.

The :ref:`Axis-Aligned toggle <ug-toggle-axis-aligned>` affects how the
Rotation Tool is attached to the primary selection. If the toggle is not
active, the rings of the tool will align with the principal axes of the
selected model. If the toggle is active, the rings will align instead with the
principal axes of the :ref:`stage <ug-stage>`.

.. admonition:: VR Only

   .. incimage:: /images/RotationToolVRSphere.jpg 200px right
   .. incimage:: /images/RotationToolVRZ.jpg      200px right
   .. incimage:: /images/RotationToolVRX.jpg      200px right

   :ref:`Grip dragging <ug-grip-dragging>` uses the orientation of the VR
   controller to define the rotation. A special rotation grip hover guide will
   be used for each controller to show which part of the Rotation Tool will be
   activated for a grip drag, as follows.

   When the ring on the guide is close to aligning with any of the axis rings
   on the Rotation Tool, the guide will show a :ref:`color-coded connection
   <ug-dimension-colors>` to that ring.  In the left image here, the ring is
   close to aligning with the red X-axis rotation ring of the Rotation Tool, so
   a red connection appears. In the center image, it is close to aligning with
   the blue Z-axis ring. In any other orientation, the guide will show a
   neutral-color connection to the center sphere as in the right image.

   If a ring is hovered, grip dragging by rotating the controller parallel to
   the ring rotates the model(s) in the same direction. When no ring is
   selected, grip dragging will rotate the model(s) by the free controller
   orientation.

.. _ug-scale-tool:

Scale Tool
,,,,,,,,,,

.. incimage:: /images/ScaleTool.jpg 200px right

The :newterm:`Scale Tool` lets you change the size of selected models. It has
fifteen 3D :newterm:`scalers`, each of which is a resizable stick with an
interactive handle at each end. The handles are shaped to help indicate how
they scale the model:

  - Three :ref:`color-coded <ug-dimension-colors>` scalers are aligned with the
    coordinate axes of the model. These scale (nonuniformly) in a single
    dimension. Each handle of these scalers is a rectangular box that is longer
    in the dimension that will be scaled.
  - Eight scalers cross diagonally through the centers of edges. These scale
    nonuniformly in two dimensions. The handles of these scalers are cylinders
    that are aligned with the two scaled dimensions.
  - Four scalers pass diagonally through the corners of the model's
    bounds. These scale uniformly in all three dimensions. The handles of these
    scalers are spheres to indicate the uniform scale.

Any dimension being scaled follows the current :ref:`precision level
<ug-precision-level>` and snaps to the current :ref:`Edge Target length
<ug-targets>` if the target is active. Color-coded :ref:`linear feedback
<ug-feedback>` shows the current relevant dimension(s) of the primary selection
during a drag.

There are two scaling modes:

  - :newterm:`Asymmetric scaling` resizes the model about the opposite
    point. That is, when you drag a scaler handle, the handle on the other end
    stays fixed. This is the default scaling mode when dragging a handle.
  - :newterm:`Symmetric scaling` resizes the model about its center. When
    dragging a scaler handle in this mode, the handle on the opposite side
    moves the same amount in the other direction. :ref:`modified-dragging
    <ug-modified-mode>` a scaler handle performs symmetric scaling.

Keep in mind that scaling a model's height symmetrically can cause it to extend
under the :ref:`stage <ug-stage>`.

If multiple models are selected, all secondary selections are scaled in place
in their local orientations by the same ratio applied to the primary
selection. The :ref:`Axis-Aligned toggle <ug-toggle-axis-aligned>` has no
effect on the Scale Tool, since using different axes might cause confusing
shearing to occur.

.. admonition:: VR Only

   .. incimage:: /images/ScaleToolVRUniform.jpg 200px right
   .. incimage:: /images/ScaleToolVRX.jpg       240px right

   :ref:`Grip dragging <ug-grip-dragging>` uses the orientation of the VR
   controller to select a scaler handle to activate. The hover guide for each
   controller will show a :ref:`color-coded <ug-dimension-colors>` connection
   to the handle closest to the guide's orientation, if any.

   In the left image, the hover guide is close to being aligned with the red X
   dimension scaler, so grip dragging will activate that scaler. In the right
   image, the guide is close to aligned with a diagonal uniform scaler, so it
   will be activated for grip dragging.

.. _ug-translation-tool:

Translation Tool
,,,,,,,,,,,,,,,,

.. incimage:: /images/TranslationTool.jpg 200px right

The :newterm:`Translation Tool` lets you change the position of selected
models. It has three :ref:`color-coded <ug-dimension-colors>` double-ended 3D
sliders along each of the principal axes. Dragging the handle at either end
translates all selected models along the corresponding axis.

Color-coded :ref:`linear feedback <ug-feedback>` shows the current translation
distance during a drag.  Translation distances follow the current
:ref:`precision level <ug-precision-level>` setting. Translation also snaps to
the current :ref:`Point Target position <ug-targets>` if the target is
active. Snapping is done when the minimum, center, or maximum value of the
primary model's bounds in the translated dimension is aligned with the target's
position.

The :ref:`Axis-Aligned toggle <ug-toggle-axis-aligned>` affects how the
Translation Tool is attached to the primary selection. If the toggle is not
active, then the sliders will always be aligned with the principal axes of the
primary model. If the toggle is acive, the sliders will align instead with the
principal axes of the :ref:`stage <ug-stage>`.

.. admonition:: VR Only

   .. incimage:: /images/TranslationToolVRZ.jpg 200px right
   .. incimage:: /images/TranslationToolVRX.jpg 200px right

   :ref:`Grip dragging <ug-grip-dragging>` uses the orientation of the VR
   controller to choose a translation axis. The hover guide for each controller
   will show a :ref:`color-coded <ug-dimension-colors>` connection to the
   handle closest to the guide's orientation, if any, as shown in the images.

.. todo::
   Ended here.

.. _ug-bevel-tool:
.. _ug-clip-tool:
.. _ug-csg-tool:
.. _ug-cylinder-tool:
.. _ug-import-tool:
.. _ug-mirror-tool:
.. _ug-rev-surf-tool:
.. _ug-specialized-tools:
.. _ug-text-tool:
.. _ug-torus-tool:

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

Grip dragging works for both translation and rotation. If the controller is
oriented so that the [hover guide][ClickDrag] attached to the controller is
nearly parallel to the plane normal arrow, grip dragging will translate the
plane along the normal based on the controller position. Otherwise,
grip dragging will rotate the sphere and plane based on the controller
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

Grip dragging also works with the radius sliders. A [hover guide][ClickDrag]
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

Grip dragging also may be used to modify either diameter. A [hover
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
