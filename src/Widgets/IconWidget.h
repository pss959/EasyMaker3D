#pragma once

#include <memory>

#include "Enums/Action.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// IconWidget is a derived PushButtonWidget that has some fields that make it
/// easier to set up 3D icons for use on shelves.
class IconWidget : public PushButtonWidget {
  public:
    /// Returns the Action associated with the IconWidget.
    Action GetAction() const { return action_; }

    /// Returns the path to the file containing a shape to import for the
    /// icon. If this is not empty, the imported shape is added to the icon in
    /// addition to any other shapes.
    const std::string & GetImportPath() const { return import_path_; }

  protected:
    IconWidget() {}

    virtual void AddFields() override;
    virtual void CreationDone(bool is_template) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Action>   action_{"action", Action::kNone};
    Parser::TField<std::string> import_path_{"import_path"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<IconWidget> IconWidgetPtr;
