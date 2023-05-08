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

    /// Redefines this to limit the range of movable points to maintain a valid
    /// Taper profile.
    virtual Range2f GetMovablePointRange(Slider2DWidget &slider,
                                         const Profile &profile,
                                         size_t index) const override;

    /// Redefines this to return false if there is not enough Y room between
    /// the neighbors.
    virtual bool CanInsertPoint(const Profile &profile,
                                size_t index) const override;

  private:
    friend class Parser::Registry;
};
