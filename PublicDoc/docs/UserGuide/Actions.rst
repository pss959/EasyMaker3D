.. _ug-actions:

Actions, 3D Icons, and Shelves
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
:ref:`Cheat Sheet <cheat-sheet>` lists all supported actions along with their
names, radial menu icons, and keyboard/controller shortcuts. The actions are
divided into categories, some of which correspond to shelves in the work area.

.. note::

   Some actions do not have corresponding 3D icons; they can be performed only
   from a :ref:`radial menu <ug-radial-menus>` or :ref:`keyboard shortcut
   <ug-keyboard-shortcuts>`.

Certain actions may not be enabled due to the current application state. A 3D
icon for an enabled action will appear with the regular light woodgrain
texture. An icon for a disabled action will be dark instead. 3D icons that
represent enabled toggle actions will be colored green when the toggle is in
the "on" state and regular woodgrain when in the "off" state.

The following sections detail what each action does in the application, divided
by category.

.. _ug-copy:
.. _ug-cut:
.. _ug-delete:
.. _ug-duplicate:
.. _ug-paste-into:
.. _ug-paste:
.. _ug-quit:
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

.. note::

   Undo and Redo actions apply to only these operations:

    - Any change to a model or models.
    - Changing the position or any other aspect of either of the :ref:`targets
      <ug-targets>`.

Actions that are :emphasis:`not` considered changes and are therefore not
undoable include:

  - Changing the view.
  - Changing the orientation or size of the :ref:`stage <ug-stage>`.
  - :ref:`Selecting or deselecting models <ug-selection>`. (However, undoing or
    redoing other actions may affect the current selection.)
  - Toggling any program state, such as target or edge visibility.
  - Applying the Copy action, since it has no visible effect except to update
    the clipboard. Note that you may be able to :ref:`use this to your
    advantage <ug-copy-quirk>`.

Hovering over the Undo or Redo icon (when enabled) shows a :ref:`tooltip
<ug-tooltips>` describing what operation will be undone or redone.

There are additional actions that are not attached to 3D icons, but are 
avalable via keyboard shortcuts and radial menus.

  - The :action:`Duplicate` action is equivalent to a Copy action followed by a
    Paste action.
  - The :action:`Quit` action exits the program after checking for unsaved
    changes. This action is also available by clicking on the exit sign in the
    :ref:`work area <ug-work-area>`.

.. _ug-create-box:
.. _ug-create-cylinder:
.. _ug-create-extruded:
.. _ug-create-imported-model:
.. _ug-create-rev-surf:
.. _ug-create-sphere:
.. _ug-create-text:
.. _ug-create-torus:

.. _ug-combine-csg-difference:
.. _ug-combine-csg-intersection:
.. _ug-combine-csg-union:
.. _ug-combine-hull:

Combination Actions
...................

.. incimage:: /images/ModelCombinationShelf.jpg 240px right

Actions to create a :ref:`combined model <ug-combined-models>` from the
:ref:`selected models <ug-selection>` are available as 3D icons on the second
shelf from the top on the left wall in the :ref:`work area
<ug-work-area>`.

The following table lists the icons from left to right and what they do. The
specialized tool links describe how to edit the resulting models.

.. list-table::
   :align:  center
   :widths: auto
   :header-rows: 1

   * - Action
     - Model Type
     - Specialized Tool
     - Combination Effects
   * - :action:`Combine CSG Difference`
     - :ref:`CSG model <ug-combined-models>`
     - :ref:`CSG Tool <ug-csg-tool>`
     - All secondary selections subtracted from the primary selection
   * - :action:`Combine CSG Intersection`
     - :ref:`CSG model <ug-combined-models>`
     - :ref:`CSG Tool <ug-csg-tool>`
     - Intersection of all selected models
   * - :action:`Combine CSG Union`
     - :ref:`CSG model <ug-combined-models>`
     - :ref:`CSG Tool <ug-csg-tool>`
     - Union of all selected models
   * - :action:`Combine Hull`
     - :ref:`Hull model <ug-combined-models>`
     - None (current :ref:`general tool <ug-general-tools>` is attached)
     - :ref:`Convex hull <ug-hull>` of all selected models

