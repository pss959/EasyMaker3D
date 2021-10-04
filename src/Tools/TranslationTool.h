#pragma once

#include <memory>

#include "Tools/GeneralTool.h"

/// TranslationTool allows interactive translation along any of the principal
/// coordinate axes.
///
/// \ingroup Tools
class TranslationTool : public GeneralTool {
  protected:
    TranslationTool();
    virtual void Attach(const SelPath &path) override;
    virtual void Detach() override;

  private:
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    void FindParts_();

    friend class Parser::Registry;
};
