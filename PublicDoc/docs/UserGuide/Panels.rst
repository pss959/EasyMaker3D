.. _ug-panels:

Panels
------

As mentioned in the :ref:`Overview <ug-panel-overview>`, |appname| uses some
more conventional 2D user interface panels. Because these panels need to work
in :ref:`VR as well as in conventional mode <ug-vr-modes>`, they appear in the
3D :ref:`work area <ug-work-area>`. Panels are always aligned with the
XZ-plane.

Panels are divided into three categories:

  - :newterm:`Application panels` implement various application functions, such
    as loading sessions or modifying program settings.  Application panels
    appear initially in a central location.
  - :newterm:`Tool panels` are used for :ref:`certain specialized tools
    <ug-specialized-tools>`. A tool panel appears initially above the
    :ref:`primary selection <ug-selection>` it is operating on.
  - The :newterm:`Tree Panel` is special; it is always fixed to the back wall
    and has :ref:`its own section in this guide <ug-tree-panel>`.

Moving and Resizing Panels
..........................

.. incimage:: /images/SessionPanel.jpg 200px right

Panels are placed initially to be reasonably easy to interact with.  When
application panels are visible, they temporarily hide the scene contents so
there is no interference. (Tool panels do not do this because you may need to
see the results of editing models with them.)

You can move any application or tool panel if it is in an inconvenient place.
All panels have interactive handles on the center of each edge that can be
dragged to move the panel to the left, right, up, or down. There is also a bar
at the bottom that can be dragged to move the panel to the left, right,
forward, or back. Each application and tool panel remembers its last position.

|block-image|

.. incimage:: /images/SettingsPanel.jpg 200px right

Some panels (such as the :ref:`Settings Panel <ug-settings-panel>` shown here)
also have interactive spherical handles in the four corners that allow you to
resize the panel. All panels have a minimum size and will not let you resize
them smaller than that.

.. admonition:: VR Only

   .. incimage:: /images/GripPanelBar.jpg 200px right

   All interactive panel handles respond to :ref:`grip-dragging
   <ug-grip-dragging>` as well. The relative orientation of the grip hover
   guide on the controller determines which handle will be used. For example,
   if the guide for the right controller is close to parallel to the X axis, it
   will highlight the move handle on the right side so you can grip-drag it.
   If the panel has scale handles and the guide is close to the diagonal
   direction, it will highlight the corresponding scale handle.

   You can interact with the contents of a panel (if it supports it) by
   pointing the grip hover guide towards the board. If the controller is low
   enough (relative to the panel), it will allow you to grip drag the bottom
   bar, as shown here.

.. _ug-panel-interaction:

Interacting with Panel Elements
...............................

Each panel consists of interactive :newterm:`elements` (buttons, dropdowns,
etc.) that work pretty much the same as in conventional 2D user interfaces.

  - Disabled elements are dark gray.
  - All enabled elements highlight when hovered with the mouse or laser
    pointer.
  - Clicking on an element with the mouse or :ref:`pinch action <ug-pinch>`
    activates it.
  - Keyboard navigation between enabled elements uses the :shortcut:`Tab` and
    :shortcut:`Shift-Tab` keys. The element with the current keyboard focus is
    shown with a cyan border outline.
  - Pressing :shortcut:`Enter` on the keyboard activates the focused element.
  - Pressing the :shortcut:`Escape` key in any application panel always cancels
    whatever the current panel is doing and dismisses it.

Some panels contain scrolling areas when their contents are too large to
fit. You can scroll a panel using a scroll wheel, the scrollbar on the right
(drag with mouse or pinch), or the :shortcut:`Up` and :shortcut:`Down` arrow
keys (or Up/Down trackpad/joystick buttons on a VR controller). The same is
true for long dropdown lists.

.. admonition:: VR Only

   :ref:`Panel touch mode <ug-touch-mode>` is active when in either of the
   :ref:`VR modes <ug-vr-modes>`. In touch mode, a touch affordance is added to
   each controller model that acts as a virtual finger. Touching the tip of
   this affordance to a button, slider, or other element works as you would
   expect. The controller will vibrate briefly when this happens for feedback.

   Most interactive elements also support clicking with the grip button. When
   the :ref:`hover guide <ug-grip-dragging>` is close enough to perpendicular
   to the panel, it will highlight the element that it will interact with.

.. _ug-application-panels:

