#pragma once

#include "Enums/PaneOrientation.h"
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
    /// Returns the orientation. The default is PaneOrientation::kVertical.
    PaneOrientation GetOrientation() const { return orientation_; }

    /// Returns the padding. The default is 0.
    float GetPadding() const { return padding_; }

    /// Returns the spacing. The default is 0.
    float GetSpacing() const { return spacing_; }

    virtual Str ToString(bool is_brief) const override;

  protected:
    BoxPane() {}

    virtual void AddFields() override;

    virtual Vector2f ComputeBaseSize() const override;
    virtual void     LayOutSubPanes() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PaneOrientation> orientation_;
    Parser::TField<float>              spacing_;
    Parser::TField<float>              padding_;
    ///@}

    friend class Parser::Registry;
};
