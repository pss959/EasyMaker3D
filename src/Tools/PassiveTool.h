#pragma once

#include <memory>

#include "Tools/Tool.h"

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

    virtual void Attach() override {
        // XXXX THIS SHOULD NEVER BE CALLED!
    }

    virtual void Detach() override { /* Nothing to do. */ }

    // XXXX Special function for attaching to a secondary selection.
    void AttachToModel(const SelPath &path);

  private:
    SG::NodePtr   corner_;   ///< Node representing a corner.
    SG::NodePtr   edge_;     ///< Node representing an edge.

    friend class Parser::Registry;
};

typedef std::shared_ptr<PassiveTool> PassiveToolPtr;
