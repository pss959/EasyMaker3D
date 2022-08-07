# Models

MakerVR supports a variety of different types of models. Many model types have
a [specialized tool][SpecializedTools] designed specifically for editing
them. Some (such as Sphere and Box models) have no specialized tools because
there is nothing that cannot be modifed with a [general tool][GeneralTools].

## Geometric Operations

One of the main features of MakerVR is the ability to create new models by
applying certain geometric operations.

### Constructive Solid Geometry (CSG)

One of the most useful ways to create a new model is to apply any of the three
{{term('constructive solid geometry (CSG)')}} operations to two or more models:

+ {{term('Union')}} adds all of the models together.
+ {{term('Intersection')}} uses the intersection of all selected models.
+ {{term('Difference')}} subtracts all secondary selections from the primary
  selection. Unlike the other two operations, difference is asymmetric, so the
  selection order really matters.

![][ImgCSGDifference]{{rightimage(100)}}
![][ImgCSGIntersection]{{rightimage(100)}}
![][ImgCSGUnion]{{rightimage(100)}}
![][ImgCSGOperands]{{rightimage(100)}}

These images show the original operand models, a box and a cylinder, and the
resulting CSG union, intersection, and difference (cylinder subtracted from the
box).

Note that in some cases converting a model to a [Clipped
model][ConvertedModels] and applying the [Clip Tool][ClipTool] may turn out to
be a simpler method of geometric editing.

### Convex Hull

![][ImgHullCapsule]{{rightimage(100)}}
![][ImgHullSpheres]{{rightimage(100)}}

Another geometric operation is the {{term('convex hull')}} operation, which
creates a new model from the [3D convex
hull](https://en.wikipedia.org/wiki/Convex_hull) of one or more selected
models. This can be used to easily create different shapes. For example, here
is an easy way to create a capsule by applying the convex hull operation to two
spheres.

![][ImgHullDisk]{{rightimage(100)}}
![][ImgHullTorus]{{rightimage(100)}}

The hull operation can sometimes be useful to apply to a single model, if the
model is not already convex. Here's an example of creating a rounded disk as
the convex hull of a torus.

### Some Patience Required

MakerVR uses the [CGAL library](https://www.cgal.org/) to compute the
combinations of objects. Even though this library is very powerful, robust, and
well-optimized, it can still take some time to compute the results in complex
cases. This work is performed in a separate thread so that the application is
not frozen while it happens. While the computation is active, the combined
model may be displayed as a 3D hourglass and the mouse cursor (visible when not
in VR mode) will also be an hourglass.

## Model Types

Models can be divided into three basic categories:

+ {{term('Primitive models')}}, which are created from scratch or imported from
  STL files.
+ {{term('Converted models')}}, which are created by converting other types of
  models in order to apply a specialized tool. For example, if you want to
  create beveled edges on a Box model, you first convert it to a Beveled model,
  then use the [Bevel Tool][BevelTool] to change the bevel if necessary.
+ {{term('Combined models')}}, which are created by combining other models in
  different ways. The result may or may not have a specialized tool associated
  with it.

### Primitive Models

+ A {{term('Box model')}} represents a 3D box with the length of all sides
  equal to 8 units by default. There is no specialized tool for a Box.
+ A {{term('Cylinder model')}} represents a closed cylinder that by default has
  a diameter and height of 8 units. Its axis of symmetry is aligned with the +Z
  ("up") axis. The [Cylinder Tool][CylinderTool] is the specialized tool for a
  Cylinder; it allows you to change the top and bottom diameters independently.
+ An {{term('Imported model')}}, as its name suggests, is imported from an STL
  file. The specialized [Import Tool][ImportTool] is used to select the file to
  import from and also to reimport the model if any changes were made to the
  STL file.
+ A {{term('RevSurf model')}} represents a closed surface of revolution,
  created by rotating a 2D profile about the Z axis. The default profile
  consists of 3 points (the minimum allowed) that forms a surface with the widest
  diameter of 8 units and a height of 8 units. The specialized [Surface of
  Revolution Tool][RevSurfTool] lets you edit the profile and also the sweep
  angle of the surface.
+ A {{term('Torus model')}} represents a torus that by default has an outer
  diameter of 8 units and an inner diameter of 1.6 units. Its axis of symmetry
  is aligned with the +Z ("up") axis. The [Torus Tool][TorusTool] is the
  specialized tool for a Torus; it allows you to change the outer and inner
  diameters independently.
+ A {{term('Sphere model')}} represents a sphere that has a diameter of 8 units
  by default. There is no specialized tool for a Sphere.
+ A {{term('Text model')}} represents extruded 3D text, which is laid out by
  default at the origin along the +X axis, extruded 2 units along the +Z (up)
  direction. It is sized by default so that the characters are approximately 4
  units in the Y dimension.  The specialized [Text Tool][TextTool] allows you
  to change the text string, font, and character spacing.

### Converted Models

Each of these model types is created by [selecting][Selection] one or more
models and clicking on the appropriate 3D icon to apply the action to convert
them. Once they are converted, you can use the corresponding specialized tool
to edit them, as listed below.

Note that this two-step process (converting and editing) makes it clear that a
new type of model must be created in order to apply the editing operation. The
converted model becomes a parent of the original model in the hierarchy (as
shown in the [Tree Panel][TreePanel]).

+ A {{term('Beveled model')}} applies a bevel or other profile to edges of a
  model. The [Convert-to-Bevel action][ConversionShelf] is used to convert the
  models and the [Bevel Tool][BevelTool] is used to modify the bevel profile.
+ A {{term('Clipped model')}} clips a model with one or more clipping
  planes. The [Convert-to-Clipped action][ConversionShelf] is used to convert
  the models and the [Clip Tool][ClipTool] is used to specify the plane(s).
+ A {{term('Mirrored model')}} mirrors a model about one or more principal
  planes. The [Convert-to-Mirrored action][ConversionShelf] is used to convert
  the models and the [Mirror Tool][MirrorTool] is used to specify how to
  mirror.

### Combined Models

A combined model is created by [selecting][Selection] the appropriate number of
models and then clicking on the appropriate action icon.

+ A {{term('CSG model')}} is created as a [constructive solid geometry
  (CSG)][CSG] combination of two or more models. The operation is specified by
  clicking on the [CSG Difference action][CSGDifferenceAction], [CSG
  Intersection action][CSGIntersectionAction], or [CSG Union
  action][CSGUnionAction] icons.  The operation may be changed afterwards with
  the [CSG Tool][CSGTool], which is the specialized tool for a CSG model.
+ A {{term('Hull model')}} is created by clicking on the [Convex Hull
  action][ConvexHullAction] icon. There is no specialized tool for a Hull model.

## Model Names

When MakerVR creates a model of any type, a unique name is assigned to it
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