The CSG actions require at least two models to be selected, but the convex hull
action can be applied to a single model.

.. _ug-convert-bend:
.. _ug-convert-bevel:
.. _ug-convert-clip:
.. _ug-convert-mirror:
.. _ug-convert-taper:
.. _ug-convert-twist:

Conversion Actions
..................

.. incimage:: /images/ModelConversionShelf.jpg 200px right

The third shelf from the top on the left wall of the :ref:`work area
<ug-work-area>` has 3D icons that apply actions to create :ref:`converted
models <ug-converted-models>` from the :ref:`selected models
<ug-selection>`. Applying any of these actions converts all selected models to
a specific type and then attaches the corresponding :ref:`specialized tool
<ug-specialized-tools>` to the primary selection.

The following table lists the icons from left to right and what they do. The
specialized tool links describe how to edit the resulting models. See those
tools for more information.

.. list-table::
   :align:  center
   :widths: auto
   :header-rows: 1

   * - Action
     - Model Type
     - Specialized Tool
     - Initial State after Conversion
   * - :action:`Convert Bend`
     - :ref:`Bent model <ug-converted-models>`
     - :ref:`Bend Tool <ug-bend-tool>`
     - No bend (0 degrees)
   * - :action:`Convert Bevel`
     - :ref:`Beveled model <ug-converted-models>`
     - :ref:`Bevel Tool <ug-bevel-tool>`
     - Simple (straight) bevel on all edges
   * - :action:`Convert Clip`
     - :ref:`Clipped model <ug-converted-models>`
     - :ref:`Clip Tool <ug-clip-tool>`
     - Top half of primary model clipped away; other models clipped by same
       plane
   * - :action:`Convert Mirror`
     - :ref:`Mirrored model <ug-converted-models>`
     - :ref:`Mirror Tool <ug-mirror-tool>`
     - Primary model mirrored left-to-right across center; other models
       mirrored across same plane
   * - :action:`Convert Taper`
     - :ref:`Tapered model <ug-converted-models>`
     - :ref:`Taper Tool <ug-taper-tool>`
     - Taper from 0 to 1 along the Y axis
   * - :action:`Convert Twist`
     - :ref:`Twisted model <ug-converted-models>`
     - :ref:`Twist Tool <ug-twist-tool>`
     - No twist (0 degrees)

Each converted model applies a single operation. To apply another one, you can
convert the model again. For example to clip a Cylinder model with multiple
planes, you can convert the cylinder to a Clipped model and edit the first
plane with the Clip Tool. Then you can convert the Clipped model again to
create another Clipped model and edit the second plane. For consistency, this
process works for all types of converted models, although it doesn't always
make sense.  (Beveling an already Beveled model probably results in something
awful.)

Creation Actions
................

Model Creation actions are all available as 3D icons on the two shelves on the
bottom of the left wall of the :ref:`work area <ug-work-area>`. Each creates
:ref:`a model of a specific type <ug-model-types>`

.. incimage:: /images/ModelCreationShelf0.jpg 300px right

From left to right on the bottom shelf, the icons/actions are:

  - Box model (:action:`Create Box`)
  - Cylinder model (:action:`Create Cylinder`)
  - Sphere model (:action:`Create Sphere`)
  - Torus model (:action:`Create Torus`)

.. incimage:: /images/ModelCreationShelf1.jpg 300px right

