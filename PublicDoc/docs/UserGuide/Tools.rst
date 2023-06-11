.. _ug-tools:

Tools
-----

:newterm:`Tools` are interactive objects that attach to :ref:`selected models
<ug-selection>` in the scene to perform a specific type of editing.

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

.. note::

   The :shortcut:`Space` shortcut is an easy way to apply this action to toggle
   between the current general tool and the specialized tool for the selection,
   if one is available.

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

Colors have no real effect on 3D-printed models, but they can be used to
distinguish different parts while editing, or just to make things look nicer.

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

.. _ug-complexity-model-types:

  - :ref:`Showing model edges <ug-show-edges>` can help you see the triangles
    more clearly as in these images.
  - You can use complexity to create models representing certain basic
    shapes. For example, the lowest complexity Cylinder model is a triangular
    prism, and the lowest complexity Sphere model is an octohedron.
  - If you want to change the complexity of a child model of a
    :ref:`converted model <ug-converted-models>` or :ref:`combined model
    <ug-combined-models>` of any type you can :ref:`select the child or
    children <ug-select-hierarchy>` and apply the complexity change. The parent
    model will update appropriately when its children are no longer selected.

.. note::

    The Complexity Tool has effect only on the following :ref:`types of models
    <ug-model-types>`: Bent, Cylinder, RevSurf (surface of revolution), Sphere,
    Text, Torus, and Twisted. The tool is disabled if the selection contains
    only models of other types.

.. admonition:: VR Only

   :ref:`Grip dragging <ug-grip-dragging>` also works; the relative position of
   the controller moves the handle.

.. _ug-name-tool:

Name Tool
,,,,,,,,,

.. incimage:: /images/NameTool.jpg 200px right

The :newterm:`Name Tool` is a panel-based tool that lets you edit the name of
the model that is the primary selection. A model's name can help ypu
distinguish its purpose or role, and is used as the default file name for
:ref:`STL export <ug-export>`.  Model names can be seen in the :ref:`Tree Panel
<ug-tree-panel>`.

When attached to a model, the Name Tool displays a :ref:`Name Tool Panel
<ug-name-tool-panel>` above the model. This panel has a text input field that
responds to the mouse and keyboard to edit the name along with an "Apply"
button that applies the changed name to the model. It will not let you apply an
invalid name (empty or with leading or trailing whitespace) or a name that is
already used by another model.

