//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Frame);

/// The Frame class implements a frame (like a picture frame) around another
/// object. The sizes of the non-resizing dimensions are parsed fields.  If the
/// "framed" field is set to an object, this sizes itself automatically to
/// match its bounds. If it is null, sizing must be done manually with
/// FitToSize().
///
/// \ingroup Items
class Frame : public SG::Node {
  public:
    ~Frame();

    /// Sets the frame to fit the given 2D size.
    void FitToSize(const Vector2f &size) const;

    /// Returns the framed object, which may be null.
    const SG::NodePtr & GetFramed() const { return framed_; }

  protected:
    Frame() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to resize to match the bounds of the framed object, if
    /// any.
    virtual Bounds UpdateBounds() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>         width_;
    Parser::TField<float>         depth_;
    Parser::ObjectField<SG::Node> framed_;
    ///@}

    friend class Parser::Registry;
};
