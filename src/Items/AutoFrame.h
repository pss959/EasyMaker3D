#pragma once

#include <memory>

#include "Items/Frame.h"

namespace Parser { class Registry; }

/// AutoFrame is a derived Frame that sizes itself automatically to match the
/// bounds of a framed object.
class AutoFrame : public Frame {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void PreSetUpIon() override;

    /// Allows the framed object to be set during runtime.
    void SetFramed(const SG::NodePtr &framed);

    /// Returns the framed object. This should never be null.
    const SG::NodePtr & GetFramed() const { return framed_; }

  protected:
    AutoFrame() {}

    /// Redefines this to resize to match the child bounds.
    virtual Bounds UpdateBounds() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<SG::Node> framed_{"framed"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<AutoFrame> AutoFramePtr;
