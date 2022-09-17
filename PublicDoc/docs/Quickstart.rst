|appname| Quickstart
====================

Installing and Running |appname|
--------------------------------

Open the `the download site <download_site_>`_ (on Google Drive) in a browser
and download the Zip file for your operating system. Extract all files wherever
you like; this will create a |appname| folder. In this folder there is a
|appname| executable that you can click on to run the application.


A Simple Doorstop Example (Non-VR)
----------------------------------

.. incimage:: /images/Doorstop.jpg 240px right

This example will use |appname| in non-VR mode to create a simple doorstop
model and output it as an STL file.

:dnote:`Clicking on any of these images will show an enlarged version.`

|block-image|

Start the Application
.....................

.. incimage:: /images/SessionPanel.jpg 240px right

Starting the application should open a maximized window that you can interact
with using the keyboard and mouse. The first thing you should see is :ref:`this
panel that lets you interact with sessions <ug-session-panel>`. Click on the
`Start new session` button to start a new session (or just hit the
:shortcut:`Escape` key).

|block-image|

Create and Position a Torus
...........................

.. incimage:: /images/CreateTorusIcon.jpg 300px right

Click on the 3D torus icon on the bottom-left shelf to drop a new torus model
into the scene. The torus will have some interactive handles attached to it
that you can use to change the sizes of the outer and inner diameters. You can
leave them at their default sizes for this example. If you change them and want
to undo your changes, you can click on the 3D undo icon on the top shelf on the
back wall or use :shortcut:`Ctrl-Z`.

|block-image|

.. incimage:: /images/QuickstartTorus.jpg 220px right

Next you will move the torus a little to the right.  Hit :shortcut:`Space` to
switch from the specialized torus tool to the general translation tool.

Drag the handle on the right side of the torus to the right until it has moved
5 units. You will notice that as you drag the torus, feedback appears to show
you the direction and distance you have moved, as shown here.

|block-image|

Create a Cylinder and Change Its Size
.....................................

.. incimage:: /images/CreateCylinderIcon.jpg 300px right

Click on the 3D cylinder icon on the bottom-left shelf to add a cylinder. Like
the torus, the cylinder has specialized handles for changing the top and bottom
diameters, but you won't need them right now.

|block-image|

.. incimage:: /images/QuickstartScale.jpg 240px right
.. incimage:: /images/ScaleToolIcon.jpg   300px right

Next, select the :ref:`Scale Tool <ug-scale-tool>` from the General Tools
shelf. This will attach a bunch of scaling widgets to the cylinder as shown
here.

|block-image|

.. incimage:: /images/QuickstartCylinder.jpg 240px right

Drag the top (green) scaler down until the height of the cylinder is 3 units.

|block-image|

Combine the two Models to Create the Doorstop
.............................................

.. incimage:: /images/QuickstartSelected.jpg 240px right

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

.. incimage:: /images/QuickstartHull.jpg  240px right
.. incimage:: /images/CombineHullIcon.jpg 300px right

With both models selected, click on the :ref:`Combine Hull <ug-combine-hull>`
icon on the Model Combining shelf. This creates a new model whose surface is a
convex hull surrounding both selected models. You can see in the Tree Panel
that the new model is named :model:`Hull_1` and that the two models it was
created from appear as indented children.

|block-image|

Change the Model Name
.....................

.. incimage:: /images/QuickstartRenamed.jpg 240px right
.. incimage:: /images/NameToolIcon.jpg      300px right

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

.. incimage:: /images/QuickstartExport.jpg 300px right
.. incimage:: /images/SessionPanelIcon.jpg 300px right

Exporting is done with the :ref:`Session Panel <ug-session-panel>` that you saw
when you started the application. Bring the panel up by clicking on the Session
Panel icon on the Session shelf. Click on the "Export selection" button, which
will open a :ref:`File Panel <ug-file-panel>` that will let you select a file
to export to, as shown here.

This panel is a fairly standard file browser. The default location for exported
files is your home directory; this can be changed in the :ref:`Settings Panel
<ug-settings-panel>`. The default name for the file will be the same as the
name of the model, which is why renaming the model made sense.

A model can be exported as either text or binary STL using the dropdown at the
top right of the panel.

|block-image|

.. _qs-vr:

Setting up VR
-------------

Feel free to skip these section if you do not have a VR headset or you just
don't want to use it.

Some VR experiences allow you or force you to walk around while using them.
That wouldn't make sense for this application, so it is designed to make
everything useful from one spot. If you're planning to use the VR mode, set it
up for a comfortable, fixed standing or seated position. Note that the scene
view is set up for sitting, so you may need to adjust your VR positioning for a
different height if you prefer to stand.

|appname| was developed using an HTC Vive and Oculus Quest 2 (since that's what
I have), but should work with any standard VR controllers supported by `SteamVR
<https://www.steamvr.com>`_. If not, `contact the author
<https://github.com/pss959>`_ for help.

.. todo::
   TBD finish this

The Doorstop Example in VR
--------------------------

.. todo::
   TBD with images
