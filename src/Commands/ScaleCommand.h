#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ScaleCommand);

/// ScaleCommand is used to scale the currently selected Models. Scaling can be
/// symmetric (scaling about the Model's center in all dimensions) or
/// asymmetric (scaling relative to a face or corner of the bounds). Asymmetric
/// scales require a change in position to compensate for the movement of the
/// bounds center.
///
/// \ingroup Commands
class ScaleCommand : public MultiModelCommand {
  public:
    enum class Mode {
        /// Scale about the opposite side, edge, or corner of the Model.
        kAsymmetric,
        /// Scale symmetrically about the center point of the Model.
        kCenterSymmetric,
        /// Scale symmetrically about the center point of the Model in the Y=0
        /// plane (top of the Stage).
        kBaseSymmetric,
    };

    virtual Str GetDescription() const override;

    /// Returns the change in scale factors, expressed as ratios applied to the
    /// current scale factors. A value of 1 in any dimension means that that
    /// dimension is not changed. A negative value in any dimension indicates
    /// that the scale is relative to the minimum bounds in that dimension for
    /// an asymmetric scale; the absolute value is used as the actual change.
    const Vector3f & GetRatios() const { return ratios_; }

    /// Sets the scale ratios.
    void SetRatios(const Vector3f &ratios) { ratios_ = ratios; }

    /// Returns the scale mode; the default is Mode::kAsymmetric.
    Mode GetMode() const { return mode_; }

    /// Sets the scale Mode.
    void SetMode(Mode mode) { mode_ = mode; }

  protected:
    ScaleCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> ratios_;
    Parser::EnumField<Mode>  mode_;
    ///@}

    friend class Parser::Registry;
};
