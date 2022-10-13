.. _ug-panels:

Panels
------

As mentioned in the :ref:`Overview <ug-panel-overview>`, |appname| uses some
more conventional 2D user interface panels. Because these panels need to work
in :ref:`VR as well as in conventional mode <ug-vr-modes>`, they appear in the
3D :ref:`work area <ug-work-area>`. Panels are always aligned with the
XZ-plane.

Panels are divided into two main categories:

  - :newterm:`Application panels` implement various application functions, such
    as loading sessions or modifying program settings.  Application panels
    appear initially in a central location.
  - :newterm:`Tool panels` are used for :ref:`certain specialized tools
    <ug-specialized-tools>`. A tool panel appears initially above the
    :ref:`primary selection <ug-selection>` it is operating on.

The Tree Panel is special; it is always fixed to the back wall and has
:ref:`its own section in the documentation <ug-tree-panel>`.

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

Application Panels
..................

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

.. todo::
   Ended here

.. PANELS ------------
.. _ug-action-panel:
.. _ug-bevel-tool-panel:
.. _ug-csg-tool-panel:
.. _ug-dialog-panel:
.. _ug-file-panel:
.. _ug-help-panel:
.. _ug-import-tool-panel:
.. _ug-name-tool-panel:
.. _ug-revsurf-tool-panel:
.. _ug-session-name:
.. _ug-tree-panel:
.. _ug-virtual-keyboard-panel:

### Action Panel

![][ImgActionPanel]{{rightimage(256)}}

The Action Panel is used by the [Radial Menu Editor
Panel][RadialMenuEditorPanel] to select the [action][Actions] associated with a
radial menu button. It presents a scrolling list of actions to choose from,
organized by category. The action that is currently bound to the button being
modified is marked as `[CURRENT]`.

See the [Cheat Sheet][CheatSheet] for a list of all actions and their
associated icons.

{{verticalspace(2)}}

### File Panel

![][ImgFilePanel]{{rightimage(256)}}

The File Panel is used by the [Session Panel][SessionPanel] and the [Settings
Panel][SettingsPanel] when a directory or file needs to be chosen. In addition,
the [Import Tool Panel][ImportToolPanel] is essentially a File Panel.

This panel works pretty much like a standard file browser. The four buttons at
the top go to the previous directory (if any), the next directory (if you went
to the previous one), one directory up, or your home directory. A button at the
bottom lets you see hidden files and directories (operating-system-specific).

The scrolling list is color coded for directories and files, with directories
listed first.

The File Format dropdown at the top right is used for selecting a format when
[exporting models][SessionPanel].

## Tool Panels

Each of these tool panels is a specialized tool for the corresponding type of
model. See [this table][GUIPanelTools] for the (obvious) connections.

### Bevel Tool Panel

![][ImgBevelToolPanel]{{rightimage(256)}}

The Bevel Tool Panel lets you edit the bevel created for all Beveled models
once they have been [created from other models][ConvertToBeveledAction]. The
bevel can actually be any sort of {{term('profile')}} applied to edges to
create various effects such as chamfering or rounding.

The panel initially shows the current profile of the [primary
selection][PrimarySelection]. The edited profile is applied to edges of all
selected Beveled models; the models update in real time as the profile is
edited.

The profile is drawn in the profile editing area and interpreted as follows:

+ The upper-right corner is where the edge is located, looking along its
  length.
+ The upper-left and lower-right points of the profile are at fixed locations
  and are colored blue to indicate this. You can add new points between them,
  move those points around, and delete points.

The default profile is just a line connecting the two fixed points, which
creates a bevel for all edges.

#### Editing the Profile with Mouse or Pinch

![][ImgBevelDeleteBox]{{rightimage(160)}}

Clicking anywhere on or near the profile line (except very close to an existing
point) creates a new interior point at that spot. New points are rendered the
same way as other interactive objects to indicate that they are movable.
Dragging on or near the line (instead of clicking) creates a new point and
immediately starts dragging it. Dragging an existing interior point moves it;
when you do this, a box with an "X" in it appears to allow you to delete the
point by dragging it over that box, as illustrated here.

#### Editing the Profile with Grip Drag

