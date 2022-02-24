#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"

/// ScaleCommand is used to scale the currently selected Models. Scaling can be
/// symmetric (scaling about the Model's center in all dimensions) or
/// asymmetric (scaling relative to a face or corner of the bounds). Asymmetric
/// scales require a change in position to compensate for the movement of the
/// bounds center.
///
/// \ingroup Commands
class ScaleCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Returns the change in scale factors, expressed as ratios applied to the
    /// current scale factors. A value of 1 in any dimension means that that
    /// dimension is not changed. A negative value in any dimension indicates
    /// that the scale is relative to the minimum bounds in that dimension for
    /// an asymmetric scale; the absolute value is used as the actual change.
    const Vector3f & GetRatios() const { return ratios_; }

    /// Sets the scale ratios.
    void SetRatios(const Vector3f &ratios) { ratios_ = ratios; }

    /// Returns true if the scale is symmetric; the default is false.
    bool IsSymmetric() const { return is_symmetric_; }

    /// Sets whether the scale is symmetric; the default is false.
    void SetIsSymmetric(bool is_symmetric) { is_symmetric_ = is_symmetric; }

  protected:
    ScaleCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> ratios_{"ratios", {1, 1, 1}};
    Parser::TField<bool>     is_symmetric_{"is_symmetric", false};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ScaleCommand> ScaleCommandPtr;
