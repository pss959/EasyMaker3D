.. _ug-actions:

Shelves, 3D Icons, and Actions
------------------------------

Most of the interaction in |appname| is initiated by clicking on one of the 3D
icons on the various shelves in the :ref:`work area <ug-work-area>`. (See that
section in the Overview for the shelf names and locations.)

Each of the 3D icons represents an :newterm:`action` that makes some sort of
change to the scene or program state. These actions can be divided into these
general types:

  - A :newterm:`toggle action` changes some program state that is either on or
    off. For example, the :ref:`Show Edges toggle action
    <ug-toggle-show-edges>` turns the visibility of polygonal edges on models
    on or off.
  - A :newterm:`tool action` selects a :ref:`tool <ug-tools>` to attach to the
    :ref:`currently-selected model(s) <ug-selection>`.
  - A :newterm:`panel action` brings up a :ref:`2D panel <ug-panels>` that
    provides some special interface.
  - All other actions produce some sort of immediate change to models, the
    selection state, or the session state.

All actions can be applied from a :ref:`radial menu <ug-radial-menus>`, and
many have corresponding :ref:`keyboard shortcuts <ug-keyboard-shortcuts>`. The
:doc:`Cheat Sheet <CheatSheet>` lists all supported actions along with their
names, radial menu icons, and keyboard/controller shortcuts. The actions are
divided into categories, some of which correspond to shelves in the work area.
Note that some actions do not have corresponding 3D icons; they can be
performed only from a radial menu or shortcut.

Certain actions may not be enabled due to the current application state. A 3D
icon for an enabled action will appear with the regular light woodgrain
texture. An icon for a disabled action will be dark instead. 3D icons that
represent enabled toggle actions will be colored green when the toggle is in
the "on" state and regular woodgrain when in the "off" state.

The following sections detail what each action does in the application, divided
by category.

.. _ug-create-box:
.. _ug-create-cylinder:
.. _ug-create-imported-model:
.. _ug-create-rev-surf:
.. _ug-create-sphere:
.. _ug-create-text:
.. _ug-create-torus:

Model Creation Actions
......................

.. incimage:: /images/ModelCreationShelf.jpg 300px right

Model Creation actions are all available as 3D icons on the bottom shelf on the
left wall of the :ref:`work area <ug-work-area>`. Each creates :ref:`a model of
a specific type <ug-model-types>`

From left to right on the shelf, the icons/actions are:

  - Box model (:action:`Create Box`)
  - Cylinder model (:action:`Create Cylinder`)
  - Sphere model (:action:`Create Sphere`)
  - Torus model (:action:`Create Torus`)
  - Surface of revolution model (:action:`Create Rev Surf`)
  - Text model (:action:`Create Text`)
  - Imported model (:action:`Create Imported Model`)

Clicking on any of these icons creates a new instance of the corresponding type
of model, which drops in from above. If the :ref:`Point Target <ug-targets>` is
visible, the new model will end with its bottom center point at the targets's
position and with the model's +Z ("up") direction aligned with the Point
Target's direction.

The new model automatically becomes the :ref:`primary selection
<ug-selection>`. If the model type has a :ref:`specialized tool
<ug-specialized-tools>` associated with it, that tool will automatically be
attached to it. Otherwise, the :ref:`current general tool <ug-general-tools>`
will be attached.

General Tool Actions
....................

.. incimage:: /images/GeneralToolShelf.jpg 300px right

Actions to choose the current :ref:`general tool <ug-general-tools>` are
available as 3D icons on the shelf above the Model Creation shelf.  Clicking on
any of these icons attaches the corresponding tool to the :ref:`primary
selection <ug-selection>`. Note that most of these tools affect all selected
models.

From left to right the general tool icons are:

  - The :action:`Name Tool` action attaches the `Name Tool <ug-name-tool>` to
    edit the name of the primary selection.
  - The :action:`Color Tool` action attaches the :ref:`Color Tool
    <ug-color-tool>` to change the color of all selected models.
  - The :action:`Complexity Tool` action attaches the:ref:`Complexity Tool
    <ug-complexity-tool>` to change the tessellation of all selected
    model(s). (Note that only cylinder, sphere, torus, surface of revolution,
    and text models respond to complexity changes; the action is disabled if
    only other types of models are selected.)
  - The :action:`Rotation Tool` action attaches the :ref:`Rotation Tool
    <ug-rotation-tool>` to rotate all selected models.
  - The :action:`Scale Tool` action attaches the:ref:`Scale Tool
    <ug-scale-tool>` to change the size of all selected models.
  - The :action:`Translation Tool` action attaches the :ref:`Translation Tool
    <ug-translation-tool>` to change the position of all selected models.

.. _ug-convert-bevel:
.. _ug-convert-clip:
.. _ug-convert-mirror:

Model Conversion Actions
........................