Application Panels
..................

.. _ug-export:
.. _ug-session-panel:

Session Panel
,,,,,,,,,,,,,

.. incimage:: /images/SessionPanel.jpg 240px right

The :newterm:`Session Panel` is shown when the application starts and can also
be brought up at any time with the :ref:`Open Session Panel
<ug-open-session-panel>` action. This panel is used to start, save, and load
:ref:`sessions <ug-sessions>`. Sessions are saved as text files with an
|session-extension| extension.

The Session Panel has the following buttons:

  - ``Continue previous/current session``. When the application starts, this
    choice will be enabled if your previous session was saved with a name,
    which will appear in brackets in the button text. Click this to resume
    working in that session. If the panel is invoked during a session, clicking
    this button will act as if nothing happened. If there was no previous
    session when the application starts, this button will say ``No previous
    session`` and will be disabled as in the image.
  - ``Load session file``. This can be used to load a different session file
    using the :ref:`File Panel <ug-file-panel>`; you will be asked if you made
    any changes to the current session that you might want to save.
  - ``Start new session``. This resets everything to empty and starts a new,
    unnamed session. If you do this during a session, you will be asked if you
    made any changes that you might want to save.
  - ``Save session``. If your session is already named, this will be enabled to
    save it again if you made any changes.
  - ``Save session as``. This lets you save your session with a different name
    using the :ref:`File Panel <ug-file-panel>`.
  - ``Export Selection``. This lets you export the :ref:`selected model(s)
    <ug-selection>` to a file, using a :ref:`File Panel <ug-file-panel>`. The
    File Panel has a dropdown that allows you to select the desired format
    (text or binary STL).
  - ``Help`` (Question mark icon at top left). Opens the :ref:`Help Panel
    <ug-help-panel>`. Dismissing the Help Panel returns to the Session Panel.
  - ``Settings`` (Gear icon at top right). Brings up the :ref:`Settings Panel
    <ug-settings-panel>`. Dismissing the Settings Panel returns to the Session
    Panel.

The default directories in which to find or save session files and STL files
are both stored as user settings and can be modified with the :ref:`Settings
Panel <ug-settings-panel>`.

.. _ug-settings-panel:
.. _ug-unit-conversion:

Settings Panel
,,,,,,,,,,,,,,

.. incimage:: /images/SettingsPanel.jpg 300px right

The :newterm:`Settings Panel` lets you modify application settings that are
saved between sessions. The location of the saved settings is
``$HOME/.config/{appname}`` on Linux and Mac, and ``%APPDATA%\{appname}`` on
Windows.

Each item in this panel has two buttons to the right:

 - The ``Default`` button resets the item to its default value.
 - The ``Current`` button sets the item to its current (saved) value. You can
   use this if you accidentally change a value and want to undo that without
   having to cancel all other changes.

The ``Accept`` button at the bottom of the panel will be enabled if you make
any changes to the current settings. Clicking it applies the new settings and
saves them to the settings file. The ``Cancel`` button leaves all settings at
their previous values. Either button makes the panel go away. The
:shortcut:`Escape` key also cancels the panel.

The items in this panel are, from top down:

 - :newterm:`Default Directories`: The first three settings are the default
   directories for saving and loading sessions, STL model export, and STL model
   import. Each of these has a :ref:`text input field <ug-text-input>` for the
   directory name and a ``Browse`` button that opens a :ref:`File Panel
   <ug-file-panel>` to choose a directory for that item.
 - :newterm:`Tooltip Delay`: This is a horizontal slider you can use to adjust
   the time it takes for :ref:`tooltips <ug-tooltips>` to appear when hovering
   the mouse or laser pointer over an interactive object.  Values are in
   seconds, from 0 at the left to 10 at the right. Setting this to 0 disables
   tooltips completely.
 - :newterm:`Build Volume Size`: This consists of three :ref:`text input fields
   <ug-text-input>` for the width, depth, and height of the :ref:`build volume
   <ug-build-volume>`, specified in the current :ref:`units <ug-units>`. You
   can set these to match the approximate size of your 3D printer's build area
   for reference.
 - :newterm:`Unit Conversion`: There are two pairs of dropdowns allowing
   conversion of units on :ref:`model import <ug-primitive-models>` and
   :ref:`model export <ug-session-panel>`. The use of these settings are
   explained in more detail in the :ref:`Overview <ug-units>`.
 - :newterm:`Radial Menus`: The last button brings up the :ref:`Radial Menu
   Panel <ug-radial-menu-panel>`, which allows you to edit which buttons appear
   in :ref:`radial menus <ug-radial-menus>`.

