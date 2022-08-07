# Shelves and Actions

Each 3D icon on the shelves in [the work area][WorkArea] represents an
{{term('action')}} that makes some sort of change to the scene or program
state. These actions can be divided into these general types:

+ A {{term('toggle action')}} changes some program state that is either on or
  off. For example, the [Show Edges toggle action][ShowEdgesToggle] turns the
  visibility of polygonal edges on or off.
+ A {{term('tool action')}} selects a [tool][Tools] to attach to the
  [currently-selected model(s)][Selection].
+ A {{term('panel action')}} brings up a 2D [GUI Panel][GUIPanels] that
  performs some specific task.
+ All other actions make some sort of change immediately, whether to models,
  the selection state, or something else.

Each action has a corresponding name and 2D icon that is used to apply that
action from [radial menus][RadialMenus]. The [Cheat Sheet][CheatSheet] lists
all supported actions along with their names, icons, and shortcuts.

Note that some actions do not have corresponding 3D icons; they can be
performed only via a radial menu or keyboard shortcut.

## Icon Appearance

Most icons appear with a light woodgrain appearance. An icon that represents an
action that is disabled because of the current state will be dark instead.

Icons that represent toggle actions will be colored green when the toggle is in
the "on" state and regular woodgrain when in the "off" state.

## Organization

The 3D icons in the scene are organized into shelves, which correspond
(roughly) to the categories in the [Cheat Sheet][CheatSheet]. The shelves are
also organized into groups placed within [the work area][WorkArea] as follows:

+ Shelves along the left wall (bottom to top): [model creation][CreationShelf],
  [general tools][GeneralToolShelf], [model conversion][ConversionShelf],
  [model combination][CombinationShelf], and [specialized tool
  access][SpecializedToolShelf].
+ Shelves along the back wall (top to bottom): [basic actions][BasicsShelf],
  [layout helpers][LayoutShelf], and [file-related actions][FileShelf].
+ Shelf above the [Tree Panel][TreePanel] on the back wall: [viewing-related
  actions][ViewingShelf].

## Left Wall Shelves

These shelves are on the left wall of the [work area][WorkArea]. They are
listed from bottom to top.

### Model Creation Shelf
![][ImgModelCreationShelf]{{rightimage(100)}}

The icons on this shelf are used to create new [models of a specific
type][ModelTypes]. From left to right, these are:

+ Imported model
+ Box model
+ Cylinder model
+ RevSurf (surface of revolution) model
+ Sphere model
+ Text model
+ Torus model

Clicking on any of these icons creates a new instance of the corresponding
model, which drops in from above. If the [point target][PointTarget] is
visible, the new model will land with its bottom center point at the base of
the point target and with the model's +Z ("up") direction aligned with the
point target's direction arrow.

The new model is automatically [selected][Selection]. If the newly-created
model has a [specialized tool][SpecializedTools] associated with it, that tool
will automatically be attached to it. Otherwise, the [current general
tool][GeneralTools] will be attached.

### General Tool Shelf
![][ImgGeneralToolShelf]{{rightimage(80)}}

The icons on this shelf represent [general tools][GeneralTools] that can be
used to modify any model. In order from left to right, they are:

+ [Color Tool][ColorTool]
+ [Complexity Tool][ComplexityTool]
+ [Rotation Tool][RotationTool]
+ [Scale Tool][ScaleTool]
+ [Translation Tool][TranslationTool]

Clicking on any of these icons attaches the corresponding tool to the [primary
selection][PrimarySelection].

### Model Conversion Shelf
![][ImgModelConversionShelf]{{rightimage(80)}}

Each of the icons on this shelf is used to apply an immediate action to convert
[selected models][Selection] to specific types of [converted
models][ConvertedModels] so that some sort of specific editing operation can be
applied. From left to right, they are:

+ {{anchor('convert-to-beveled-action')}}The {{term('Convert-to-Beveled')}}
  action converts the selected models to Beveled models and attaches the [Bevel
  Tool][BevelTool] to the primary selection.
+ {{anchor('convert-to-clipped-action')}}The {{term('Convert-to-Clipped')}}
  action converts the selected models to Clipped models and attaches the [Clip
  Tool][ClipTool] to the primary selection.
+ {{anchor('convert-to-mirrored-action')}}The {{term('Convert-to-Mirrored')}}
  converts the selected models to Mirrored models and attaches the [Mirror
  Tool][MirrorTool] to the primary selection.

### Model Combination Shelf
![][ImgModelCombinationShelf]{{rightimage(80)}}

Each of these icons combines one or more [selected models][Selection] into a
[combined model][CombinedModels] of some type. From left to right, they are:

+ {{anchor('csg-difference-action')}}{{term('CSG Difference')}}
+ {{anchor('csg-intersection-action')}}{{term('CSG Intersection')}}
+ {{anchor('csg-union-action')}}{{term('CSG Union')}}
+ {{anchor('convex-hull-action')}}{{term('Convex Hull')}}

Note that the CSG operations require 2 or more models to be selected, but the
Convex Hull operation can be applied to a single model.