.. admonition:: VR Only

   If you are using the application in VR with the headset on, activating the
   text input field will bring up :ref:`the virtual keyboard panel
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

   .. incimage:: /images/RotationToolVRSphere.jpg -140px right
   .. incimage:: /images/RotationToolVRZ.jpg      -140px right
   .. incimage:: /images/RotationToolVRX.jpg      -140px right

   :ref:`Grip dragging <ug-grip-dragging>` uses the orientation of the VR
   controller to define the rotation. A special rotation grip hover guide will
   be used for each controller to show which part of the Rotation Tool will be
   activated for a grip drag, as follows.

   When the ring on the guide is close to aligning with any of the axis rings
   on the Rotation Tool, the guide will show a :ref:`color-coded connection
   <ug-dimension-colors>` to that ring.  In the left image here, the ring is
   close to aligning with the red X-axis rotation ring of the Rotation Tool, so
   a red connection appears. In the center image, it is close to aligning with
   the blue Z-axis ring. In any other orientation, the guide will show a yellow
   connection to the center sphere as in the right image.

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
    moves the same amount in the other direction.

.. note::

    Scaling is asymmetric by default.  :ref:`Modified-dragging
    <ug-modified-mode>` any scaler handle performs symmetric scaling.


Keep in mind that scaling a model's height symmetrically can cause it to extend
under the :ref:`stage <ug-stage>`.

If multiple models are selected, all secondary selections are scaled in place
in their local orientations by the same ratio applied to the primary
selection. The :ref:`Axis-Aligned toggle <ug-toggle-axis-aligned>` has no
effect on the Scale Tool, since using different axes might cause confusing
shearing to occur.

.. admonition:: VR Only

   .. incimage:: /images/ScaleToolVRUniform.jpg -140px right
   .. incimage:: /images/ScaleToolVRX.jpg       -140px right

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
translates all selected models along the corresponding axis. Color-coded
:ref:`linear feedback <ug-feedback>` shows the current translation distance
during a drag.

.. note::

   Translation distances and feedback follow the current :ref:`precision level
   <ug-precision-level>` setting. You can change the setting during use of this
   tool for more precise placement.

Translation also snaps to the current :ref:`Point Target position <ug-targets>`
if the target is active. Snapping is done when the minimum, center, or maximum
value of the primary model's bounds in the translated dimension is aligned with
the target's position.

The :ref:`Axis-Aligned toggle <ug-toggle-axis-aligned>` affects how the
Translation Tool is attached to the primary selection. If the toggle is not
active, then the sliders will always be aligned with the principal axes of the
primary model. If the toggle is acive, the sliders will align instead with the
principal axes of the :ref:`stage <ug-stage>`.

.. admonition:: VR Only

   .. incimage:: /images/TranslationToolVRZ.jpg -140px right
   .. incimage:: /images/TranslationToolVRX.jpg -140px right

   :ref:`Grip dragging <ug-grip-dragging>` uses the orientation of the VR
   controller to choose a translation axis. The hover guide for each controller
   will show a :ref:`color-coded <ug-dimension-colors>` connection to the
   handle closest to the guide's orientation, if any, as shown in the images.

   |block-image|

.. _ug-specialized-tools:

Specialized Tools
.................

The following sections describe the various specialized tools that are used to
edit :ref:`specific types of models <ug-model-types>`. Each specialized tool
can be attached to the :ref:`current selection <ug-selection>` only if all
selected models are of the correct type for that tool.

For more information about each specialized tool that uses a :ref:`panel
<ug-panels>` for its interface, see the corresponding panel documentation.

.. _ug-bend-tool:

Bend Tool
,,,,,,,,,

.. incimage:: /images/BendTool.jpg -200px right

The specialized :newterm:`Bend Tool` allows you to bend a model along a
circular arc relative to an axis. It is enabled when all of the selected models
are :ref:`Bent models <ug-converted-models>`. You can convert any model to a
Bent model with the :ref:`Convert Bend action <ug-convert-twist>`.

The math for a bend operation is fairly complicated. (Using this tool to see
how it works is probably much easier than reading about it.) A bend is based on
the following values:

  - An :newterm:`axis direction` around which the model is bent.
  - An :newterm:`axis center` that affects the bend radius.
  - A :newterm:`bend angle` that specifies the amount of bending.
  - An :newterm:`offset value` that defines how much to offset mesh vertices in
    the axis direction for each 360 degrees of bend. (This can be used to
    create a helix, for example.)

.. incimage:: /images/BendToolApplied.jpg -200px right

The Bend Tool has the following interactive parts:

  - An arrow showing the axis direction. The handles at the ends of the arrow
    can be dragged to rotate the axis direction. The shaft of the arrow can be
    dragged in the plane perpendicular to the axis direction to translate the
    axis center.
  - A spoked ring surrounding the model that can be dragged to change the bend
    angle. Feedback shows the current angle, which is snapped to the current
    :ref:`angle precision level setting <ug-precision-level>`.
  - A double-ended green arrow near the ring that is used to change the offset
    value. Feedback shows the current offset, which is the amount of
    translation applied for each 360 degrees of bend. The offset is snapped to
    the current :ref:`precision level setting <ug-precision-level>`.

The above image shows the result of changing the axis center, bend angle, and
offset applied to a rotated and scaled Cylinder model. Note that in this
example the complexity of the Bent model was changed with the :ref:`Complexity
Tool <ug-complexity-tool>` to increase the number of mesh vertices in the
direction of the bend.

When rotating the axis by dragging the arrow handles, the axis direction will
snap to the :ref:`Point Target direction <ug-targets>` if the target is active
or to any of the principal axes. If the :ref:`Axis-Aligned toggle
<ug-toggle-axis-aligned>` is active, the principal axes of the :ref:`stage
<ug-stage>` are used; otherwise, the local axes of the primary model are
used. The interactive ends of the arrow change color when snapped to either the
target color or the :ref:`color corresponding to a principal axis
<ug-dimension-colors>`.

When dragging the arrow shaft to translate the axis center, the center snaps to
the :ref:`Point Target position <ug-targets>` if the target is active and also
to the minimum, center, and maximum coordinates of the unbent primary model in
both dragging dimensions. This makes it easier to snap the center to important
points on the unbent model. The arrow shaft will change color to the active
target color when it is snapped.

.. note::

   :ref:`Modified-dragging <ug-modified-mode>` when rotating the axis or
   translating the center deactivates any snapping. It also deactivates
   snapping the bend angle or offset value to the current precision level
   during interaction.

.. admonition:: VR Only

   :ref:`Grip-dragging <ug-grip-dragging>` works as follows:

        - If the controller is oriented so that the hover guide is nearly
          parallel to the axis direction (or its opposite), grip dragging will
          let you rotate the axis direction by rotating the controller.
        - If the hover guide is nearly perpendicular to the axis direction,
          grip dragging will translate the axis center (by the arrow shaft).
        - Otherwise, grip dragging will let you change the bend angle by
          rotating the controller.

   Snapping occurs as above unless :ref:`modified-dragging <ug-modified-mode>`.

.. _ug-bevel-tool:

Bevel Tool
,,,,,,,,,,

.. incimage:: /images/BevelTool.jpg 200px right

The specialized :newterm:`Bevel Tool` is a panel-based tool used to edit the
bevel profile for one or more :ref:`Beveled models <ug-converted-models>`. A
Beveled model can be created from another model by applying the :ref:`Convert
Bevel action <ug-convert-bevel>`. Attaching the Bevel Tool displays a
:ref:`Bevel Tool Panel <ug-bevel-tool-panel>` above the primary selection.
Changes to the bevel profile in the panel are applied to all selected Beveled
models.

See the :ref:`Bevel Tool Panel documentation <ug-bevel-tool-panel>` for more
information about this tool.

|block-image|

.. _ug-clip-tool:

Clip Tool
,,,,,,,,,

.. incimage:: /images/ClipTool.jpg 200px right

The specialized :newterm:`Clip Tool` allows you to slice away parts of selected
models by using a clipping plane. It is enabled when all of the selected models
are :ref:`Clipped models <ug-converted-models>`. You can convert any model to a
Clipped model with the :ref:`Convert Clip action <ug-convert-clip>`.

The default clipping plane for a new Clipped model clips away the top half of
the primary selection, however it is oriented. The Clip Tool is then attached
to this plane to let you edit it. Whenever the Clip Tool is attached, it aligns
itself with the clipping plane already applied to the primary model.

This tool has two interactive parts:

  - An arrow indicating the normal to the clipping plane. The part of the
    selected model(s) on the side of the plane with the head of the arrow is
    clipped away. Dragging either end of the arrow rotates it around its center
    to change the orientation of the clipping plane.
  - A translucent rectangle showing the clipping plane relative to the primary
    selection. This rectangle can be dragged along the direction of the normal
    arrow to translate the clipping plane.

.. incimage:: /images/ClipToolActive.jpg 260px right

All selected Clipped models are clipped in real time by the current plane while
you rotate or translate the plane as shown here.

When rotating the plane by dragging the arrow, the plane normal will snap to
the :ref:`Point Target direction <ug-targets>` if the target is active or to
any of the principal axes. If the :ref:`Axis-Aligned toggle
<ug-toggle-axis-aligned>` is active, the principal axes of the :ref:`stage
<ug-stage>` are used; otherwise, the local axes of the primary model are
used. The interactive ends of the plane normal arrow change color when snapped
to either the target color or the :ref:`color corresponding to a principal axis
<ug-dimension-colors>`. 

|block-image|

.. incimage:: /images/ClipToolFeedback.jpg 260px right

When dragging the rectangle to translate the plane, :ref:`linear feedback
<ug-feedback>` shows the current size of the primary model along the plane
normal as shown here. The plane is limited to the extents of the primary model;
it cannot be moved completely off this model, since that would leave either
nothing or the entire model. While translating, the plane snaps to the
:ref:`Point Target position <ug-targets>` if the target is active and also to
the center of the primary selection. The feedback will change color to the
active target color when it is snapped to either point.

.. note::

   :ref:`Modified-dragging <ug-modified-mode>` when rotating or translating the
   clipping plane deactivates any snapping.

.. admonition:: VR Only

   :ref:`Grip-dragging <ug-grip-dragging>` works for both translation and
   rotation. If the controller is oriented so that the hover guide is nearly
   parallel to the plane normal arrow, grip dragging will translate the plane
   along the normal based on the controller position. Otherwise, grip dragging
   will rotate the plane based on the controller orientation.  Snapping occurs
   as above unless :ref:`modified-dragging <ug-modified-mode>`.

.. _ug-csg-tool:

CSG Tool
,,,,,,,,

.. incimage:: /images/CSGTool.jpg 200px right

The specialized :newterm:`CSG Tool` is a panel-based tool used to change the
CSG operation applied to one or more :ref:`CSG models
<ug-combined-models>`. Attaching the CSG Tool displays a :ref:`CSG Tool Panel
<ug-csg-tool-panel>` above the primary selection. Changes to the CSG operation
in the panel are applied to all selected CSG models.

|block-image|

.. _ug-cylinder-tool:

Cylinder Tool
,,,,,,,,,,,,,

.. incimage:: /images/CylinderTool.jpg 200px right

The specialized :newterm:`Cylinder Tool` is enabled when all of the selected
models are :ref:`Cylinder models <ug-primitive-models>`. It allows you to
change the top and bottom diameters of all selected Cylinder models to create
cones or truncated cones.

The Cylinder Tool has two :newterm:`scalers`, each of which is a resizable
stick with an interactive handle at each end. The handles work symmetrically;
dragging either handle changes the corresponding diameter of the selected
Cylinder models.

|block-image|

.. incimage:: /images/CylinderToolActive.jpg 200px right

The diameter being modified follows the current :ref:`precision level
<ug-precision-level>` setting and also snaps to the current :ref:`Edge Target
length <ug-targets>` if the target is active. :ref:`Linear feedback
<ug-feedback>` shows the current size of the diameter during a drag as shown
here.

.. admonition:: VR Only

   :ref:`Grip-dragging <ug-grip-dragging>` also works with the scalers. The
   relative vertical position of a controller chooses the top or bottom scaler.

.. _ug-extruded-tool:

Extruded Tool
,,,,,,,,,,,,,

.. incimage:: /images/ExtrudedTool.jpg 200px right

The specialized :newterm:`Extruded Tool` is a panel-based tool used to edit the
profile used for one or more :ref:`Extruded models <ug-primitive-models>`.
Attaching the Extruded Tool displays a :ref:`Extruded Tool Panel
<ug-extruded-tool-panel>` above the primary selection.  Changes to the
extrusion profile in the panel are applied to all selected Extruded models.

See the :ref:`Extruded Tool Panel documentation <ug-extruded-tool-panel>` for
more information about this tool.

|block-image|

.. _ug-import-tool:

Import Tool
,,,,,,,,,,,

.. incimage:: /images/ImportToolPanel.jpg 200px right

The specialized :newterm:`Import Tool` is a panel-based tool that is enabled if
there is exactly one :ref:`Imported model <ug-primitive-models>` selected. It
has three basic uses:

  - Choosing the initial file to import the model data from, right after
    creating the Imported model with the :ref:`Create Imported Model
    <ug-create-imported-model>` action.
  - Re-importing the model data from the same file after the file has changed.
  - Changing the file to import the model data from.

The Import Tool displays a :ref:`Import Tool Panel <ug-import-tool-panel>`
above the selected model. Accepting the same file for an Imported model will
re-import the data. Choosing a different file will load the data from that
file. Any errors during data import will be displayed in a :ref:`Dialog Panel
<ug-dialog-panel>`.

.. note::

   As :ref:`mentioned previously<ug-primitive-models>`, many publicly-available
   STL models are not valid solid meshes (watertight, not
   self-intersecting). The :ref:`Info Panel <ug-info-panel>` can tell you why
   a mesh is considered to be invalid.

|block-image|

.. _ug-mirror-tool:

Mirror Tool
,,,,,,,,,,,

.. incimage:: /images/MirrorToolAfter.jpg  -160px right
.. incimage:: /images/MirrorToolBefore.jpg -160px right

The specialized :newterm:`Mirror Tool` is enabled when all of the selected
models are :ref:`Mirrored models <ug-converted-models>`. You can convert any
model to a Mirrored model with the :ref:`Convert Mirror action
<ug-convert-mirror>`. The default mirroring plane is oriented with the primary
selection to mirror its local X (left-to-right) axis as shown here.

|block-image|

.. incimage:: /images/MirrorToolRotated.jpg -160px right

The Mirror Tool has the same interface as the Clip Tool for editing the
mirroring plane (including in VR); refer to :ref:`the Clip Tool
documentation<ug-clip-tool>` for details. For example, you can rotate the plane
to mirror vertically instead of horizontally as shown here.

Translating the mirroring plane results in the primary selection moving to
compensate. Additionally, all secondary selections are mirrored across the same
plane, so they will move to the other side of it.

|block-image|

.. _ug-rev-surf-tool:

RevSurf Tool
,,,,,,,,,,,,

.. incimage:: /images/RevSurfTool.jpg 200px right

The specialized :newterm:`RevSurf Tool` is a panel-based tool used to edit the
revolved profile for one or more :ref:`RevSurf models <ug-primitive-models>`.
Attaching the RevSurf Tool displays a :ref:`RevSurf Tool Panel
<ug-rev-surf-tool-panel>` above the primary selection.  Changes to the revolved
profile in the panel are applied to all selected RevSurf models.

See the :ref:`RevSurf Tool Panel documentation <ug-rev-surf-tool-panel>` for
more information about this tool.

|block-image|

.. _ug-taper-tool:

Taper Tool
,,,,,,,,,,

.. incimage:: /images/TaperTool.jpg -200px right

The specialized :newterm:`Taper Tool` is a panel-based tool used to edit the
taper profile or axis for one or more :ref:`Tapered models
<ug-converted-models>`. A Tapered model can be created from another model by
applying the :ref:`Convert Taper action <ug-convert-taper>`. Attaching the
Taper Tool displays a :ref:`Taper Tool Panel <ug-taper-tool-panel>` above the
primary selection.  Changes to the taper profile or axis in the panel are
applied to all selected Tapered models.

See the :ref:`Taper Tool Panel documentation <ug-taper-tool-panel>` for more
information about this tool.

|block-image|

.. _ug-text-tool:


Text Tool
,,,,,,,,,

.. incimage:: /images/TextTool.jpg 200px right

The specialized :newterm:`Text Tool` is a panel-based tool used to edit one or
more :ref:`Text models <ug-primitive-models>`.  Attaching the Text Tool
displays a :ref:`Text Tool Panel <ug-text-tool-panel>` above the primary
selection. The panel lets you change the text string, font, and character
spacing.  Clicking the "Apply" button in the panel applies all changes to all
selected Text models.

See the :ref:`Text Tool Panel documentation <ug-text-tool-panel>` for more
information about this tool.

|block-image|

.. _ug-torus-tool:

Torus Tool
,,,,,,,,,,,,,

.. incimage:: /images/TorusTool.jpg -180px right

The specialized :newterm:`Torus Tool` is enabled when all of the selected
models are :ref:`Torus models <ug-primitive-models>`. It allows you to change
the inner and outer diameters of all selected Torus models.

The Torus Tool has two :newterm:`scalers`, each of which is a resizable stick
with an interactive handle at each end. The handles work symmetrically;
dragging either handle changes the corresponding diameter of the selected Torus
models. The scaler for the outer diameter is aligned horizontally with the X
axis, and the scaler for the inner diameter is aligned vertically with the Z
axis.

|block-image|

.. incimage:: /images/TorusToolActive.jpg -180px right

The diameter being modified follows the current :ref:`precision level
<ug-precision-level>` setting and also snaps to the current :ref:`Edge Target
length <ug-targets>` if the target is active. :ref:`Linear feedback
<ug-feedback>` shows the current size of the diameter during a drag as shown
here.

If the inner diameter is increased sufficiently, the outer diameter will be
increased as well to keep the torus from intersecting itself. Reducing the
inner diameter during the same drag operation will reduce the outer diameter as
well up to its previous size.

.. admonition:: VR Only

   :ref:`Grip-dragging <ug-grip-dragging>` also works with the scalers. The
   relative orientation of the grip hover guide on the controller (closer to
   horizontal or vertical) chooses the outer or inner diameter scaler.

.. _ug-twist-tool:

Twist Tool
,,,,,,,,,,

.. incimage:: /images/TwistToolTwisting.jpg -200px right
.. incimage:: /images/TwistTool.jpg         -200px right

The specialized :newterm:`Twist Tool` allows you to twist the vertices of a
model around an axis. It is enabled when all of the selected models are
:ref:`Twisted models <ug-converted-models>`. You can convert any model to a
Twisted model with the :ref:`Convert Twist action <ug-convert-twist>`.

The Twist Tool has the same interface as the Bend Tool for editing the twist
axis direction, axis center, twist angle, and twist offset; refer to :ref:`the
Bend Tool documentation<ug-bend-tool>` for details.

Twisting a mesh holds the mesh vertices at the base (box end) of the axis in
place while the mesh vertices at the other end are twisted around the axis by
the twist angle. All mesh vertices between those extremes are twisted and
offset proportionally. The right image here shows a Box model being twisted.

.. incimage:: /images/TwistToolHighComplexity.jpg 200px right
.. incimage:: /images/TwistToolLowComplexity.jpg  200px right

.. note::

   A Twisted model is affected by the :ref:`Complexity Tool
   <ug-complexity-tool>`. Increasing the complexity of a Twisted model
   increases the number of slices (and vertices) in the direction of the axis
   as shown here. Also note that changing the complexity of the original model
   the twist is applied to may also affect the twist.

|block-image|

.. admonition:: VR Only

   Grip dragging works the same way as for the :ref:`Bend Tool<ug-bend-tool>`.