.. _ug-info-panel:

Info Panel
,,,,,,,,,,

.. incimage:: /images/InfoPanel.jpg 240px right

The :newterm:`Info Panel` shows information about all selected models,
including the number of vertices, number of triangles, and size in all three
dimensions. If any model has an :ref:`invalid mesh <ug-model-colors>`, this is
noted, along with the reason it is considered invalid.

If either the :ref:`Point or Edge Target <ug-targets>` is active, the relevant
information about it will also be shown in the Info Panel.

The Info Panel is resizable using the corner handles. If the text does not all
fit vertically in the info area, the thumb in the scroll bar will appear to let
you scroll up and down.

The ``Done`` button or the :shortcut:`Escape` key closes the Info Panel.

|block-image|

.. _ug-help-panel:

Help Panel
,,,,,,,,,,

.. incimage:: /images/HelpPanel.jpg 240px right

The :newterm:`Help Panel` shows text with the current application version along
with two buttons that open this :ref:`User Guide <ug>` and the :ref:`Cheat
Sheet <cheat-sheet>` in your default browser.

The ``Done`` button or the :shortcut:`Escape` key closes the Info Panel.

|block-image|

.. _ug-radial-menu-panel:

Radial Menu Panel
,,,,,,,,,,,,,,,,,

.. incimage:: /images/RadialMenuPanel.jpg 300px right

The :newterm:`Radial Menu Panel` can be opened from the :ref:`Settings Panel
<ug-settings-panel>` to let you edit which buttons appear in the :ref:`left and
right radial menus <ug-radial-menus>`. The radio buttons in the box at the top
let you choose one of the following options:

  - ``Use Independent Settings``: Editing areas for both left and right menus
    appear to let you edit buttons for each side independently as shown in the
    image.
  - ``Use Left Settings for Both``: Only the area for the left menu is shown;
    the buttons defined in there are used for both menus.
  - ``Use Right Settings for Both``: Only the area for the right menu is shown;
    the buttons defined in there are used for both menus.
  - ``Disable Radial Menus``: Both areas are hidden and radial menus are not
    available.

You can set the number of buttons to use in a radial menu to 2, 4, or 8. The
menu diagram updates to reflect the current number. Clicking on any button in
the diagram brings up the :ref:`Action Panel <ug-action-panel>` that lets you
choose the :ref:`action <ug-actions>` attached to that button. Each type of
action is represented by a horribly-designed 2D icon that is displayed in the
radial menu diagram and in the actual radial menu as well. The |noneicon| null
icon is shown when no action is bound to a button. All actions and icons are
shown in the :ref:`Cheat Sheet <cheat-sheet>`.

.. _ug-action-panel:

Action Panel
,,,,,,,,,,,,

.. incimage:: /images/ActionPanel.jpg 300px right

The :newterm:`Action Panel` is used by the :ref:`Radial Menu Panel
<ug-radial-menu-panel>` to select the :ref:`action <ug-actions>` attached to a
radial menu button. It presents a scrolling list of actions to choose from,
organized by category. The action that is currently bound to the button being
modified is highlighted.

See the :ref:`Cheat Sheet <cheat-sheet>` for a list of all actions and their
associated icons.

|block-image|

.. _ug-file-panel:

File Panel
,,,,,,,,,,

.. incimage:: /images/FilePanel.jpg 300px right

The :newterm:`File Panel` is used by the :ref:`Session Panel
<ug-session-panel>` and the :ref:`Settings Panel <ug-settings-panel>` when a
directory or file needs to be chosen. In addition, the :ref:`Import Tool Panel
<ug-import-tool-panel>` is essentially a File Panel.

This panel works pretty much like a standard file browser. The four buttons at
the top go to the previous directory (if any), the next directory (if you went
to the previous one), one directory up, or your home directory. A button at the
bottom lets you see (operating-system-specific) hidden files and directories.

The scrolling list is color coded for directories and files, with directories
listed first.

When using a File Panel for :ref:`exporting models <ug-session-panel>`, a File
Format dropdown appears at the top right for selecting a format.

|block-image|

.. _ug-dialog-panel:

