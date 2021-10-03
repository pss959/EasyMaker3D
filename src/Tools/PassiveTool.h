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
    /// This can be attached to any Model.
    virtual bool CanAttach(const Selection &sel) const override { return true; }

    virtual void Attach(const ModelPtr &model) override {
        // XXXX DO SOMETHING!!!
    }

    virtual void Detach() override { /* Nothing to do. */ }
};

typedef std::shared_ptr<PassiveTool> PassiveToolPtr;
