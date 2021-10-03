#pragma once

#include "Tools/Tool.h"

/// GeneralTool is a derived Tool class that serves as an abstract base class
/// for all interactive tools that can be attached to just about all types of
/// Models.
///
/// \ingroup Tools
class GeneralTool : public Tool {
  protected:
    virtual bool CanAttach(const Selection &sel) const override { return true; }
};

typedef std::shared_ptr<GeneralTool> GeneralToolPtr;