Dialog Panel
,,,,,,,,,,,,

.. incimage:: /images/DialogMessagePanel.jpg  300px right
.. incimage:: /images/DialogQuestionPanel.jpg 300px right

A :newterm:`Dialog Panel` is used by other application panels to ask you a
question or inform you of some other condition. A question dialog will have
buttons with answer choices. A message dialog will just have one button to
dismiss it.

|block-image|

.. _ug-virtual-keyboard-panel:

Virtual Keyboard Panel
,,,,,,,,,,,,,,,,,,,,,,

.. admonition:: VR Only

   .. incimage:: /images/VirtualKeyboardPanel.jpg 300px right

   The :newterm:`Virtual Keyboard Panel` is used only in :ref:`full VR mode
   <ug-vr-modes>` when you are wearing the VR headset and you need to edit text
   in a panel. It is most easily used by touching the keys with the :ref:`touch
   affordance <ug-touch-mode>`. (You can also use the laser pointer, but it is
   more awkward.)

.. _ug-tool-panels:

Tool Panels
...........

.. _ug-bevel-tool-panel:

Bevel Tool Panel
,,,,,,,,,,,,,,,,

The :newterm:`Bevel Tool Panel` lets you edit the bevel created for all Beveled
models once they have been :ref:`converted from other models
<ug-converted-models>`. The bevel can be any sort of :newterm:`profile` applied
to model edges to create various effects such as chamfering or rounding.

All interactive changes made to a profile can be undone and redone
individually.

Profile Area
::::::::::::

.. incimage:: /images/BevelToolPanel.jpg 300px right

The large area in the panel shows the current profile of the :ref:`primary
selection <ug-selection>` and lets you edit it. If you make changes, they are
applied to all selected Beveled models. The models update in real time as the
profile is edited.

The profile is interpreted as follows:

  - The lower-left corner is where the edge is located, looking along its
    length.
  - The upper-left and lower-right points of the profile are at fixed locations
    and are colored blue to indicate this. You can add new points between them,
    move those points around, and delete points.

The default profile is just a line connecting the two fixed points, which
creates a bevel for all edges as in this image.

|block-image|

Adding, Moving, and Deleting Points
:::::::::::::::::::::::::::::::::::

.. incimage:: /images/BevelToolPanelDelete.jpg 200px right
.. incimage:: /images/BevelToolPanelHover.jpg  200px right

If you move the mouse close to an existing profile line (but not near an
existing point), a red square will appear to indicate that a new interior
profile point can be created there as shown in the left image. Clicking in that
spot will create a new point. Click-dragging lets you move the new point where
you want.

Dragging an existing interior point moves it; when you do this, a box with an
"X" in it appears to allow you to delete the point by dragging it over that
box, as shown in the right image.

|block-image|

Profile Scaling
:::::::::::::::

.. incimage:: /images/BevelToolPanelScaleLarge.jpg 200px right
.. incimage:: /images/BevelToolPanelScaleSmall.jpg 200px right

The slider at the bottom of the panel allows you to change the size of the
profile as it is applied to edges without having to move any points. For
example, you can create a rounding profile and change the radius by adjusting
the slider as shown here.

|block-image|

Adjusting the Maximum Edge Angle
::::::::::::::::::::::::::::::::

.. incimage:: /images/BevelToolPanelMaxAngle.jpg 200px right

The slider on the right side of the panel allows you to change the
:newterm:`maximum edge angle`, from 0 to 180 degrees. This setting determines
which edges of a model will have the bevel profile applied to them.

For example, suppose you want to bevel just the edges along the top and bottom
faces of a cylinder, as shown here. These edges form 90 degree angles, so as
long as the maximum angle is at least 90, they will have the bevel profile
applied. The angles between faces forming the sides are typically greater than
90 degrees (unless the cylinder has very low :ref:`complexity
<ug-complexity-tool>`). As long as the maximum angle is smaller than that those
edges will be left alone. The default is 120 degrees.

Grip Dragging
:::::::::::::

