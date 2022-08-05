# GUI Panels

MakerVR uses a few conventional user interface panels. Although they appear in
the 3D work space, they are essentially 2D and are always aligned with the
XZ-plane.

Panels are divided into two main categories:

+ {{term('Application panels')}} implement various application functions, such
  as loading sessions or modifying program settings. Application panels appear
  initially over the center of the [stage][Stage].
+ {{term('Tool panels')}} are [used for certain specialized
  tools][GUIPanelTools]. A tool panel appears initially above the [primary
  selection][PrimarySelection].

Note that the [Tree Panel][TreePanel] is a special beast and is described
elsewhere.

## Moving and Resizing Panels

![][ImgPanelHandles]{{rightimage(200)}}

Panels are placed initially to be reasonably visible. Additionally, when
application panels are visible, they temporarily hide the scene contents so
there is no interference. Tool panels do not do this so you can see the results
of editing models with them.

You can move any application or tool panel if it is in an inconvenient place.
All panels have an interactive handle on the center of each edge that allows
you to move the panel. The left and right edge handles move the panel to the
left and right, and the top and bottom edge handles move it up and down. Once
an application or tool panel has been moved, it remembers its position.

Some panels (such as the [Info Panel][InfoPanel] shown here) also have
interactive sphere handles in the four corners that allow you to resize
them. Most panels have a minimum size and will not let you resize them smaller
than that.

All of these panel handles respond to grip-dragging; the orientation of a
controller determines which handle will be used. For example, if the [hover
guide][ClickDrag] for the right controller is close to parallel to the X axis,
it will highlight the move handle on the right side, so you can grip-drag it.
If the panel has scale handles and the guide is close to the diagonal
direction, it will highlight the corrresponding scale handle.

## Interacting with Panel Elements

![][ImgPanelColors]{{rightimage(200)}}

+ All panel buttons highlight when hovered with the mouse or laser pointer. A
  highlighted button will be shown with a blue background.
+ Keyboard navigation between buttons is possible with arrow keys and also
  `tab`/`shift-tab`. The current keyboard-focused button is shown with a green
  background. If nothing is shown with focus, hitting the tab key should focus
  on the first button.
+ Disabled buttons are dark gray.
+ Clicking on a button with the mouse or pinch action activates it.
+ Pressing enter on the keyboard with a button focused activates it.
+ Pressing the `escape` key always cancels whatever the panel is doing and
  dismisses it.

Some panels contain scrolling areas when their contents are too large to
fit. You can scroll a panel using the scrollbar on the right (mouse only),
dragging the contents up or down (mouse only), or by pressing the Up or Down
trackpad/joystick buttons on a VR controller. The same is true for long
dropdown lists.

Note that most interactive elements also support clicking with the grip
button. When the [hover guide][ClickDrag] is close enough to perpendicular to
the panel, it will highlight the GUI element that it will interact with.

## Application Panels

### Session Panel

![][ImgSessionPanel]{{rightimage(200)}}

The Session Panel is shown when the application starts and can also be brought
up at any time by clicking on the [Session Panel action][SessionPanelAction]
icon.  This panel is used to start, save, and load [sessions][Sessions].
Sessions are saved as text files with an `".mvr"` (MakerVR) extension. Feel
free to look at and edit the session files if that's your thing.

The Session Panel has the following buttons:

+ `Continue previous/current session`. When the application starts, this choice
  will be enabled if your previous session was saved with a name (which will
  appear here in parentheses). Click this to resume working in that session. If
  the panel is invoked during a session, clicking this button will act as if
  nothing happened.
+ `Load session file`. This can be used to load a different session file with
  the [File Panel][FilePanel]; you will be informed if you made any changes to
  the current session (if any) that you might want to save.
+ `Start new session`. This resets everything to empty and starts a new,
  unnamed session. If you do this during a session, you will be informed if you
  made any changes that you might want to save.
+ `Save session`. If your session is already named, this will be enabled to
  save it again if you made any changes.
