#pragma once

/// The TextAction enum represents different actions that are supported by the
/// TextInputPane when editing text.
enum class TextAction {
    kClear,           ///< Clear (delete) all text.
    kDeleteNext,      ///< Delete the character after the cursor.
    kDeletePrevious,  ///< Delete the character before the cursor.
    kDeleteToEnd,     ///< Delete from the cursor to the end of the text.
    kDeleteToStart,   ///< Delete from the start of the text to the cursor.
    kMoveNext,        ///< Move the cursor to the next character.
    kMovePrevious,    ///< Move the cursor to the previous character.
    kMoveToEnd,       ///< Move the cursor to the end of the text.
    kMoveToStart,     ///< Move the cursor to the start of the text.
    kRedo,            ///< Redo the last undone edit.
    kSelectAll,       ///< Select all text.
    kSelectNext,      ///< Add the next character to the selection.
    kSelectNone,      ///< Deselect any selected text.
    kSelectPrevious,  ///< Add the previous character to the selection.
    kSelectToEnd,     ///< Extend the selection to the end of the text.
    kSelectToStart,   ///< Extend the selection to the start of the text.
    kUndo,            ///< Revert the last edit.
};