.. admonition:: VR Only

   .. incimage:: /images/BevelToolPanelGripMidpoint.jpg 200px right
   .. incimage:: /images/BevelToolPanelGripPoint.jpg    200px right

   In either VR mode, you can use the laser pointer and pinch action to edit
   the profile and adjust the sliders in the panel. However, it is very hard to
   make fine adjustments this way. It is much easier to use :ref:`touch mode
   interaction <ug-touch-mode>` or :ref:`grip-dragging
   <ug-grip-dragging>`. Touch mode interaction works as you would expect, where
   the touch affordance acts like the mouse.

   Grip dragging is enabled when the hover guide is pointed approximately
   towards the panel. The relative position of each controller determines
   whether it will interact with the main profile area or one of the two
   sliders. As usual, the hover guide will connect to the active input to
   indicate what will happen.

   When interacting with the profile area, the relative position of the
   controller determines whether a grip drag will operate on an existing point
   or will create a new point. In the left image, the controller position is
   closer to an existing point, so it highlights it. In the right image, it is
   not closer to an existing point, so it highlights the midpoint with a red
   square, indicating that grip-dragging it will create a new point that you
   can then move around.

.. _ug-csg-tool-panel:

CSG Tool Panel
,,,,,,,,,,,,,,

.. incimage:: /images/CSGToolPanel.jpg 200px right

The :newterm:`CSG Tool Panel` is a very simple panel that lets you change the
:ref:`CSG operation <ug-csg>` applied to all selected :ref:`CSG models
<ug-combined-models>`. Clicking any of the radio buttons applies the change
immediately.

|block-image|

.. _ug-import-tool-panel:

Import Tool Panel
,,,,,,,,,,,,,,,,,

.. incimage:: /images/ImportToolPanel.jpg 200px right

The :newterm:`Import Tool Panel` is essentially a :ref:`File Panel
<ug-file-panel>` that lets you specify the file to import or reimport the mesh
used for an :ref:`Imported model <ug-primitive-models>`. If multiple Imported
models are selected, all of them will be changed to use the file when the
``Accept`` button is pressed.

|block-image|

.. _ug-name-tool-panel:

Name Tool Panel
,,,,,,,,,,,,,,,

.. incimage:: /images/NameTool.jpg 200px right

The :newterm:`Name Tool Panel` is a very simple panel that is used by the
:ref:`Name Tool <ug-name-tool>` to edit the name of the :ref:`primary selection
<ug-selection>`. The tool is activated when you apply the general :ref:`Name
Tool action <ug-tool-actions>`. The model's name is updated whenever you hit
the ``Apply`` button.

|block-image|

.. _ug-revsurf-tool-panel:

RevSurf Tool Panel
,,,,,,,,,,,,,,,,,,

.. incimage:: /images/RevSurfToolPanel.jpg 200px right

The :newterm:`RevSurf Tool Panel` lets you edit the profile that is revolved
around the Z (up) axis for all selected :ref:`RevSurf (surface of revolution)
models <ug-primitive-models>`. The panel initially shows the current profile of
the :ref:`primary selection <ug-selection>`. Any changes to the profile are
applied to all selected RevSurf models; the models update in real time as the
profile is edited.

|block-image|

Editing the RevSurf Profile
:::::::::::::::::::::::::::

Editing the profile for the RevSurf models is essentially the same as in the
:ref:`Bevel Tool Panel <ug-bevel-tool-panel>`, except that there must always be
at least 3 points in the profile; the panel will not let you delete an interior
profile point if it is the only one left.

Adjusting the Sweep Angle
:::::::::::::::::::::::::

.. incimage:: /images/RevSurfToolPanelSweep.jpg 200px right

By default, a RevSurf model is created by revolving the profile 360 degrees
around the Z (up) axis. You can change this with the slider at the bottom of
the panel. Angles less than 360 result in a partial sweep with end cap
polygons, as shown here.

|block-image|

.. _ug-text-tool-panel:

Text Tool Panel
,,,,,,,,,,,,,,,

.. incimage:: /images/TextToolPanel.jpg 200px right

The :newterm:`Text Tool Panel` lets you edit the text string, font, and
character spacing for all selected :ref:`Text models <ug-primitive-models>`.
The panel initially shows the values for the :ref:`primary selection
<ug-selection>`. Clicking the ``Apply`` button applies all changes made with
the panel to all selected Text models.

The panel lets you specify the following settings:

  - :emphasis:`Text string`.
  - :emphasis:`Font name`. The dropdown contains all available fonts.
  - :emphasis:`Character spacing`. The value of this slider multiplies the
    spacing between individual characters to move them closer together or
    further apart. The default is 1, which is the standard spacing defined by
    the font.

.. _ug-tree-panel:

