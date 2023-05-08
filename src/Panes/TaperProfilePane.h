#pragma once

#include "Base/Memory.h"
#include "Panes/ProfilePane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TaperProfilePane);

/// TaperProfilePane is a derived ProfilePane that limits profile editing to
/// valid Taper profiles (monotonically decreasing in Y, endpoints at Y=1 and
/// Y=0).
///
/// \ingroup Panes
class TaperProfilePane : public ProfilePane {
  protected:
    TaperProfilePane();
    virtual ~TaperProfilePane() override;

  private:
    friend class Parser::Registry;
};
