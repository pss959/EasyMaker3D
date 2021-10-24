#pragma once

#include <memory>

#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// IconWidget is a derived PushButtonWidget that has some fields that make it
/// easier to set up 3D icons for use on shelves.
class IconWidget : public PushButtonWidget {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Returns the path to the file containing a shape to import for the
    /// icon. If this is not empty, the imported shape is added to the icon in
    /// addition to any other shapes.
    const std::string & GetImportPath() const { return import_path_; }

    /// Redefines this to set up imported shapes.
    virtual void PreSetUpIon() override;

  protected:
    IconWidget() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> import_path_{"import_path"};
    ///@}

    /// Guards against adding imported shape multiple times.
    bool added_imported_shape_ = false;

    friend class Parser::Registry;
};

typedef std::shared_ptr<IconWidget> IconWidgetPtr;
