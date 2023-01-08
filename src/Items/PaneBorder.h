#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PaneBorder);

/// PaneBorder is used to add a colored border to any Pane. It uses polygons to
/// simulate wide lines.
///
/// \ingroup Items
class PaneBorder : public SG::Node {
  public:
    virtual void PostSetUpIon() override;

    /// Sets the 2D size of the border. The geometry is updated to maintain the
    /// proper border width.
    void SetSize(const Vector2f &size);

  protected:
    PaneBorder();

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_;
    Parser::TField<float> width_;
    ///@}

    /// Sets up the TriMesh to form a border with the correct width for the
    /// given size.
    void UpdateMesh_(float width, const Vector2f &size);

    friend class Parser::Registry;
};