.. incimage:: /images/ModelConversionShelf.jpg 200px right

The center shelf on the left wall has 3D icons that apply actions to create
:ref:`converted models <ug-converted-models>` from the :ref:`selected models
<ug-selection>`. Applying any of these actions converts all selected models to
a specific type and then attaches the corresponding :ref:`specialized tool
<ug-specialized-tools>` to the primary selection.  From left to right, they
are:

  - The :action:`Convert Bevel` action converts all selected models to Beveled
    models and attaches the :ref:`Bevel Tool <ug-bevel-tool>` to the primary
    selection.
  - The :action:`Convert Clip` action converts all selected models to Clipped
    models and attaches the :ref:`Clip Tool <ug-clip-tool>` to the primary
    selection.
  - The :action:`Convert Mirror` action converts all selected models to
    Mirrored models and attaches the :ref:`Mirror Tool <ug-mirror-tool>` to the
    primary selection.

.. _ug-combine-csg-difference:
.. _ug-combine-csg-intersection:
.. _ug-combine-csg-union:
.. _ug-combine-hull:

Model Combination Actions
.........................

.. incimage:: /images/ModelCombinationShelf.jpg 240px right

Actions to create a :ref:`combined model <ug-combined-models>` from the
:ref:`selected models <ug-selection>` are available as 3D icons on the shelf
above the Model Conversion shelf on the left wall. From left to right, they
are:

  - The :action:`Combine CSG Difference` action applies the :ref:`CSG
    difference <ug-csg>` operation to all selected models and attaches the
    :ref:`CSG Tool <ug-csg-tool>` to the resulting CSG model. Note that this
    operation is not symmetric, so it depends on the selection order.
  - The :action:`Combine CSG Intersection` action applies the :ref:`CSG
    intersection <ug-csg>` operation to all selected models and attaches the
    :ref:`CSG Tool <ug-csg-tool>` to the resulting CSG model.
  - The :action:`Combine CSG Union` action applies the :ref:`CSG union
    <ug-csg>` operation to all selected models and attaches the :ref:`CSG Tool
    <ug-csg-tool>` to the resulting CSG model.
  - The :action:`Combine Hull` action applies the :ref:`convex hull <ug-hull>`
    operation to all selected models. There is no specialized tool for a hull
    model, so the current :ref:`general tool <ug-general-tools>` is attached to
    the resulting model.

The CSG actions require at least two models to be selected, but the convex hull
action can be applied to a single model.

.. _ug-toggle-specialized-tool:

Specialized Tool Action
.......................

Each :ref:`specialized tool <ug-specialized-tools>` can be attached only to a
specific type of model. When a model of that type is created, the corresponding
specialized tool is attached to it automatically.

The :action:`Toggle Specialized Tool` action switches between the specialized
tool for the :ref:`current selection <ug-selection>` and the current
:ref:`general tool <ug-general-tools>`. This action is disabled if any selected
model has no specialized tool or if multiple models are selected and they are
not all of the same type.

.. incimage:: /images/TorusToolIcon.jpg    80px right
.. incimage:: /images/TextToolIcon.jpg     80px right
.. incimage:: /images/RevSurfToolIcon.jpg  80px right
.. incimage:: /images/MirrorToolIcon.jpg   80px right
.. incimage:: /images/ImportToolIcon.jpg   80px right
.. incimage:: /images/CylinderToolIcon.jpg 80px right
.. incimage:: /images/ClipToolIcon.jpg     80px right
.. incimage:: /images/CSGToolIcon.jpg      80px right
.. incimage:: /images/BevelToolIcon.jpg    80px right
.. incimage:: /images/NullIcon.jpg         80px right

|block-image|

Clicking on the 3D icon on the top shelf on the left wall applies the toggle
action if available. This icon changes shape to reflect what the toggle will
do. It will be one of the icons shown here, from left to right:

  - The null icon is shown when no model is selected or no specialized tool
    that can be applied to the current selection. It is always disabled.
  - The :ref:`Bevel Tool <ug-bevel-tool>` icon is shown when the current
    selection consists of only :ref:`Beveled models <ug-converted-models>`.
  - The :ref:`CSG Tool <ug-csg-tool>` icon is shown when the current
    selection consists of only :ref:`CSG models <ug-combined-models>`.
  - The :ref:`Clip Tool <ug-clip-tool>` icon is shown when the current
    selection consists of only :ref:`Clipped models <ug-converted-models>`.
  - The :ref:`Cylinder Tool <ug-cylinder-tool>` icon is shown when the current
    selection consists of only :ref:`Cylinder models <ug-primitive-models>`.
  - The :ref:`Import Tool <ug-import-tool>` icon is shown when the current
    selection consists of a single :ref:`Imported model <ug-primitive-models>`.
  - The :ref:`Mirror Tool <ug-mirror-tool>` icon is shown when the current
    selection consists of only :ref:`Mirrored models <ug-converted-models>`.
  - The :ref:`Rev Surf Tool <ug-rev-surf-tool>` icon is shown when the current
    selection consists of only :ref:`RevSurf models <ug-primitive-models>`.
  - The :ref:`Text Tool <ug-text-tool>` icon is shown when the current
    selection consists of only :ref:`Text models <ug-primitive-models>`.
  - The :ref:`Torus Tool <ug-torus-tool>` icon is shown when the current
    selection consists of only :ref:`Torus models <ug-primitive-models>`.

