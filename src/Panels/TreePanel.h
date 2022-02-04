#pragma once

#include <memory>
#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// TreePanel is a derived Panel class that is attached to he WallBoard. It
/// displays the current session info, current Models, and allows various
/// interactions with them.
class TreePanel : public Panel {
  public:
    /// Resets the TreePanel to initial conditions.
    void Reset();

    /// Sets the name of the session to display at the top along with a string
    /// representing modifications made in the current session.
    void SetSessionString(const std::string &str);

  protected:
    TreePanel();

    virtual void InitInterface() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;  /// Implementation instance.

    friend class Parser::Registry;
};

typedef std::shared_ptr<TreePanel> TreePanelPtr;