From left to right on the next shelf up, the icons/actions are:

  - Surface of revolution model (:action:`Create Rev Surf`)
  - Extruded model (:action:`Create Extruded`)
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
the middle shelf on the back wall. From left to right:

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
    stage axes, not the model's local axes. This toggle affects these
    tools: :ref:`Bend Tool <ug-bend-tool>`, :ref:`Clip Tool <ug-clip-tool>`,
    :ref:`Mirror Tool <ug-mirror-tool>`, :ref:`Rotation Tool
    <ug-rotation-tool>`, :ref:`Translation Tool <ug-translation-tool>`, and
    :ref:`Twist Tool <ug-twist-tool>`.

One other layout-related action is available only via a :ref:`keyboard shortcut
<ug-keyboard-shortcuts>` or :ref:`radial menu <ug-radial-menus>`:

  - The :action:`Move To Origin` action translates the :ref:`primary selection
    <ug-selection>` so that the center of its bounds is centered over the
    origin (center of the :ref:`stage <ug-stage>`) without rotating and the
    lowest point of its bounds is resting on the stage.  Any secondary
    selections are moved by the same amount.

.. _ug-decrease-complexity:
.. _ug-increase-complexity:
.. _ug-move-next:
.. _ug-move-previous:

Modification Actions
....................

Certain actions that immediately modify the currently selected models are
available only via :ref:`keyboard shortcuts <ug-keyboard-shortcuts>` or
:ref:`radial menus <ug-radial-menus>`:

  - The :action:`Increase Complexity` and :action:`Decrease Complexity` actions
    increase or decrease the complexity of :ref:`all selected models
    <ug-selection>` by .05 (within the 0-1 range). This can be a quicker way to
    modify complexity than by using the :ref:`Complexity Tool
    <ug-complexity-tool>`.
  - The :action:`Move Next` and :action:`Move Previous` actions can be used to
    change the order of top-level models or child models within the same parent
    model. These actions are also available with buttons in the :ref:`Tree
    Panel <ug-tree-panel>`.

.. _ug-decrease-precision:
.. _ug-increase-precision:

Precision Actions
.................

The current :ref:`precision level <ug-precision-level>` can be changed with the
:action:`Increase Precision` and :action:`Decrease Precision` actions. These
are available via :ref:`keyboard shortcuts <ug-keyboard-shortcuts>`,
:ref:`radial menus <ug-radial-menus>`, or by using the :ref:`Precision Control
<ug-precision-control>` on the back wall of the :ref:`work area
<ug-work-area>`.

.. _ug-toggle-left-radial-menu:
.. _ug-toggle-right-radial-menu:

Radial Menu Actions
...................

The :action:`Toggle Left Radial Menu` and :action:`Toggle Right Radial Menu`
actions are available from via :ref:`keyboard and controller shortcuts
<ug-keyboard-shortcuts>`. They show or hide the :ref:`radial menu
<ug-radial-menus>` for the corresponding hand.

.. _ug-select-all:
.. _ug-select-first-child:
.. _ug-select-next-sibling:
.. _ug-select-none:
.. _ug-select-parent:
.. _ug-select-previous-sibling:

Selection Actions
.................

The following actions that modify the :ref:`current selection <ug-selection>`
are available only via :ref:`keyboard shortcuts <ug-keyboard-shortcuts>` or
:ref:`radial menus <ug-radial-menus>`.

  - The :action:`Select All` action selects all top-level models (in order). If
    any models are already selected, they remain selected and all unselected
    top-level models are added as additional secondary selections.
  - The :action:`Select None` action deselects all selected models.
  - The :action:`Select First Child`, :action:`Select Next Sibling`,
    :action:`Select Previous Sibling`, and :action:`Select Parent`, actions can
    be used to :ref:`change selections through model hierarchies
    <ug-select-hierarchy>`.

.. note::

   The :ref:`Tree Panel <ug-tree-panel>` can be used to view and change the
   current selection, especially within a model hierarchy.

.. _ug-open-help-panel:
.. _ug-open-info-panel:
.. _ug-open-session-panel:
.. _ug-open-settings-panel:

