#pragma once

#include "Base/Memory.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconPane);

/// IconPane is a derived LeafPane that displays an icon from the icon atlas.
///
/// \ingroup Panes
class IconPane : public LeafPane {
  public:
    /// Sets the name of the icon to display.
    void SetIconName(const Str &name);

    virtual Str ToString(bool is_brief) const override;

  protected:
    IconPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> icon_name_;
    ///@}

    friend class Parser::Registry;
};
