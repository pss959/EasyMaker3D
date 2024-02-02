Chess Rook Example
------------------

This tutorial example uses |appname| to create a model of a rook, using some
more advanced features. It assumes you are familiar with the basics of the
application from the :doc:`Doorstop` example and other tutorials.

.. incvideo:: Rook

|block-image|

Create the Main RevSurf Shape
.............................

.. incimage:: /images/RookRevSurf.jpg       200px right
.. incimage:: /images/CreateRevSurfIcon.jpg 300px right

:vidref:`Rook:RevSurf` The main part of the rook is created as a RevSurf
(surface of revolution) model. Click on the :ref:`Create RevSurf icon
<ug-create-rev-surf>` to drop a RevSurf model into the scene. The specialized
:ref:`Rev Surf Tool <ug-rev-surf-tool>` will be attached to the new model.

Create and drag points to approximate this rook profile. (See the :doc:`Profile
editing tutorial <Profile>` if necessary.)

|block-image|

Hollow Out the Top
..................

.. incimage:: /images/RookTarget.jpg -200px right

:vidref:`Rook:Target` The next step is to create a circular hollow in the top
of the rook using a cylinder. The cylinder must be placed precisely in the
center of the top of the rook, so the :ref:`Point Target <ug-targets>` will come
in very handy.

The Point Target can be dragged onto the surface of any model. With
:ref:`modified-dragging <ug-modified-mode>`, it can be dragged onto the 3D
bounding box of any model, snapping to the edges and center of the faces of the
bounds. This feature is used to place the target at the center of the top of
the rook RevSurf as shown here.

:ref:`Changing the view <ug-viewing>` will make it a lot easier to see the top
of the RevSurf. You can drag the height slider on the pole up until the top is
visible and use the right mouse button to rotate the view to look down on the
RevSurf.

Then, turn on the Point Target and modified-drag it onto the top center point
of the RevSurf. The snap indicator sphere on the target will turn white when
the target is snapped to the center. Note that you can hide models temporarily
with the :ref:`Tree Panel <ug-tree-panel>` if you cannot see the Point Target
to drag it.

|block-image|

.. incimage:: /images/RookScaledHollower.jpg   -200px right
.. incimage:: /images/RookUnscaledHollower.jpg -200px right

:vidref:`Rook:Cylinder` With the Point Target placed, click on the :ref:`Create
Cylinder icon <ug-create-cylinder>` to drop a cylinder model into the scene.
Note that it will be positioned with its bottom center on the target position
and align its "up" direction with the target arrow as shown on the left here.

Select the :ref:`Scale Tool <ug-scale-tool>` and use modified-dragging (for
symmetric scaling) on any cylindrical handle that changes the width and depth
of the cylinder simultaneously. Scale it down to a 1 unit diameter as shown
on the right here.

|block-image|

.. incimage:: /images/RookPositionedHollower.jpg -200px right

The next step is to translate the cylinder down so it projects .5 units into
the RevSurf. Since the default :ref:`precision level <ug-precision-level>` is
1-unit increments for translation, you have to increase the precision level
first. You can do this by clicking on the top button on the :ref:`Precision
Control <ug-precision-control>` on the back wall or with the :shortcut:`x`
keyboard shortcut.

Then, attach the :ref:`Translation Tool <ug-translation-tool>` to the cylinder
and drag it down until the feedback shows .5 units. The result should look like
this image.

|block-image|

.. incimage:: /images/RookHollowed.jpg             -200px right
.. incimage:: /images/CombineCSGDifferenceIcon.jpg  300px right

:vidref:`Rook:Hollow` With the cylinder positioned, it can be subtracted from
the RevSurf to create the hollow. Select the RevSurf first and the cylinder
second (using modified-clicking). Then click on the :ref:`Combine CSG
Difference icon <ug-combine-csg-difference>` to perform the subtraction.
The result should look like the right image.

|block-image|

Cut the Slots
.............

.. incimage:: /images/RookSlotCutters.jpg  -200px right
.. incimage:: /images/RookScaledCutter.jpg -200px right

The last step is to cut slots in the top of the rook using thin boxes. Hide the
rook body using the :ref:`Tree Panel <ug-tree-panel>`. Create a box by clicking
on the :ref:`Create Box icon <ug-create-box>`. Use the :ref:`Scale Tool
<ug-scale-tool>` to scale the width down to .2 units. Again, note that you can
adjust the precision level to 1 to scale down to 1 unit and then to .1 to
finish scaling to .2 units. The result is shown in the left image.

:ref:`Copy <ug-copy>` the box and :ref:`paste <ug-paste>` it. The copy will
then be selected. Use the :ref:`Rotation Tool <ug-rotation-tool>` to rotate the
copy 90 degrees around the up axis. The result is shown in the right image.

|block-image|

XXXX MORE TBD
.............

.. incimage:: /images/Rook.jpg 200px right
.. incimage:: /images/RookWithSlotCutters.jpg 200px right
