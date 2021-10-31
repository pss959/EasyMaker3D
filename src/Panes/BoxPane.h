#pragma once

#include <memory>

#include "Panes/MultiPane.h"

namespace Parser { class Registry; }

/// BoxPane is a derived MultiPane that arranges contained Panes either
/// vertically or horizontally.
class BoxPane : public MultiPane {
  public:
    enum class Orientation {
        kVertical,    ///< Contained elements are laid out vertically.
        kHorizontal,  ///< Contained elements are laid out horizontally.
    };

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Returns the orientation. The default is Orientation::kVertical.
    Orientation GetOrientation() const { return orientation_; }

    /// Defines this to set the size on all cell panes.
    virtual void SetSize(const Vector2f &size) override;

  protected:
    BoxPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Orientation> orientation_{"orientation",
                                                Orientation::kVertical};
    Parser::TField<float>          spacing_{"spacing", 0};
    Parser::TField<float>          padding_{"padding", 0};
    ///@}

    void LayOutPanes_(const Vector2f &size);
    Vector2f ComputeMinSize_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<BoxPane> BoxPanePtr;
