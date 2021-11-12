#pragma once

#include <memory>

#include "SG/Node.h"

namespace Parser { class Registry; }

/// The Frame class implements a frame (like a picture frame) around another
/// object. It sizes itself automatically to match the bounds of the framed
/// object. The sizes of the non-resizing dimensions are parsed fields.
class Frame : public SG::Node {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void PreSetUpIon() override;

  protected:
    Frame() {}

    /// Redefines this to resize to match the child bounds.
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
