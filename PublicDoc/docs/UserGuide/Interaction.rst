Interaction
-----------

While MakerVR is designed to be reasonably intuitive, there are a few things
that may need explanation before you can use it effectively.

Keyboard Shortcuts
..................

All keyboard shortcuts are listed in the :doc:`Cheat Sheet <CheatSheet>`. In
the future, there may be a way for users to change them.

Clicking and Dragging
.....................

|appname| responds to mouse and keyboard actions in any of the :ref:`three
modes <ug-vr-modes>`. Note that the left mouse button is used for all click and
drag actions. The right mouse button is used to :ref:`change the view
orientation <ug-viewing>`.

In either of the VR modes, controller use is as follows:

![Controller feedback with laser pointer and hover
guide][ImgControllerVR]{{rightimage(200)}}

+ The controllers appear in the VR scene as hands that react to interaction,
  courtesy of [SteamVR](https://valvesoftware.github.io/steamvr_unity_plugin/).

+ A cyan laser pointer emanates from the forward direction of each controller.
  the {{term('pinch')}} action (usually the trigger button) activates whatever
  the laser is pointing to, if it is interactive. The pinch action is used for
  the controller equivalent of clicking and dragging with the mouse.

+ Some [tools][Tools] allow you to use a controller's {{term('grip button')}}
  to interact. In this case, a tool will attach a {{term('hover guide')}} to
  the hands to help indicate what the grip action will affect. When active, the
  hover guide will show a connection to the interactive element that will be
  activated when the grip button is pressed. There are different types of hover
  guides; see the descriptions of the individual tools for details on grip
  dragging. The connection and guide are usually [color-coded][Colors].

Note that in [hybrid mode][VRModes], some behind-the-scenes nonsense takes
place so that you can see the hands, guides, and pointers in the (non-VR)
window.

Panel Touch Mode
----------------



Radial Menus
............

![Radial menu in VR][ImgRadialMenuVR]{{rightimage(200)}}

In [VR or hybrid mode][VRModes], pressing the {{term('menu')}} button on a
controller activates a {{term('radial menu')}} attached to the corresponding
hand in the scene. Each button in a radial menu is a shortcut to some
action. Slide your thumb around the trackpad or move the joystick (depending on
the controller) to highlight the desired item and press the trackpad or
joystick button to activate it. The menu is dismissed when you activate any
radial menu button, if you press the *menu* button again, or if you press any
keyboard key.

Note that radial menus are also available for use with the mouse in
conventional mode via [keyboard shortcuts][CheatSheet]. In this mode, the menu
appears in a fixed position on the appropriate side of the window for
interaction with the mouse. You can configure the radial menu for each hand in
the [Radial Menu Editor Panel][RadialMenuEditorPanel].

## Alternate Interaction Mode

Some interactive objects have two different modes of operation, regular and
what is referred to as {{term('Alternate Mode')}}. The term {{term('"Alt"')}}
is also used to refer to this mode within this guide, such as in "Alt-click",
"Alt-drag", or "Alt-select". {{tiny('Please do not confuse this with the "Alt"
key.')}}

Alternate mode is activated by either of these:

+ Holding down the shift key on the keyboard
+ Double-clicking with the mouse, pinch, or grip action. Note that this works
  both for Alt-clicking and for Alt-dragging (press-release-press-drag).

For example, when clicking on a model to select it, the Alt version is to
multi-select the model or deselect it if it is already selected. When dragging
a handle in the [Scale Tool][ScaleTool], the Alt version performs a symmetric
scale about the model's center point instead of a asymmetric scale. Other uses
are described elsewhere in this guide.

## The Stage

The {{term('stage')}} is the large disc on the floor of the work area. Models
are placed on the stage by default when they are created.

The stage can be manipulated to change your view of the models:

+ Drag it in a circular arc to rotate it along with the models on it.
+ Drag toward or away from the stage center to scale the stage and models on it
  up or down.
+ In conventional mode, the mouse wheel also scales the stage.
+ [Alt-clicking][AltMode] anywhere on the stage resets it to its default size
  and rotation.

Rotating and scaling the stage is a fairly easy way to get a different
perspective on your models without modifying the models themselves. Another
option is to use [inspector mode][InspectorMode].

## Viewing

|appname| is designed to be maintain a relatively stable viewpoint. However,
there may be times when you want to change the view of your scene (in addition
to manipulating the [stage][Stage] or using [inspector mode][InspectorMode]).

In VR mode, you can turn your head to see the room from different angles, if
you wish. In [conventional or hybrid mode][VRModes], you can use the right
mouse button to rotate the view. Additionally, the height slider on the pole on
the right side of [the work area][WorkArea] can be dragged up and down to raise
and lower your view of the room. Clicking on the ring brings you back to the
default height.  [Alt-clicking][AltMode] on the ring also resets the view
direction to the default, unless you are in VR mode (where the view always
aligns with your head, of course).

## Inspector Mode

|appname| provides a special {{term('inspector mode')}} that lets you easily
examine a model from all directions. To inspect a model, [select it as the
primary selection][Selection], and click on the [Inspector
action][InspectorAction] icon.  A shortcut to this mode is to long-press (more
than a half-second or so) either the mouse button or laser pointer/pinch action
on a model, whether selected or not. There is also a [keyboard
shortcut][CheatSheet].

When in [VR mode][VRModes] with the headset on, the model will be attached to
the controller in the hand that initiated  inspection. You can move and turn
your hand around to see the model from all angles.

When in [conventional mode or hybrid mode][VRModes], the model will be enlarged
and placed in the middle of the application window. In conventional mode,
moving the mouse (no need to click) rotates the model. In hybrid mode,
interaction depends on whether inspector mode was initiated with the mouse. If
so, it is the same as conventional mode. If inspector mode was initiated with a
controller, that controller rotates the model as in VR mode.

Any key press, mouse click, or controller button press exits inspector mode.

## Tooltips

Anything in the application that is interactive responds to hovering with the
mouse or laser pointer by highlighting in some way. For most items, if you
hover long enough, a tooltip will appear with information about the
interaction. For example, hovering over a tool icon gives information about
what the tool does, while hovering over any model in the scene shows the name
of the model.

The delay for showing tooltips is a setting that can be edited in the [Settings
Panel][SettingsPanel]. Setting the delay to 0 turns off tooltips completely.

## Text Input

Some parts of the application require text input, such as editing the name of a
model or entering the name of a new file. Using the keyboard is always an
option for text. If you have the headset on in VR mode, you might not want to
take it off or put the controllers down just to edit text. You're in luck; text
editing in VR mode is available using
[VRKeys](https://github.com/campfireunion/VRKeys).  It provides a virtual
keyboard with a xylophone-like interface for editing text. It's pretty cool.

Attached to the front of the virtual keyboard is a bar that can be used to
reposition the keyboard. Grip-dragging this bar with either controller allows
you to move the bar and keyboard in all three dimensions to make it easier to
access.
