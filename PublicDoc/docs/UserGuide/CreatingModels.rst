.. _ug-model-creation:

Creating Models
---------------

|appname| supports a variety of model types. Most have a :ref:`specialized tool
<ug-specialized-tools>` designed specifically for editing them. Some (such as
Sphere and Box models) have no specialized tools because there is nothing that
cannot be modifed with a :ref:`general tool <ug-general-tools>`].

.. _ug-model-types:

Model Types
...........

Models can be divided into three basic categories:

  - :newterm:`Primitive models` are created from scratch or imported from STL
    files.
  - :newterm:`Combined models` are created by combining other models in
    different ways. The resulting model may or may not have a specialized tool
    associated with it.
  - :newterm:`Converted models` are created by converting other types of models
    in order to apply a particular specialized tool. For example, if you want
    to create beveled edges on a Box model, you first convert it to a Beveled
    model, then use the specialized :ref:`Bevel Tool <ug-bevel-tool>` to modify
    the bevel.

These are all described in more detail in the following sections.

.. _ug-primitive-models:

Primitive Models
,,,,,,,,,,,,,,,,

.. incimage:: /images/PrimitiveRevSurf.jpg  -100px right
.. incimage:: /images/PrimitiveTorus.jpg    -100px right
.. incimage:: /images/PrimitiveSphere.jpg   -100px right
.. incimage:: /images/PrimitiveCylinder.jpg -100px right
.. incimage:: /images/PrimitiveBox.jpg      -100px right

|block-image|

.. incimage:: /images/PrimitiveImported.jpg -100px right
.. incimage:: /images/PrimitiveText.jpg     -100px right
.. incimage:: /images/PrimitiveExtruded.jpg -100px right

|block-image|

  - First row:

    - A :newterm:`Box model` represents a 3D box with the length of all sides
      equal to 4 units by default. There is no specialized tool for a Box.
    - A :newterm:`Cylinder model` represents a closed cylinder that by default
      has a diameter and height of 4 units. Its axis of symmetry is aligned
      with the +Z ("up") axis. The specialized :ref:`Cylinder Tool
      <ug-cylinder-tool>` allows you to change the top and bottom diameters
      independently.
    - A :newterm:`Sphere model` represents a sphere that has a diameter of 4
      units by default. There is no specialized tool for a Sphere.
    - A :newterm:`Torus model` represents a torus that by default has an outer
      diameter of 4 units and an inner diameter (height) of .8 units. Its axis
      of symmetry is aligned with the +Z ("up") axis. The specialized
      :ref:`Torus Tool <ug-torus-tool>` allows you to change the outer and
      inner diameters independently.
    - A :newterm:`RevSurf model` represents a closed surface of revolution
      created by rotating a 2D profile about the Z ("up") axis. The default
      profile consists of 3 points (the minimum allowed) that forms a surface
      with the widest diameter of 4 units and a height of 4 units. The
      specialized :ref:`Surface of Revolution Tool <ug-rev-surf-tool>` lets you
      edit the profile and also the sweep angle of the surface.

  - Second row:

    - An :newterm:`Extruded model` represents a closed profile extruded along
      the +Z (up) direction. Its default height is 4 units. The specialized
      :ref:`Extruded Tool <ug-extruded-tool>` allows you to edit the extruded
      profile.
    - A :newterm:`Text model` represents extruded 3D text, which is laid out by
      default at the origin along the +X axis, extruded 4 units along the +Z
      (up) direction. It is sized by default so that the characters are
      approximately 4 units in the Y dimension. The specialized :ref:`Text Tool
      <ug-text-tool>` allows you to change the text string, font, and character
      spacing.
    - An :newterm:`Imported model` is read from an STL file. The specialized
      :ref:`Import Tool <ug-import-tool>` is used to select or change the file
      to import from.

      .. note::

         A newly-created Imported model will be displayed as a dummy
         tetrahedron with an :ref:`invalid color <ug-model-colors>` until a
         valid STL file has been selected with the Import Tool.

         Also, many, many publicly-available STL models are not valid meshes
         (watertight, not self-intersecting) and will also appear with the
         invalid color. The :ref:`Info Panel <ug-info-panel>` can tell you why
         the mesh is considered to be invalid.

