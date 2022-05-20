#pragma once

#include "Base/Memory.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ImagePane);

/// ImagePane is a derived LeafPane that displays an image.
class ImagePane : public LeafPane {
  public:
    virtual std::string ToString() const override;

  protected:
    ImagePane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};
