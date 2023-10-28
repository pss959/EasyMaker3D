#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(RootModel);
DECL_SHARED_PTR(TreePanel);

namespace Parser { class Registry; }

/// TreePanel is a derived Panel class that is attached to he WallBoard. It
/// displays the current session info, current Models, and allows various
/// interactions with them.
///
/// \ingroup Panels
class TreePanel : public Panel {
  public:
    /// Resets the TreePanel to initial conditions.
    void Reset();

    /// Sets the name of the session to display at the top along with a string
    /// representing modifications made in the current session.
    void SetSessionString(const Str &str);

    /// Sets the RootModel that defines the current set of models.
    void SetRootModel(const RootModelPtr &root_model);

    /// This should be called when the current set of Models in the scene
    /// changes or when the current selection changes so the TreePanel can
    /// update its display.
    void ModelsChanged();

    /// Defines this to update contents if necessary.
    virtual void UpdateForRenderPass(const Str &pass_name) override;

  protected:
    TreePanel();

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

    /// Redefines this to also set up the Impl_ from the Context.
    virtual void ProcessContext() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;  ///< Implementation instance.

    friend class Parser::Registry;
};
