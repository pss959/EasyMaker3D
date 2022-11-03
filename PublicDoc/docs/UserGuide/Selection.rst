.. _ug-selection:

Selecting Models
----------------

The easiest way to select a model is to click on it with the mouse or pinch
action. Selecting a model attaches the appropriate :ref:`tool <ug-tools>` to
it for editing.

Multiple models can be selected by :ref:`modified-clicking <ug-modified-mode>`
them. Modified-clicking an unselected model adds it as a selection, and
modified-clicking a selected model deselects it.

The :ref:`Tree Panel <ug-tree-panel>` can also be used to view and change the
current selection.

Primary and Secondary Selections
................................

.. incimage:: /images/Selection.jpg 300px right

When more than one model is selected, the first selected model is known as the
:newterm:`primary selection` and all other selected models are the
:newterm:`secondary selections`. The current editing tool is attached to only
the primary selection. All secondary selections are indicated with a
noninteractive blue wireframe box as shown here. Some tools and actions treat
the primary and secondary selections differently.

.. _ug-select-hierarchy:

Parent and Child Selection
..........................

A model that is created from other models adds the original models as children
and hides them. This is true for both :ref:`converted models
<ug-converted-models>` and :ref:`combined models <ug-combined-models>`.

You can make the child models visible by selecting them, using either the
:ref:`Tree Panel <ug-tree-panel>` or by using the following :ref:`keyboard
shortcuts <ug-keyboard-shortcuts>`:

  - :shortcut:`Ctrl-Down` (the :ref:`Select FirstChild <ug-select-first-child>`
    action) selects the first child of the selected parent model.
  - :shortcut:`Ctrl-Left` (the :ref:`Select Previous Sibling
    <ug-select-previous-sibling>` action) selects the previous sibling of the
    selected child or top-level model, wrapping around as necessary.
  - :shortcut:`Ctrl-Right` (the :ref:`Select Next Sibling
    <ug-select-next-sibling>` action) selects the next sibling of the
    selected child or top-level model, wrapping around as necessary.
  - :shortcut:`Ctrl-Up` (the :ref:`Select Parent <ug-select-parent>` action)
    selects the parent of a child model.

In addition, the :shortcut:`Ctrl-A` shortcut (the :ref:`Select All
<ug-select-all>` action) selects all top-level models and the
:shortcut:`Shift-Ctrl-A` shortcut (the :ref:`Select None <ug-select-none>`
action) deselects all selected models. You can also deselect everything by
clicking on a wall or anywhere else in the :ref:`work area <ug-work-area>` that
is not interactive. Keep in mind the :ref:`stage <ug-stage>` is interactive, so
clicking on it does not change the selection.
