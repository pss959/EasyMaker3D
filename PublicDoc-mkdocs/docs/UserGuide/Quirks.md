# Quirks

Every application has some quirks that may need extra explanation. So far,
there is really only one for MakerVR.

## Copy, Paste, Undo, Redo

The interactions between copy and paste operations, coupled with undo and redo,
are generally intuitive, but may require some explanation. Here are the basic
rules:

+ Copying a model to the clipboard creates a clone of the model, meaning that
  any changes to the original model will not affect the copy.
+ Pasting a model from the clipboard also creates a clone, so that changes to
  the pasted model will not affect the model in the clipboard or any other
  models created by pasting it at other times.
+ Pasted clones will have the same name as the original model in the clipboard,
  but with a letter appended to it after an underscore (such as `"Box_1_A"`).
+ Undoing and redoing a paste operation always pastes the exact models that
  were pasted the first time.
+ Undoing and redoing any commands have no effect on the contents of the
  clipboard. Note that you can use this feature to your advantage if you want
  to undo back to an earlier state but still have another model available to
  paste into the scene.
