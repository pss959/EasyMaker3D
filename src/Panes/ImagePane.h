#pragma once

#include "Base/Memory.h"
#include "Panes/LeafPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ImagePane);

/// ImagePane is a derived LeafPane that displays an image.
///
/// \ingroup Panes
class ImagePane : public LeafPane {
  public:
    virtual Str ToString(bool is_brief) const override;

  protected:
    ImagePane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> path_;
    ///@}

    friend class Parser::Registry;
};