+ `Save session as`. This lets you save your session with a different name
  using the [File Panel][FilePanel].
+ `Export Selection`. This lets you export the selected model(s) to a file,
  using a [File Panel][FilePanel] that also contains a dropdown that allows you
  to select the desired format (text or binary STL).
+ `Help` (question mark icon at top left). Brings up the [Help
  Panel][HelpPanel]. Dismissing the Help Panel returns to the Session Panel.
+ `Settings` (gear icon at top right). Brings up the [Settings
  Panel][SettingsPanel]. Dismissing the Settings Panel returns to the Session
  Panel.

The default directories for session files and STL files are both stored as user
settings and can be modified with the [Settings Panel][SettingsPanel].

### Settings Panel

![][ImgSettingsPanel]{{rightimage(240)}}

The Settings Panel lets you modify application settings that are saved between
sessions. The location of the saved settings is defined by [Unity
PlayerPrefs](https://docs.unity3d.com/ScriptReference/PlayerPrefs.html).

#### Default Directories

The first three settings are the default directories for saving and loading
sessions, STL model import, and STL model export. Each of these lets you [enter
text][TextInput] for the directory name or click on the `Browse` button to open
a [File Panel][FilePanel] for that item.

#### Tooltip Delay

Next is a slider to adjust the time it takes for [tooltips][Tooltips] to appear
when hovering the mouse or laser pointer over an interactive object.  Values
are in seconds, from 0 at the left to 5 at the right. Setting this to 0
disables tooltips completely.

#### Build Volume Size

Next are three text input fields that allow you to specify the size of the
[build volume][BuildVolume] in all three dimensions, using the current
[units][Units].

#### Unit Conversion

Next are two pairs of dropdowns allowing conversion of units on model import
and export. These are explained further in the section on [Coordinate System
and Units][Units].

#### Radial Menus

The last button brings up the [Radial Menu Editor
Panel][RadialMenuEditorPanel], which allows you to edit which buttons appear in
[radial menus][RadialMenus].

### Info Panel

![][ImgInfoPanel]{{rightimage(200)}}

The Info Panel shows information about all selected models, including the
number of vertices, number of triangles, and size in all three dimensions. If
any model has an invalid mesh, this is noted, along with the reason it is
considered invalid.

The info area in the panel will contain scroll bars if there is more text than
will fit. Also note that the Info Panel has resize handles (spheres) in the
corners to change its size.

### Help Panel

![][ImgHelpPanel]{{rightimage(180)}}

The Help Panel shows text with the current application version. There are two
buttons below the text. The first one brings up this user guide in your default
browser. The second one shows the [Cheat Sheet][CheatSheet] in the browser.

{{verticalspace(3)}}

### Radial Menu Editor Panel

![][ImgRadialMenuEditorPanel]{{rightimage(256)}}

This panel can be opened from the [Settings Panel][SettingsPanel] to edit the
[radial menus][RadialMenus]. The radio buttons in the box at the top let you
choose one of the following options:

+ `Use Independent Settings`: The buttons defined for each controller are used
  for that controller.
+ `Use Left Settings for Both`: The buttons defined for the left controller are
  used for both controllers.
+ `Use Right Settings for Both`: The buttons defined for the right controller
  are used for both controllers.

You can set the number of buttons to use on a controller to 1, 2, 4, or 8. The
menu diagram updates to reflect the current number. Clicking on any button in
the diagram brings up the [Action Panel][ActionPanel] to let you choose the
[action][Actions] attached to that button. Each action is indicated by a
horribly-designed 2D icon that will show up in the radial menu diagram and in
the actual radial menu as well. The null icon
(![][ImgMINone]{{inlineimage(32)}}) is shown when no real action is bound to a
button.

In [VR and hybrid modes][VRModes] you can activate a radial menu by pressing
the menu button on a controller.  Radial menus are also available in
conventional mode via [keyboard shortcuts][Shortcuts] for use with the mouse.

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
