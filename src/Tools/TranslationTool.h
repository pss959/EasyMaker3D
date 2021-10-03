#pragma once

#include "Tools/GeneralTool.h"

/// TranslationTool allows interactive translation along any of the principal
/// coordinate axes.
///
/// \ingroup Tools
class TranslationTool : public GeneralTool {
  protected:
    TranslationTool() {}
    virtual void Attach(const ModelPtr &model) override;
    virtual void Detach() override;

    friend class Parser::Registry;
};
