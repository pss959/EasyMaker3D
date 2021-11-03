#pragma once

#include <memory>

#include "Panes/ContainerPane.h"

namespace Parser { class Registry; }

/// BoxPane is a derived ContainerPane that arranges contained Panes either
/// vertically or horizontally.
class BoxPane : public ContainerPane {
  public:
    enum class Orientation {
        kVertical,    ///< Contained elements are laid out vertically.
        kHorizontal,  ///< Contained elements are laid out horizontally.
    };

    virtual void AddFields() override;

    /// Returns the orientation. The default is Orientation::kVertical.
    Orientation GetOrientation() const { return orientation_; }

    /// Defines this to set the size on all cell panes.
    virtual void SetSize(const Vector2f &size) override;

  protected:
    BoxPane() {}

    /// Redefines this to do math.
    virtual Vector2f ComputeMinSize() const;

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
