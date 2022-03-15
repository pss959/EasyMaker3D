#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// IconPane is a derived Pane that displays an icon from the icon atlas.
class IconPane : public Pane {
  public:
    /// Sets the name of the icon to display in the Pane.
    void SetIconName(const std::string &name);

    virtual std::string ToString() const override;

  protected:
    IconPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> icon_name_{"icon_name"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<IconPane> IconPanePtr;