Session Actions
...............

.. incimage:: /images/SessionShelf.jpg 240px right

The following four actions are available as 3D icons on the Session shelf,
which is the bottom shelf on the back wall. From left to right:

  - The :action:`Open Session Panel` action displays the :ref:`Session Panel
    <ug-session-panel>`, which allows you to save your session, load a new
    session, and so on.
  - The :action:`Open Settings Panel` action displays the :ref:`Settings Panel
    <ug-settings-panel>`, which allows you to edit application settings.
  - The :action:`Open Info Panel` action displays the :ref:`Info Panel
    <ug-info-panel>`, which shows information about currently selected
    models and the targets.
  - The :action:`Open Help Panel` action displays the :ref:`Help Panel
    <ug-help-panel>`, which displays the application version and has buttons to
    open this guide or the :ref:`Cheat Sheet <cheat-sheet>` in your default
    browser.

.. _ug-toggle-specialized-tool:

Specialized Action
..................

The 3D icon on the top shelf on the left wall toggles between the
:ref:`specialized tool <ug-specialized-tools>` for the :ref:`current selection
<ug-selection>` and the current :ref:`general tool <ug-general-tools>`. This
icon changes shape to reflect what the toggle will do.

.. incimage:: /images/NullIcon.jpg 80px right

When no model is selected or if no specialized tool can be applied to the
current selection, the :newterm:`null icon` is shown. This icon is always
disabled.

|block-image|

If a specialized tool can be used for the current selection, this icon changes
to one of the ones shown here. Clicking on the icon applies the :action:`Toggle
Specialized Tool` action to switch between the specialized tool and the current
general tool. The icon will be shown with the active icon color when the
corresponding specialized tool is attached and the regular woodgrain icon color
when toggled back to a general tool.

.. incimage:: /images/TwistToolIcon.jpg    64px right
.. incimage:: /images/TorusToolIcon.jpg    64px right
.. incimage:: /images/TextToolIcon.jpg     64px right
.. incimage:: /images/TaperToolIcon.jpg    64px right
.. incimage:: /images/RevSurfToolIcon.jpg  64px right
.. incimage:: /images/MirrorToolIcon.jpg   64px right
.. incimage:: /images/ImportToolIcon.jpg   64px right
.. incimage:: /images/CylinderToolIcon.jpg 64px right
.. incimage:: /images/ClipToolIcon.jpg     64px right
.. incimage:: /images/CSGToolIcon.jpg      64px right
.. incimage:: /images/BevelToolIcon.jpg    64px right
.. incimage:: /images/BendToolIcon.jpg    64px right

|block-image|

The following table explains the above icons from left to right. For each icon,
the table lists the specialized tool the icon switches to and the type of model
it can be attached to. Note that all selected models must be of the appropriate
type for the icon to be enabled.

.. list-table::
   :align:  center
   :widths: auto
   :header-rows: 1

   * - Icon/Specialized Tool
     - Required Selected Model Type
   * - :ref:`Bend Tool <ug-bend-tool>`
     - :ref:`Bent models <ug-converted-models>`
   * - :ref:`Bevel Tool <ug-bevel-tool>`
     - :ref:`Beveled models <ug-converted-models>`
   * - :ref:`CSG Tool <ug-csg-tool>`
     - :ref:`CSG models <ug-combined-models>`
   * - :ref:`Clip Tool <ug-clip-tool>`
     - :ref:`Clipped models <ug-converted-models>`
   * - :ref:`Cylinder Tool <ug-cylinder-tool>`
     - :ref:`Cylinder models <ug-primitive-models>`
   * - :ref:`Import Tool <ug-import-tool>`
     - (One) :ref:`Imported model <ug-primitive-models>`
   * - :ref:`Mirror Tool <ug-mirror-tool>`
     - :ref:`Mirrored models <ug-converted-models>`
   * - :ref:`Rev Surf Tool <ug-rev-surf-tool>`
     - :ref:`RevSurf models <ug-primitive-models>`
   * - :ref:`Taper Tool <ug-taper-tool>`
     - :ref:`Tapered models <ug-converted-models>`
   * - :ref:`Text Tool <ug-text-tool>`
     - :ref:`Text models <ug-primitive-models>`
   * - :ref:`Torus Tool <ug-torus-tool>`
     - :ref:`Torus models <ug-primitive-models>`
   * - :ref:`Twist Tool <ug-twist-tool>`
     - :ref:`Twisted models <ug-converted-models>`