New primitive models are placed at the center of the :ref:`stage <ug-stage>`
unless the :ref:`Point Target <ug-targets>` is active, in which case the new
model is placed to align with it.

.. _ug-combined-models:

Combined Models
,,,,,,,,,,,,,,,

One of the main features of |appname| is the ability to create new models by
combining other models. When models are combined, the original models become
children of the combined model in the model hierarchy (as shown in the
:ref:`Tree Panel <ug-tree-panel>`). The original models can be modified later,
and the changes will be reflected in the combined model.

A combined model is defined in its own coordinate system, originally aligned
with the stage. That is, if you take two rotated models and create a combined
model from them, the combined model's axes will not be rotated like either of
the parts; they will align with the stage.

.. _ug-csg:

Constructive Solid Geometry (CSG)
:::::::::::::::::::::::::::::::::

.. incimage:: /images/CSGUnion.jpg        180px right
.. incimage:: /images/CSGOperands.jpg     180px right

A useful way to create a new model is to apply any of the three
:newterm:`constructive solid geometry (CSG)` operations to two or more models:

  - A :newterm:`CSG Union` is formed by adding all selected models together.
  - :newterm:`CSG Intersection` uses the intersection of all selected models.
  - :newterm:`CSG Difference` subtracts all :ref:`secondary selections from the
    primary selection <ug-selection>`. Unlike union and intersection, CSG
    difference is asymmetric, so the selection order really matters.

.. incimage:: /images/CSGDifference.jpg   180px right
.. incimage:: /images/CSGIntersection.jpg 180px right

These images show the original operand models, a box and a cylinder, and the
resulting CSG union, intersection, and difference (cylinder subtracted from the
box).

.. note::

   In some cases using the :ref:`Clip Tool <ug-clip-tool>` may be a sufficient
   and simpler method of geometric editing.

|block-image|

.. _ug-hull:

Convex Hull
:::::::::::

.. incimage:: /images/HullCapsule.jpg 180px right
.. incimage:: /images/HullSpheres.jpg 180px right

Another way to combine models is with the :newterm:`convex hull` operation,
which creates a new model from the `3D convex hull
<https://en.wikipedia.org/wiki/Convex_hull>`_ of the selected models. This can
be used to easily create different shapes. For example, here is an easy way to
create a capsule by applying the convex hull operation to two spheres.

.. incimage:: /images/HullDisk.jpg    180px right
.. incimage:: /images/HullTorus.jpg   180px right

The convex hull operation can sometimes be useful to apply to a single model,
if the model is not already convex. Here's an example of creating a rounded
disk as the convex hull of a torus.

.. note::

   There is no specialized tool for a Hull model.

.. _ug-converted-models:

Converted Models
,,,,,,,,,,,,,,,,

Certain operations require models to first be converted to a different type.
Each of these model types is created by :ref:`selecting <ug-selection>` one or
more models and clicking on the appropriate conversion tool icon. Once the
conversion is done, you can use the corresponding specialized tool to edit the
resulting models.

.. note::

   This two-step process (converting, then editing) makes it clear that a new
   type of model must be created in order to apply the editing operation. The
   converted model becomes a parent of the original model in the hierarchy (as
   shown in the :ref:`Tree Panel <ug-tree-panel>`). This scheme also simplifies
   the interface and also allows the original objects to be modified even after
   the conversion operation has been applied.

