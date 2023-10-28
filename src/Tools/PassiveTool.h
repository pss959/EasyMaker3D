#pragma once

#include "Tools/Tool.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(PassiveTool);

/// PassiveTool is a derived Tool class that has no interactive interface. It is
/// a special case that is used to indicate objects that are selected but that
/// either have no editable components or that are part of the secondary
/// selection. There is no icon for it because it is not user-selectable.
///
/// \ingroup Tools
class PassiveTool : public Tool {
  protected:
    PassiveTool() {}

    /// This can be attached to any Model.
    virtual bool CanAttach(const Selection &sel) const override { return true; }

    virtual void Attach() override;
    virtual void Detach() override;

  private:
    SG::NodePtr   corners_[8];   ///< Nodes representing the 8 corners.
    SG::NodePtr   edges_[12];    ///< Nodes representing the 12 edges.
    float         edge_size_;    ///< Original size of each edge in all 3 dims.

    void CreateParts_();
    void UpdateCorners_(const Vector3f &model_scale);
    void UpdateEdges_(const Vector3f &model_scale);

    friend class Parser::Registry;
};