When one of the CSG operations is applied, the [CSG Tool][CSGTool] will be
automatically attached to the resulting CSG model.

### Specialized Tool Shelf

![][ImgTorusToolIcon]{{rightimage(60)}}
![][ImgTextToolIcon]{{rightimage(60)}}
![][ImgRevSurfToolIcon]{{rightimage(60)}}
![][ImgMirrorToolIcon]{{rightimage(60)}}
![][ImgImportToolIcon]{{rightimage(60)}}
![][ImgCylinderToolIcon]{{rightimage(60)}}
![][ImgClipToolIcon]{{rightimage(60)}}
![][ImgCSGToolIcon]{{rightimage(60)}}
![][ImgBevelToolIcon]{{rightimage(60)}}

There is a single icon on this shelf at any time, which implements the
![][ImgNullIcon]{{rightimage(60)}}
{{term('Specialized-Tool')}} toggle action. This [toggles between using the
current general tool and the specialized tool][ToolSwitching] for the primary
selection, if any. When a specialized tool is available, the icon changes form
to one of these icons to show which tool will be used. Otherwise, the tools
shows a null sign icon and is disabled.

## Back Wall Shelves

These shelves are on the back wall of the [work area][WorkArea]. They are
listed from top to bottom.

### Basics Shelf
![][ImgBasicsShelf]{{rightimage(80)}}

Icons on this shelf apply standard actions you would expect to find in an
interactive application. They are listed here from left to right.

+ {{anchor('undo-action')}}The {{term('Undo')}} action undoes the last
  operation performed that modified the scene in some way.
+ {{anchor('redo-action')}}The {{term('Redo')}} action redoes the last undone
  action.
+ {{anchor('delete-action')}}The {{term('Delete')}} action deletes all selected
  models from the scene. They are gone forever, unless you undo.
+ {{anchor('cut-action')}}The {{term('Cut')}} action copies all selected models
  from the scene to the clipboard and removes them from the scene.
+ {{anchor('copy-action')}}The {{term('Copy')}} action copies all selected
  models from the scene into the clipboard.
+ {{anchor('paste-action')}}The {{term('Paste')}} action pastes all models from
  the clipboard into the scene as top-level models at their previous 3D
  locations.
+ {{anchor('paste-into-action')}}The {{term('Paste-Into')}} action can be used
  to paste models from the clipboard as children of the [currently-selected
  model][Selection], as long as that model is one that can have children. For
  example, if you have a [CSG union model][CombinedModels] and you want to add
  another piece (say, a box) to it, you can cut or copy the box (in the correct
  position, of course), select the CSG model, and click the Paste-Into
  icon. The box will be added as a child of the union.

Note that Undo/Redo actions apply to these operations:

+ Any change to a model or models.
+ Changing the position or any other aspect of either of the
  [targets][Targets].

Actions that are *not* considered changes and are therefore not undoable
include:

+ Changing the view or the [stage][Stage] orientation or scale.
+ [Selecting or deselecting models][Selection]. (However, undoing other actions
  may affect the current selection.)
+ Toggling any program state, such as target or edge visibility.
+ Using the [Copy action][CopyAction], since it has no visible effect except to
  update the clipboard. Note that you may be able to [use this to your
  advantage][CopyQuirk].

Hovering over the Undo or Redo icon shows a [tooltip][Tooltips] indicating what
operation will be undone or redone.

### Layout Shelf
![][ImgLayoutShelf]{{rightimage(80)}}

This shelf contains icons for operations that can help lay out or position
models with respect to each other. They are, from left to right:

+ {{anchor('point-target-toggle')}}The {{term('Point-Target')}} toggle action
  changes the visibility of the [point target][PointTarget].
+ {{anchor('edge-target-toggle')}}The {{term('Edge-Target')}} toggle action
  changes the visibility of the [edge target][EdgeTarget].
+ {{anchor('linear-layout-action')}}The {{term('Linear-Layout')}} action uses
  the [edge target][EdgeTarget] to lay out selected models along a line. See
  the section on [linear layout][LinearLayout] for details.
+ {{anchor('radial-layout-action')}}The {{term('Radial-Layout')}} action uses
  the [point target][PointTarget] to lay out selected models along a circular
  arc or to move a single model to the target. See the section on [radial
  layout][RadialLayout] for details.
+ {{anchor('axis-aligned-toggle')}}The {{term('Axis-Aligned')}} toggle action
  changes whether certain tools operate in the selected model's local
  coordinates or in [stage][Stage] coordinates. For example, if a model has
  been rotated to an arbitrary orientation and you want to move it 5 units to
  the right in the scene, you can turn on axis-alignment and the [Translation
  Tool][TranslationTool] will then be aligned with the stage axes, not the
  model's local axes. This toggle affects the [Clip Tool][ClipTool], [Mirror
  Tool][MirrorTool], [Rotation Tool][RotationTool], and [Translation
  Tool][TranslationTool].

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
+ {{anchor('move-to-origin-action')}}The {{term('Move-to-Origin')}} action
  moves the primary selection so that it is at the origin (center of the
  [stage][Stage]). Any secondary selections are moved by the same amount.
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
