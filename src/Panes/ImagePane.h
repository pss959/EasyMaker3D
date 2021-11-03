#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// ImagePane is a derived Pane that displays an image.
class ImagePane : public Pane {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    virtual void PreSetUpIon() override;

  protected:
    ImagePane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ImagePane> ImagePanePtr;