The converted model types and their tools are as follows:

  - A :newterm:`Bent model` bends a model along a circular arc defined by an
    axis and angle. The :ref:`Convert to Bent action <ug-convert-bend>` is used
    to convert the models and attach the specialized :ref:`Bend Tool
    <ug-bend-tool>` to let you change the axis and angle.
  - A :newterm:`Beveled model` applies a bevel or other profile to edges of a
    model. The :ref:`Convert to Bevel action <ug-convert-bevel>` is used to
    convert the models and apply a default bevel. The specialized :ref:`Bevel
    Tool <ug-bevel-tool>` is attached to the selection to allow you to modify
    the bevel profile.
  - A :newterm:`Clipped model` applies a clipping plane to a model. The
    :ref:`Convert to Clipped action <ug-convert-clip>` is used to convert the
    models and apply the default clipping plane that removes the top half of
    the primary model. The specialized :ref:`Clip Tool <ug-clip-tool>` is
    attached to let you modify the clipping plane.
  - A :newterm:`Mirrored model` mirrors a model about a plane. The
    :ref:`Convert to Mirrored action <ug-convert-mirror>` is used to convert
    the models and mirror them about the default mirroring plane (which mirrors
    left/right through the center of the primary model) and the specialized
    :ref:`Mirror Tool <ug-mirror-tool>` is attached to let you change the
    plane.
  - A :newterm:`Tapered model` tapers the vertices of a model along one of its
    principal axes. The :ref:`Convert to Tapered action <ug-convert-taper>` is
    used to convert the models and attach the specialized :ref:`Taper Tool
    <ug-taper-tool>` to let you change the axis and taper profile.
  - A :newterm:`Twisted model` twists the vertices of a model about an
    axis. The :ref:`Convert to Twisted action <ug-convert-twist>` is used to
    convert the models and attach the specialized :ref:`Twist Tool
    <ug-twist-tool>` to let you change the axis and twist angle.

.. note::

   A converted model synchronizes its transformations (scale, rotation, and
   translation) with the original model in some way. Any changes made to the
   transformations for either the original or converted model are applied
   to the other.

   For Bent, Beveled, and Twisted models, it makes more sense to apply the
   scale of the original model before the relevant operation. In these cases,
   the scales of the original model and the converted model differ, but are
   kept in sync to make this work.

.. _ug-model-names:

Model Names
...........

When |appname| creates a model of any type, a unique name is assigned to it
automatically by affixing an underscore and a number to its base model type,
such as :model:`Sphere_1` or :model:`Imported_13`. Model names can be seen in
the :ref:`Tree Panel <ug-tree-panel>` and edited with the :ref:`Name Tool
<ug-name-tool>`.

.. _ug-model-colors:

Model Colors
............

.. incimage:: /images/InvalidColors.jpg 200px right

Reasonable colors are chosen automatically for new models created in the
scene. The :ref:`Color Tool <ug-color-tool>` allows you to change the color of
any model for aesthetic reasons. (The colors have no effect on the 3D-printed
model.)

Two special model colors are outside the normal range as shown here:

  - Any model with an invalid mesh is colored like the model on the left to
    indicate that it will probably not print successfully. A mesh is invalid if
    it is not closed, has weird connectivity, or self-intersects.
  - If the :ref:`build volume <ug-build-volume>` is visible, any part of any
    model that lies outside of it will be colored as shown on the right in the
    image. This indicates that the model will likely not be printed
    successfully.

When an invalid mesh is detected, it will retain the invalid mesh color until
the problem is fixed. Changes made to such a model with the Color Tool will
still take effect, but you will not be able to see them while the model is
invalid. Also, some tools will be disabled when an invalid model is selected to
avoid compounding the problem.

One exception is the :ref:`convex hull <ug-hull>` operation, which is enabled
even for invalid meshes. This can be used as a quick way to "fix" an invalid
mesh if the hull is close to what you want.

.. _ug-show-edges:

Model Edge Display
..................

.. incimage:: /images/ModelEdges.jpg   200px right
.. incimage:: /images/ModelNoEdges.jpg 200px right

Clicking The :ref:`Show Edges Toggle <ug-toggle-show-edges>` icon lets you
display edges of your models as visible lines or hide them.  Showing them can
help you see how your models are tessellated into triangles for 3D printing or
to help place the :ref:`Point Target <ug-targets>` and :ref:`Edge Target
<ug-targets>`.

|block-image|