![][ImgBevelGripMidpoint]{{rightimage(160)}}
![][ImgBevelGripPoint]{{rightimage(160)}}

When [grip hovering selects the profile editing area][GUIPanelInteraction], it
will highlight each movable point and also the midpoint of each profile line
segment, whichever is closest to the current relative controller position. The
midpoint is rendered as a blue square, as shown here. When a point is
highlighted, grip dragging moves that point. When a midpoint square is
highlighted, grip dragging creates a new point along that segment and starts
dragging it. As in the mouse/pinch case, a delete box with an "X" appears to
allow points to be removed.

#### Profile Scaling

![][ImgBevelScaleLarge]{{rightimage(200)}}
![][ImgBevelScaleSmall]{{rightimage(200)}}

The slider at the bottom of the panel allows you to scale the size of the
profile as it is applied to edges without having to readjust any points. For
example, you can create a rounding profile and change the radius by adjusting
the slider as shown here.

{{verticalspace(3)}}

#### Maximum Angle

![][ImgBevelMaxAngle]{{rightimage(220)}}

The slider on the right side of the panel allows you to change the maximum edge
angle, from 0 to 180 degrees. This determines which model edges are beveled.
For example, suppose you have a cylinder and want to bevel just the edges
forming the top and bottom faces, as shown here. These edges form 90 degree
angles, so as long as the maximum angle is at least 90, they will have the
bevel profile applied. The edges between faces forming the sides are typically
greater than 90 degrees (unless the cylinder has very low
[complexity][ComplexityTool]; as long as the maximum angle is smaller than that
those edges will be left alone.  The default is 120 degrees.

### CSG Tool Panel

![][ImgCSGToolPanel]{{rightimage(180)}}

The CSG Tool Panel is a very simple panel that lets you change the [CSG
operation][CSG] applied to all selected CSG models.

{{verticalspace(4)}}

### Import Tool Panel

![][ImgImportToolPanel]{{rightimage(220)}}

The Import Tool Panel is essentially a [File Panel][FilePanel] that lets you
reimport the mesh used for an [Imported model][PrimitiveModels] or to change
which file to import it from. If multiple Imported models are selected, all of
them will be changed to use the new file if one is selected.

{{verticalspace(4)}}

### RevSurf Tool Panel

The RevSurf Tool Panel lets you edit the profile that is revolved around the +Z
axis for all selected [RevSurf (surface of revolution)
models][PrimitiveModels]. The panel initially shows the current profile of the
[primary selection][PrimarySelection]. The edited profile is applied to all
selected RevSurf models; the models update in real time as the profile is
edited.

#### Profile Editing

![][ImgRevSurfToolPanel]{{rightimage(200)}}

Profile editing is essentially the same as in the [Bevel Tool
Panel][BevelToolPanel], with two (related) exceptions:

+ The default profile has 3 points as shown here.
+ There must be at least 3 points in the revolved profile, so the panel will
  not let you delete a movable point if it is the only one left.

{{verticalspace(2)}}

#### Sweep Angle

![][ImgRevSurfSweep]{{rightimage(256)}}

Below the profile editing area is a slider that lets you set the sweep angle in
degrees for the surface. The default is 360, meaning that the profile makes a
complete revolution. Values less than 360 result in a partial sweep with end
cap polygons, as shown here.

{{verticalspace(4)}}

### Text Tool Panel

![][ImgTextToolPanel]{{rightimage(200)}}

The Text Tool Panel lets you edit the text string and font characteristics used
to create 3D text for all selected [Text models][PrimitiveModels]. The panel
initially shows the values for the [primary selection][PrimarySelection]. All
changes made with the panel affect all selected Text models.

The panel lets you specify the following settings:

+ Text string.
+ Font family. You can select from the dropdown list containing all available
  font families.
+ Font style. This dropdown lets you select from the styles available for the
  currently-selected family.
+ Character spacing (slider). This factor multiplies the spacing between
  individual characters to move them closer together or further apart. The
  default is 1, which is the spacing defined by the font.

Note that the text shown in this image has been rotated to face the camera for
demonstration purposes, since it is extruded in the +Z direction for 3D
printing.