.. note::

   The :shortcut:`Space` shortcut is an easy way to toggle between the current
   general and specialized tools when a specialized tool is available.

.. _ug-switch-to-next-tool:
.. _ug-switch-to-previous-tool:
.. _ug-tool-actions:

General Tool Actions
....................

.. incimage:: /images/GeneralToolShelf.jpg 300px right

Actions to choose the current :ref:`general tool <ug-general-tools>` are
available as 3D icons on the third shelf from the bottom on the left wall of
the :ref:`work area <ug-work-area>`. Clicking on any of these icons attaches
the corresponding general tool to the :ref:`primary selection
<ug-selection>`.

.. note::

    Most of these tools also affect all :ref:`secondary
    selections<ug-selection>` in addition to the primary model.

The following table lists the general tool icons from left to right.

.. list-table::
   :align:  center
   :widths: auto
   :header-rows: 1

   * - Action/General Tool
     - Tool Effects
   * - :ref:`Name Tool <ug-name-tool>`
     - Edits the name of the primary selection
   * - :ref:`Color Tool <ug-color-tool>`
     - Changes the color of all selected models
   * - :ref:`Complexity Tool <ug-complexity-tool>`
     - Changes the tessellation of all selected models\*
   * - :ref:`Rotation Tool <ug-rotation-tool>`
     - Rotates all selected models
   * - :ref:`Scale Tool <ug-scale-tool>`
     - Changes the size of all selected models
   * - :ref:`Translation Tool <ug-translation-tool>`
     - Changes the position of all selected models

.. note::

   \*Only :ref:`certain model types<ug-complexity-model-types>` respond to
   Complexity Tool changes; the action and tool icon are disabled if only other
   types of models are selected.

In addition, the :action:`Switch To Next Tool` and :action:`Switch To Previous
Tool` actions are available via :ref:`keyboard and controller shortcuts
<ug-keyboard-shortcuts>` to quickly cycle through the available general tools
for the current selection.

.. _ug-hide-selected:
.. _ug-show-all:
.. _ug-toggle-build-volume:
.. _ug-toggle-inspector:
.. _ug-toggle-show-edges:

Viewing Actions
...............

.. incimage:: /images/ViewingShelf.jpg 200px right

The 3D icons on the Viewing shelf (above the :ref:`Tree Panel <ug-tree-panel>`
on the back wall) apply actions that deal with viewing models. From left to
right:

  - The :action:`Toggle Show Edges` action lets you show or hide :ref:`model
    edges <ug-show-edges>` as visible lines.
  - The :action:`Toggle Build Volume` action lets you show or hide the
    translucent representation of your 3D printer's :ref:`build volume
    <ug-build-volume>`.
  - The :action:`Toggle Inspector` action lets you turn on :ref:`inspector mode
    <ug-inspector-mode>` for the current :ref:`primary selection
    <ug-selection>`.

Other viewing actions are available only via :ref:`keyboard shortcuts
<ug-keyboard-shortcuts>` or :ref:`radial menus <ug-radial-menus>`:

  - The :action:`Hide Selected` action makes all currently selected top-level
    models temporarily invisible.
  - The :action:`Show All` action makes all top-level models visible again.

.. note::

   The :ref:`Tree Panel <ug-tree-panel>` can also be used to hide or show
   models.
