# Tree Panel

![][ImgTreePanel]{{rightimage(160)}}

The Tree Panel is the framed board on the back wall. It has multiple uses:

+ Displaying a list of all models in the scene, including their tree structure
  and current status.
+ Selecting models.
+ Toggling model visibility.
+ Editing model names.
+ Changing order of models.

## Session Name and Status

The current name of the session being edited is shown at the top of the panel.
If the session has not yet been saved with a name, it will show as
`<Untitled>`.

If the session has been modified since it was loaded, there will be a
modification string within square brackets after the session name.  This string
may contain the following characters:

+ An asterisk (`'*'`) indicates that one or more models has been created or
  modified and not undone.
+ An exclamation point (`'!'`) indicates that the session state (such as edge
  visibility or bound volume visibility) has changed.
+ A plus sign (`'+'`) indicates that the set of commands in the session file
  has changed.

Note that a session can be saved if any of these symbols appears. For example,
if you make changes to models and then undo back to their original state, there
will be no asterisk, but the plus sign lets you know that the session can still
be saved (because the sequence of commands has changed). The session state is
also saved, so changing it allows you to save as well.

## Model Names

Each row under the top row of the panel represents a model in the scene. Any
model that has children will show those children as indented names under its
name. There will also be a triangle that you can click or pinch to collapse or
expand the children. Clicking on a model name [selects that
model](#selecting-models).

## Text Colors

Each model displayed in the Tree Panel is shown with its name in color-coded
text to indicate its status:

+ The <span style="color:rgb(212,20,20)">primary selection is red</span>.
+ All <span style="color:rgb(0,0,254)">secondary selections are blue</span>.
+ All <span style="color:rgb(128,10,117)">models with visibility off are
  purple</span>.
+ All <span style="color:rgb(102,102,102)">children hidden because their
  parents are visible are gray</span>.
+ All <span style="color:rgb(0,0,0)">other models are black</span>.

## Font Size

The slider on the right of the Tree Panel changes the font size of the text and
the sizes of all of the buttons.  This can be useful if you have a lot of
models and want to see more of them or to use [rectangle
selection][TreePanelSelection] on more names.

## Changing Visibility

Each model name in the Tree Panel has a little eye icon to its left. Clicking
this icon toggles the visibility of the corresponding model in the scene.

There is also an eye icon next to the session name at the top. If any model is
currently hidden, clicking this will show all models (the [Show All
action][ShowAllAction]. Otherwise, it will hide all models..

## Selecting Models

Clicking with the mouse or pinch on a model name selects it as the primary
selection. [Alt-clicking][AltMode] on a model name toggles its selection
status.

You can also use the mouse or pinch to drag out a selection rectangle in the
panel. All model names intersected by the rectangle will be selected. [Changing
the order of names][ReorderingModels] may help in some cases.

Note that you cannot have both a parent and child model selected at the same
time. Allowing this would be horrendous.

## Changing Model Names

![][ImgEditName]{{rightimage(32)}}

MakerVR creates names for new models using the type of model and a numerical
suffix to guarantee uniqueness. You can use the edit button on the top right of
the panel to bring up a text editing field that lets you [edit][TextInput] the
current name of the model. It will not let you create a duplicate name.

## Reordering Models

![][ImgMoveUpDown]{{rightimage(64)}}

There are some cases where the order of models in the Tree Panel matters. One
is when you want to use [rectangle selection][TreePanelSelection] to select
some set of models. Another is when the order of children within a parent
matters, such as within a [CSG difference][CSG]. In these cases the move-up
([Move Previous action][MovePreviousAction]) and move-down ([Move Next
action][MoveNextAction]) buttons can be used to move the primary selection up
and down in the list. Note that if the primary selection is not a top-level
model, it can only be moved up and down within its parent model.
