#pragma once

#include <memory>

#include "SG/Node.h"

namespace Parser { class Registry; }

/// The Frame class implements a frame (like a picture frame) around another
/// object. The sizes of the non-resizing dimensions are parsed fields.
class Frame : public SG::Node {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Sets the frame to fit the given 2D size.
    void FitToSize(const Vector2f &size) const;

  protected:
    Frame() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> width_{"width"};
    Parser::TField<float> depth_{"depth"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<Frame> FramePtr;