Tree Panel
..........

.. incimage:: /images/TreePanel.jpg 200px right

The :newterm:`Tree Panel` is the framed panel on the back wall. It has multiple
uses:

  - Displaying a list of all models in the scene, including their tree
    structure and current status.
  - Selecting models.
  - Hiding and showing models.
  - Changing model order.

As mentioned previously, the Tree Panel is always visible in the same place,
unlike other (movable) panels.

|block-image|

.. _ug-session-name:

Session Name and Status
,,,,,,,,,,,,,,,,,,,,,,,

At the top of the Tree Panel is a row showing the current name of the session
being edited.  If the session was not loaded from a file and has not yet been
saved with a name, it will show as ``<Untitled Session>``. To the left is a way
to change the visibility of all models, as described :ref:`below
<ug-visibility>`.

If the session has been modified since it was loaded or started, there will be
a string of special characters within square brackets after the session name.
This string may contain the following characters:

   - An asterisk (``*``) indicates that one or more models has been created or
     modified and not undone.
   - An exclamation point (``!``) indicates that the session state (such as
     :ref:`edge visibility <ug-toggle-show-edges>` or :ref:`build volume
     visibility <ug-toggle-build-volume>`) has changed.
   - A plus sign (``+``) indicates that the commands in the session file
     have changed, usually by being undone.

All three characters are shown for the session in the above image.

Note that a session can only be saved to the same file if at least one of these
symbols appears. For example, if you make changes to models and then undo back
to their original state, there will be no asterisk, but the plus sign lets you
know that the session can still be saved (because the sequence of commands has
changed). The session state is also saved, so making changes to it allow you to
save as well.

Model Names and Colors
,,,,,,,,,,,,,,,,,,,,,,

Every row under the top row of the Tree Panel represents a top-level model in
the scene along with its indented children, if it has any. If it has children,
a triangle appears on the left (as shown in the above image) that you can click
to collapse or expand the children.

The name of each model uses color-coded text to indicate its status:

  - The name of the :tree-primary:`primary selection is bold red`.
  - The names of all :tree-secondary:`secondary selections are bold blue`.
  - The names of all :tree-hbu:`models hidden because the visibility was turned
    off are purple and italic`.
  - The names of all :tree-hbm:`models hidden because some ancestor or
    descendent model is visible are gray and italic`.
  - The names of all other models are black.

.. _ug-visibility:

Changing Model Visibility
,,,,,,,,,,,,,,,,,,,,,,,,,

To the left of each top-level model name in the Tree Panel is a little eye
icon. Clicking this icon toggles the visibility of the corresponding model in
the scene.

There is also an eye icon next to the session name at the top. If any top-level
model is currently hidden, clicking this will show all top-level models using
the :ref:`Show All action <ug-show-all>`. Otherwise, it will hide all models.

.. _ug-tree-panel-selection:

Selecting Models
,,,,,,,,,,,,,,,,

.. incimage:: /images/TreePanelSelect.jpg 200px right

Clicking with the mouse (or pinch) on a model name in the Tree Panel selects
that model as the primary selection. :ref:`Modified-clicking
<ug-modified-mode>` on a model name toggles its selection status.

You can also use the mouse (or pinch) to drag out a rectangle withinin the
panel. All model names intersected by the rectangle will be selected as shown
here. :ref:`Changing the order of model names <ug-reorder-models>` may help in
some cases to allow contiguous selections.

Note that you cannot have both a parent and child model in the same hierarchy
selected at the same time. That would be bad.

|block-image|

.. _ug-reorder-models:

Reordering Models
,,,,,,,,,,,,,,,,,

.. incimage:: /images/TreePanelUpDown.jpg 200px right

There are some rare cases where the order of models in the Tree Panel matters.
One is when you want to use :ref:`rectangle selection
<ug-tree-panel-selection>` to select some set of models and they need to be
listed consecutively. Another is when the order of children within a parent
matters, such as within a :ref:`CSG difference <ug-csg>`.

In these cases the two buttons to the right of the session name can be used to
apply the :ref:`Move Previous <ug-move-previous>` and :ref:`Move Next
<ug-move-next>` actions, which move the primary selection up and down in the
list. Note that if the primary selection is not a top-level model, it can only
be moved up and down within its parent model.

These buttons are enabled only if there is a single model selected and it can
be moved in the corresponding direction.
