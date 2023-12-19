A Simple Doorstop Example (Non-VR)
----------------------------------

This example will use |appname| in non-VR mode to create a simple doorstop
model and output it as an STL file.

.. incvideo:: Doorstop ../videos/Doorstop.mp4 600px center
   :poster:  /images/DoorstopFinished.jpg

|block-image|

Start the Application
.....................

.. videobutton:: Doorstop 0 Watch

.. incimage:: /images/DoorstopSessionPanel.jpg 240px right

Starting the application should open a maximized window that you can interact
with using the keyboard and mouse. The first thing you should see is :ref:`this
panel that lets you interact with sessions <ug-session-panel>`. Click on the
`Start new session` button to start a new session (or just hit the
:shortcut:`Escape` key).

|block-image|

Create and Position a Torus
...........................

.. incimage:: /images/CreateTorusIcon.jpg 300px right

Click on the :ref:`Create Torus icon <ug-create-torus>` icon (3D torus icon on
the bottom-left shelf) to drop a new torus model into the scene. The torus will
have some interactive handles attached to it that you can use to change the
sizes of the outer and inner diameters. You can leave them at their default
sizes for this example. If you change them and want to undo your changes, you
can click on the 3D undo icon on the top shelf on the back wall or use
:shortcut:`Ctrl-Z`.

|block-image|

.. incimage:: /images/TranslationToolIcon.jpg 300px right

Next you will translate (move) the torus a little to the right using the
:ref:`Translation Tool <ug-translation-tool>`. There are several ways to switch
to the Translation Tool from the current state:

 - Click on the :ref:`Translation Tool icon <ug-scale-tool>` from the General
   Tools shelf (see image).
 - Click on the :ref:`Toggle Specialized Tool icon
   <ug-toggle-specialized-tool>` on the top left shelf. This switches from the
   specialized torus tool to the current general tool, which is the Translation
   Tool by default.
 - Press :shortcut:`Space`, which also switches between the current specialized
   tool and the current general tool.

.. incimage:: /images/DoorstopTorus.jpg 220px right

Once the Translation Tool is attached, drag the translation handle on the left
or right side of the torus to the right until it has moved 5 units. You will
notice that as you drag the torus, feedback appears to show you the direction
and distance you have moved, as shown here.

|block-image|

Create a Cylinder and Change Its Size
.....................................

.. incimage:: /images/CreateCylinderIcon.jpg 300px right

Click on the :ref:`Create Cylinder icon <ug-create-cylinder>` icon (3D cylinder
on the bottom-left shelf) to add a cylinder. Like the torus, the cylinder has
specialized handles for changing the top and bottom diameters, but you won't
need them right now.

|block-image|

.. incimage:: /images/DoorstopScale.jpg 240px right
.. incimage:: /images/ScaleToolIcon.jpg 300px right

Next, select the :ref:`Scale Tool <ug-scale-tool>` from the General Tools
shelf. This will attach a bunch of scaling widgets to the cylinder as shown
here.

|block-image|

.. incimage:: /images/DoorstopCylinder.jpg 240px right

Drag the top (blue) scaler down until the height of the cylinder is 3 units.

|block-image|

Combine the two Models to Create the Doorstop
.............................................

.. incimage:: /images/DoorstopSelected.jpg 240px right

The next step is to select both models. Since the cylinder is already selected,
you just need to add the torus to the selection. You can do that in any of
these ways:

 - Shift-click on the torus model.
 - Double-click on the torus model
 - Shift-click or double-click on the name of the torus (:model:`Torus_1`) in
   the :ref:`Tree Panel <ug-tree-panel>` on the back wall.
 - Drag a rectangle in the Tree Panel that includes both model names.
 - Use the :shortcut:`Ctrl-A` select-all shortcut.

|block-image|

.. incimage:: /images/DoorstopHull.jpg    240px right
.. incimage:: /images/CombineHullIcon.jpg 300px right

With both models selected, click on the :ref:`Combine Hull <ug-combine-hull>`
icon on the Model Combining shelf. This creates a new model whose surface is a
convex hull surrounding both selected models. You can see in the Tree Panel
that the new model is named :model:`Hull_1` and that the two models it was
created from appear as indented children (hidden in the scene).

|block-image|

Change the Model Name
.....................

.. incimage:: /images/DoorstopRenamed.jpg 240px right
.. incimage:: /images/NameToolIcon.jpg    300px right

This is not a necessary step, but it will make the next step (exporting)
easier. With the hull model selected, click on the :ref:`Name Tool
<ug-name-tool>` icon from the General Tools shelf. This will bring up a 2D
panel for editing the model's name.

Click in the text input area to activate it, and change the name of the model
to :model:`Doorstop`. Click the "Apply" button to change the model name; the
new name should be shown in the Tree Panel.

|block-image|

Export the Model for 3D Printing
................................

.. incimage:: /images/DoorstopExport.jpg   300px right
.. incimage:: /images/SessionPanelIcon.jpg 300px right

Exporting is done with the :ref:`Session Panel <ug-session-panel>` that you saw
when you started the application. Bring the panel up by clicking on the Session
Panel icon on the Session shelf (you may need to move the Name Panel out of the
way or switch to a different tool) or with the :shortcut:`Ctrl-s`
shortcut. Click on the "Export selection" button, which will open a :ref:`File
Panel <ug-file-panel>` that will let you select a file to export to, as shown
here.

This panel is a fairly standard file browser. The default location for exported
files is your home directory; this can be changed in the :ref:`Settings Panel
<ug-settings-panel>`. The default name for the file will be the same as the
name of the model, which is why renaming the model made sense.

A model can be exported as either text or binary STL using the dropdown at the
top right of the panel. The resulting STL file can be imported into almost any
3D printing application.

|block-image|
