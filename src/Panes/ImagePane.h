#pragma once

#include "Memory.h"
#include "Panes/Pane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ImagePane);

/// ImagePane is a derived Pane that displays an image.
class ImagePane : public Pane {
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
