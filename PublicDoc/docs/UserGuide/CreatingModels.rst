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

.. incimage:: /images/PrimitiveImported.jpg 100px right
.. incimage:: /images/PrimitiveText.jpg     100px right
.. incimage:: /images/PrimitiveRevSurf.jpg  100px right
.. incimage:: /images/PrimitiveTorus.jpg    100px right
.. incimage:: /images/PrimitiveSphere.jpg   100px right
.. incimage:: /images/PrimitiveCylinder.jpg 100px right
.. incimage:: /images/PrimitiveBox.jpg      100px right

|block-image|

  - A :newterm:`Box model` represents a 3D box with the length of all sides
    equal to 4 units by default. There is no specialized tool for a Box.
  - A :newterm:`Cylinder model` represents a closed cylinder that by default
    has a diameter and height of 4 units. Its axis of symmetry is aligned with
    the +Z ("up") axis. The specialized :ref:`Cylinder Tool <ug-cylinder-tool>`
    allows you to change the top and bottom diameters independently.
  - A :newterm:`Sphere model` represents a sphere that has a diameter of 4
    units by default. There is no specialized tool for a Sphere.
  - A :newterm:`Torus model` represents a torus that by default has an outer
    diameter of 4 units and an inner diameter (height) of .8 units. Its axis of
    symmetry is aligned with the +Z ("up") axis. The specialized :ref:`Torus
    Tool <ug-torus-tool>` allows you to change the outer and inner diameters
    independently.
  - A :newterm:`RevSurf model` represents a closed surface of revolution
    created by rotating a 2D profile about the Z ("up") axis. The default
    profile consists of 3 points (the minimum allowed) that forms a surface
    with the widest diameter of 4 units and a height of 4 units. The
    specialized :ref:`Surface of Revolution Tool <ug-rev-surf-tool>` lets you
    edit the profile and also the sweep angle of the surface.
  - A :newterm:`Text model` represents extruded 3D text, which is laid out by
    default at the origin along the +X axis, extruded 4 units along the +Z (up)
    direction. It is sized by default so that the characters are approximately
    4 units in the Y dimension. The specialized :ref:`Text Tool <ug-text-tool>`
    allows you to change the text string, font, and character spacing.
  - An :newterm:`Imported model` is read from an STL file. The specialized
    :ref:`Import Tool <ug-import-tool>` is used to select or change the file to
    import from.

.. _ug-combined-models:

Combined Models
,,,,,,,,,,,,,,,

One of the main features of |appname| is the ability to create new models by
combining other models. When models are combined, the original models become
children of the combined model in the model hierarchy (as shown in the
:ref:`Tree Panel <ug-tree-panel>`). The original models can be modified later,
and the changes will be reflected in the combined model.

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

Note that in some cases using the :ref:`Clip Tool <ug-clip-tool>` may be a
sufficient and simpler method of geometric editing.

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

Note that there is no specialized tool for a Hull model.

.. _ug-converted-models:

Converted Models
,,,,,,,,,,,,,,,,

Certain operations require models to first be converted to a different type.

Each of these model types is created by :ref:`selecting <ug-selection>` one or
more models and clicking on the appropriate conversion tool icon. Once the
conversion is done, you can use the corresponding specialized tool to edit
the resulting models, as listed below.

Note that this two-step process (converting, then editing) makes it clear that
a new type of model must be created in order to apply the editing operation.
The converted model becomes a parent of the original model in the hierarchy (as
shown in the :ref:`Tree Panel <ug-tree-panel>`). This scheme also simplifies the
interface and also allows the original objects to be edited even after the
conversion operation has been applied.

  - A :newterm:`Beveled model` applies a bevel or other profile to edges of a
    model. The :ref:`Convert to Bevel action <ug-convert-bevel>` is used to
    convert the models, and the specialized :ref:`Bevel Tool <ug-bevel-tool>` is
    used to modify the bevel profile.
  - A :newterm:`Clipped model` applies one or more clipping planes to a
    model. The :ref:`Convert to Clipped action <ug-convert-clip>` is used to
    convert the models and the specialized :ref:`Clip Tool <ug-clip-tool>` is
    used to specify the clipping plane or planes.
  - A :newterm:`Mirrored model` mirrors a model about one or more principal
    planes. The :ref:`Convert to Mirrored action <ug-convert-mirror>` is used
    to convert the models and the specialized :ref:`Mirror Tool
    <ug-mirror-tool>` is used to specify how to mirror.

.. _ug-model-names:

Model Names
...........

.. todo::
   Ended here...

When |appname| creates a model of any type, a unique name is assigned to it
automatically by affixing an underscore and a number to its base model type,
such as `"Sphere_1"` or `"Imported_13"`. Names can be seen and changed in the
[Tree Panel][TreePanel].

## Model Colors

![][ImgSpecialMeshColors]{{rightimage(200)}}

Reasonable colors are chosen automatically for new models created in the
scene. The [Color Tool][ColorTool] allows you to change them for aesthetic
reasons; the colors have no effect on the 3D-printed model.

Two special colors are used for models that have invalid meshes (bright red) or
that at least partly lie outside the current [build volume][BuildVolume]
(bright cyan) if the build volume is visible, as shown here.

When an invalid mesh is detected, it will retain the invalid mesh color until
the problem is fixed. Changes made to such a model with the Color Tool will
still take effect, but you will not be able to see them while the model is
invalid. Also, some tools will be disabled when an invalid model is selected to
avoid compounding the problem.

## Model Edge Display

![][ImgEdges]{{rightimage(140)}}

Clicking The [Show Edges Toggle][ShowEdgesToggle] icon lets you display edges
of your models as visible lines or hide them.  Showing them can help you see
how your models are tessellated into triangles for 3D printing or to help place
the [Edge Target][EdgeTarget].

Note that this feature is not available on OSX, due to [shader
limitations](https://docs.unity3d.com/Manual/Metal.html); it is always disabled
on that platform.