All but the null icon will be shown with the active icon color when the
corresponding specialized tool is attached and the regular woodgrain icon color
when toggled back to a general tool.

Note that the :shortcut:`Space` shortcut is an easy way to toggle between
general and specialized tools when enabled.

.. _ug-copy:
.. _ug-cut:
.. _ug-delete:
.. _ug-paste-into:
.. _ug-paste:
.. _ug-redo:
.. _ug-undo:

Basic Actions
.............

.. incimage:: /images/BasicsShelf.jpg 300px right

|appname| provides most of the basic operations you would expect in an
interactive application. Each of these is available by clicking on a 3D icon on
the Basic Operations shelf on the top of the back wall in the :ref:`work area
<ug-work-area>`. From left to right, these are:

  - The :action:`Undo` action undoes the last operation performed that modified
    the scene in some way.
  - The :action:`Redo` action redoes the last undone action.
  - The :action:`Delete` action deletes all selected models from the
    scene. They are gone forever, unless you undo.
  - The :action:`Cut` action copies all selected models from the scene to the
    clipboard and removes them from the scene.
  - The :action:`Copy` action copies all selected models from the scene into
    the clipboard.
  - The :action:`Paste` action pastes all models from the clipboard into the
    scene as top-level models at their previous 3D locations.
  - The :action:`Paste Into` action can be used to paste models from the
    clipboard as children of the :ref:`primary selection <ug-selection>` model,
    as long as that model is one that can have children. For example, if you
    have a :ref:`CSG union model <ug-combined-models>` and you want to add a
    Box model to it, you can cut or copy the box in its correct position,
    select the CSG model, and click the Paste Into icon. The box will be added
    as a child of the union and therefore become part of the CSG operation.

Note that Undo and Redo actions apply to these operations:

  - Any change to a model or models.
  - Changing the position or any other aspect of either of the :ref:`targets
    <ug-targets>`.

Actions that are :emphasis:`not` considered changes and are therefore not
undoable include:

  - Changing the view.
  - Changing the orientation or size of the:ref:`stage <ug-stage>`.
  - :ref:`Selecting or deselecting models <ug-selection>`. (However, undoing or
    redoing other actions may affect the current selection.)
  - Toggling any program state, such as target or edge visibility.
  - Applying the Copy action, since it has no visible effect except to update
    the clipboard. Note that you may be able to :ref:`use this to your
    advantage <ug-copy-quirk>`.

Hovering over the Undo or Redo icon (when enabled) shows a :ref:`tooltip
<ug-tooltips>` describing what operation will be undone or redone.

.. _ug-linear-layout:
.. _ug-move-to-origin:
.. _ug-radial-layout:
.. _ug-toggle-axis-aligned:
.. _ug-toggle-edge-target:
.. _ug-toggle-point-target:

Layout Actions
..............

.. incimage:: /images/LayoutShelf.jpg 300px right

