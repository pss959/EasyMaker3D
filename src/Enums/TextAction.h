//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// The TextAction enum represents different actions that are supported by the
/// VirtualKeyboard and TextInputPane when inserting and editing text.
///
/// \ingroup Enums
enum class TextAction {
    ///\name Insertion.
    ///@{
    kInsert,          ///< Insert text characters.
    kToggleShift,     ///< Toggle the shift state of the VirtualKeyboard.
    ///@}

    /// \name Deletion.
    ///@{
    kDeleteAll,       ///< Delete all text.
    kDeleteNext,      ///< Delete the character after the cursor.
    kDeletePrevious,  ///< Delete the character before the cursor.
    kDeleteSelected,  ///< Delete selection or character before cursor.
    kDeleteToEnd,     ///< Delete from the cursor to the end of the text.
    kDeleteToStart,   ///< Delete from the start of the text to the cursor.
    ///@}

    /// \name Cursor Motion.
    ///@{
    kMoveNext,        ///< Move the cursor to the next character.
    kMovePrevious,    ///< Move the cursor to the previous character.
    kMoveToEnd,       ///< Move the cursor to the end of the text.
    kMoveToStart,     ///< Move the cursor to the start of the text.
    ///@}

    /// \name Selection Changes.
    ///@{
    kSelectAll,       ///< Select all text.
    kSelectNext,      ///< Add the next character to the selection.
    kSelectNone,      ///< Deselect any selected text.
    kSelectPrevious,  ///< Add the previous character to the selection.
    kSelectToEnd,     ///< Extend the selection to the end of the text.
    kSelectToStart,   ///< Extend the selection to the start of the text.
    ///@}

    /// \name Undo/redo.
    ///@{
    kUndo,            ///< Revert the last edit.
    kRedo,            ///< Redo the last undone edit.
    ///@}

    /// \name Completion.
    ///@{
    kAccept,          ///< Accept the edited text.
    kCancel,          ///< Restore the text prior to editing.
    ///@}
};
