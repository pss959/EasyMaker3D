#pragma once

#include "Base/Memory.h"
#include "Panes/ContainerPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(BoxPane);

/// BoxPane is a derived ContainerPane that arranges contained Panes either
/// vertically or horizontally.
///
/// \ingroup Panes
class BoxPane : public ContainerPane {
  public:
    enum class Orientation {
        kVertical,    ///< Contained elements are laid out vertically.
        kHorizontal,  ///< Contained elements are laid out horizontally.
    };

    /// Returns the orientation. The default is Orientation::kVertical.
    Orientation GetOrientation() const { return orientation_; }

    /// Returns the padding. The default is 0.
    float GetPadding() const { return padding_; }

    /// Returns the spacing. The default is 0.
    float GetSpacing() const { return spacing_; }

    virtual std::string ToString() const override;

  protected:
    BoxPane() {}

    virtual void AddFields() override;

    virtual Vector2f ComputeBaseSize() const override;
    virtual void     LayOutSubPanes() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Orientation> orientation_{"orientation",
                                                Orientation::kVertical};
    Parser::TField<float>          spacing_{"spacing", 0};
    Parser::TField<float>          padding_{"padding", 0};
    ///@}

    /// Computes the amount of extra size to add for expanding Panes in the
    /// given dimension, given the total size in that dimension.
    float ComputeExtraSize_(int dim, float size);

    /// Returns true if the given Pane expands in the given dimension.
    static bool Expands_(const Pane &pane, int dim) {
        return dim == 0 ? pane.IsWidthResizable() : pane.IsHeightResizable();
    }

    friend class Parser::Registry;
};
