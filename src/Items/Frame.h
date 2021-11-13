#pragma once

#include <memory>

#include "SG/Node.h"

namespace Parser { class Registry; }

/// The Frame class implements a frame (like a picture frame) around another
/// object. The sizes of the non-resizing dimensions are parsed fields.  If the
/// "framed" field is set to an object, this sizes itself automatically to
/// match its bounds. If it is null, sizing must be done manually with
/// FitToSize().
class Frame : public SG::Node {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void PreSetUpIon() override;

    /// Sets the frame to fit the given 2D size.
    void FitToSize(const Vector2f &size) const;

    /// Returns the framed object, which may be null.
    const SG::NodePtr & GetFramed() const { return framed_; }

  protected:
    Frame() {}

    /// Redefines this to resize to match the bounds of the framed object, if
    /// any.
    virtual Bounds UpdateBounds() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>         width_{"width"};
    Parser::TField<float>         depth_{"depth"};
    Parser::ObjectField<SG::Node> framed_{"framed"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<Frame> FramePtr;
