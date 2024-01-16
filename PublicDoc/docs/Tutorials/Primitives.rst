Creating and Editing Primitive Models
-------------------------------------

This tutorial shows how to create and edit each of the primitive model types.
Most of the primitive model types have specialized editing features that are
provided by a :ref:`specialized tool <ug-specialized-tools>`. The associated
specialized tool is attached to a new model when it is created.

The different types of models and the specialized tools used to edit them are
summarized here:

..
   _Note: Adding the noheader class and adjusting CSS is the only way I found
   to have a headerless table in RST.

.. container:: twocolumn

    .. rst-class:: noheadertable

    ======== ============================= =======================================
    Box      :vidref:`Primitives:Icons`    <None>
    Sphere   :vidref:`Primitives:Icons`    <None>
    Cylinder :vidref:`Primitives:Cylinder` :ugref:`ug-cylinder-tool` Cylinder Tool
    Torus    :vidref:`Primitives:Torus`    :ugref:`ug-torus-tool`    Torus Tool
    ======== ============================= =======================================

    .. rst-class:: noheadertable

    ======== ============================= =======================================
    RevSurf  :vidref:`Primitives:RevSurf`  :ugref:`ug-rev-surf-tool` RevSurf Tool
    Extruded :vidref:`Primitives:Extruded` :ugref:`ug-extruded-tool` Extruded Tool
    Text     :vidref:`Primitives:Text`     :ugref:`ug-text-tool`     Text Tool
    Imported :vidref:`Primitives:Imported` :ugref:`ug-import-tool`   Import Tool
    ======== ============================= =======================================

.. incvideo:: Primitives ../videos/Primitives/Primitives.mp4 80% center

Box and Sphere Models
.....................

The Box and Sphere models have no specialized editing features and therefore do
not have specialized tools. When either of these models is created, the current
:ref:`general tool <ug-general-tools>` is attached.

Cylinder Model
..............

A Cylinder model has a specialized tool that allows you to change the top and
bottom diameters independently by dragging handles. This means you can easily
turn the cylinder into a cone or truncated cone.

Torus Model
...........

A Torus model also has a specialized tool that allows you to change the inner
and outer diameters independently by dragging handles. Note that if the inner
diameter is increased to a point where it would cause the hole to disappear,
the outer radius is automatically increased to prevent this.

RevSurf Model
.............

A RevSurf (surface of revolution) model is created by rotating a 2D profile
around the Z (up) axis. The specialized tool allows you to edit the profile and
to change the sweep (rotation) angle to be less than 360 degrees.

Extruded Model
..............

An Extruded model is formed by extruding a closed 2D profile along the Z (up)
axis. The specialized tool allows you to edit the profile. There is an option
to set the profile to a regular polygon with a customizable number of sides;
you can then modify the points of this polygon like any other profile.

Text Model
..........

A Text model extrudes one or more text characters along the Z (up) axis to
create 3D text. The specialized tool allows you to edit the text string, the
font, and the spacing between characters.

Imported Model
..............

An Imported model is created by importing a 3D model in STL format from a file.
The specialized tool is essentially a file browser that lets you choose or
change the file to import the model from.