The following actions are available as 3D icons on the Layout shelf, which is
just below the Basic Operations shelf on the back wall. From left to right:

  - The :action:`Toggle Point Target` action changes the visibility of the
    :ref:`Point Target <ug-targets>`.
  - The :action:`Toggle Edge Target` action changes the visibility of the
    :ref:`Edge Target <ug-targets>`.
  - The :action:`Linear Layout` action uses the :ref:`Edge Target <ug-targets>`
    to lay out selected models along a line. See the section on :ref:`linear
    layout <ug-layout>` for details.
  - The :action:`Radial Layout` action uses the :ref:`Point Target
    <ug-targets>` to lay out multiple selected models along a circular arc or
    to move a single model to the target. See the section on :ref:`radial
    layout <ug-layout>` for details.
  - The :action:`Toggle Axis Aligned` action changes whether certain
    :ref:`tools <ug-tools>` operate in the selected model's local coordinates
    or in :ref:`stage <ug-stage>` coordinates. For example, if a model has been
    rotated to an arbitrary orientation and you want to move it 5 units to the
    right in the scene, you can turn this toggle on and the :ref:`Translation
    Tool <ug-translation-tool>` will then align its geometry and motion with the
    stage axes, not the model's local axes. This toggle affects the :ref:`Clip
    Tool <ug-clip-tool>`, :ref:`Mirror Tool <ug-mirror-tool>`, :ref:`Rotation
    Tool <ug-rotation-tool>`, and :ref:`Translation Tool <ug-translation-tool>`.

One other layout-related action is available only via a :ref:`keyboard shortcut
<ug-keyboard-shortcuts>` or :ref:`radial menu <ug-radial-menus>`:

  - The :action:`Move To Origin` action translates the :ref:`primary selection
    <ug-selection>` so that its bottom center is at the origin (center of the
    :ref:`stage <ug-stage>`) without rotating. Any secondary selections are
    moved by the same amount.

.. todo::
   Ended here.

### File Shelf
![][ImgFileShelf]{{rightimage(80)}}

The file shelf has four icons, each of which brings up a specific [GUI
Panel][GUIPanels]. From left to right, they are:

+ {{anchor('session-panel-action')}}The {{term('Session-Panel')}} action brings
  up the [Session Panel][SessionPanel], which allows you to save your session,
  load a new session, and so on.
+ {{anchor('settings-panel-action')}}The {{term('Settings-Panel')}} action
  brings up the [Settings Panel][SettingsPanel], which allows you to edit
  application settings.
+ {{anchor('info-panel-action')}}The {{term('Info-Panel')}} action brings up
  the [Info Panel][InfoPanel], which shows information about currently selected
  models.
+ {{anchor('help-panel-action')}}The {{term('Help-Panel')}} action brings up
  the [Help Panel][HelpPanel], which displays the application version and has
  buttons to open this guide or the [Cheat Sheet][CheatSheet] in a browser.

## Viewing Shelf
![][ImgViewingShelf]{{rightimage(80)}}

The viewing shelf is alone above the [Tree Panel][TreePanel] on the back
wall. It has icons for tools that deal with viewing models. From left to right:

+ {{anchor('inspector-action')}}The {{term('Inspector')}} action shows the
  current primary selection in [inspector mode][InspectorMode].
+ {{anchor('build-volume-toggle')}}The {{term('Build-Volume')}} toggle action
  shows or hides a representation of your 3D printer's [build
  volume][BuildVolume].
+ {{anchor('show-edges-toggle')}}The {{term('Show-Edges')}} toggle action lets
  you [show or hide model edges][ShowEdges] as visible lines.

## Other Actions

There are other actions available through other parts of the work area or only
through [radial menus][RadialMenus] and keyboard shortcuts; see the [Cheat
Sheet][CheatSheet] for details. They are:

+ {{anchor('edit-name-action')}}The {{term('Edit-Name')}} action is the same as
  using the [Tree Panel][TreePanel] to [edit the name][EditName] of the primary
  selection.
+ {{anchors(['hide-selected-action','show-all-action'])}}The
  {{term('Hide-Selected')}} action hides all selected top-level models, and the
  {{term('Show-All')}} action shows all hidden top-level models.
+ {{anchors(['increase-complexity-action','decrease-complexity-action'])}}The
  {{term('Increase-Complexity')}} and {{term('Decrease-Complexity')}} actions
  increase or decrease the complexity of selected models by .05 (within the 0-1
  range). This can be a quicker way to modify complexity than by using the
  [Complexity Tool][ComplexityTool].
+ {{anchors(['increase-precision-action','decrease-precision-action'])}}The
  {{term('Increase-Precision')}} and {{term('Decrease-Precision')}} actions are
  equivalent to clicking the corresponding buttons on the [precision
  control][PrecisionLevel].
+ {{anchors(['left-radial-menu-toggle','right-radial-menu-toggle'])}}The
  {{term('Left-Radial-Menu')}} and {{term('Right-Radial-Menu')}} toggle actions
  show or hide the [radial menu][RadialMenus] for the corresponding hand.
+ {{anchors(['move-previous-action','move-next-action'])}}The
  {{term('Move-Previous')}} and {{term('Move-Next')}} actions are the same as
  using the [Tree Panel][TreePanel] to [change the order of
  models][ReorderingModels] within a parent model or at the top level.
+ {{anchors(['select-all-action','select-none-action'])}}The
  {{term('Select-All')}} action selects all top-level models, and the
  {{term('Select-None')}} action deselects all selected models.
+ {{anchors(['select-first-child-action', 'select-previous-sibling-action',
  'select-next-sibling-action', 'select-parent-action'])}}The
  {{term('Select-First-Child')}}, {{term('Select-Previous-Sibling')}},
  {{term('Select-Next-Sibling')}}, and {{term('Select-Parent')}} actions are
  used to [select through model hierarchies][SelectInHierarchy].
+ {{anchors(['switch-to-previous-tool-action','switch-to-next-tool-action'])}}The
  {{term('Switch-to-Previous-Tool')}} and {{term('Switch-to-Next-Tool')}}
  actions [change which is the current general tool][ToolSwitching].
