#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Border);

/// Border is used to create a colored 2D border. It uses triangles to simulate
/// wide lines.
///
/// \ingroup Items
class Border : public SG::Node {
  public:
    virtual void PostSetUpIon() override;

    /// Sets the 2D size of the border. The geometry is updated to maintain the
    /// proper border width.
    void SetSize(const Vector2f &size);

  protected:
    Border();

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
