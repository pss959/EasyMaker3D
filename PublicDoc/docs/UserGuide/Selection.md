# Selection

To select a model, click on it with the mouse or pinch action, and the
appropriate editing [tool][Tools] will be attached to it.

Multiple models can be selected by [alt-clicking][AltMode] on them.
Alt-clicking an unselected model adds it as a selection, while alt-clicking a
selected model deselects it.

The [Tree Panel][TreePanel] can also be used to view and change the current
selection.

## Primary and Secondary Selections

![Primary and secondary selections][ImgSelection]{{rightimage(100)}}

When more than one model is selected, the first selected model is known as the
{{term('primary selection')}}, and all other selected models form the
{{term('secondary selections')}}. The current editing tool is attached to only
the primary selection. All secondary selections are indicated with a
noninteractive blue wireframe box. Some tools and actions treat the primary and
secondary selections differently.

## Parent and Child Selection

A model that is created from other models adds the original models as children
and hides them. This is true for both [Converted Models][ModelTypes] and
[Combined Models][ModelTypes].

You can make the child models visible by selecting them, using either the [Tree
Panel][TreePanel] or the following [keyboard shortcuts][Shortcuts]:

+ `Ctrl-DownArrow` (the [Select-First-Child action][SelectFirstChildAction])
  selects the first child of a parent model.
+ `Ctrl-LeftArrow` (the [Select-Previous-Sibling
  action][SelectPreviousSiblingAction])selects the previous sibling of a child
  or a top-level model, wrapping around as necessary.
+ `Ctrl-RightArrow` (the [Select-Next-Sibling
  action][SelectNextSiblingAction])selects the next sibling of a child or a
  top-level model, wrapping around as necessary.
+ `Ctrl-UpArrow` (the [Select-Parent action][SelectParentAction]) selects the
  parent of a child model.

In addition, `Ctrl-A` (the [Select-All action][SelectAllAction]) selects all
top-level models and `Shift-Ctrl-A` (the [Select-None
action][SelectNoneAction]) deselects everything. You can also deselect
everything by clicking on a wall or anywhere else in the work area that is not
interactive. (Keep in mind the [stage][Stage] is interactive, so it doesn't
count.)
