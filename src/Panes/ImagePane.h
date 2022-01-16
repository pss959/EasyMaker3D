#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// ImagePane is a derived Pane that displays an image.
class ImagePane : public Pane {
  protected:
    ImagePane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone(bool is_template) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ImagePane> ImagePanePtr;
