#pragma once

#include "Memory.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconPane);

/// IconPane is a derived LeafPane that displays an icon from the icon atlas.
class IconPane : public LeafPane {
  public:
    /// Sets the name of the icon to display.
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
