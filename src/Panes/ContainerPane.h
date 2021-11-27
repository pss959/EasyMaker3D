#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "Util/General.h"

/// ContainerPane is an abstract base class for all derived Pane classes that
/// contain one or more other Pane instances.
class ContainerPane : public Pane {
  public:
    virtual void AddFields() override;

    /// Returns a vector of all contained Panes.
    const std::vector<PanePtr> GetPanes() const { return panes_; }

    /// Returns the contained Pane with the given name, or a null pointer if
    /// there is no such Pane. Note that this is not recursive.
    PanePtr FindPane(const std::string &name) const;

    /// Returns the contained Pane with the templated type and given name, or a
    /// null pointer if there is no such Pane.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const std::string &name) const {
        return Util::CastToDerived<T>(FindPane(name));
    }

    virtual void PreSetUpIon()  override;
    virtual void PostSetUpIon() override;

  protected:
    ContainerPane() {}

    /// Convenience that calls SetRectInParent() based on the given size and
    /// upper-left corner position, assuming that GetSize() returns the correct
    /// current size for this.
    void SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                        const Vector2f &size);

    /// Allow derived classes to replace Panes with new ones.
    void ReplacePanes(const std::vector<PanePtr> &panes);

    /// Redefines this to also copy the were_panes_added_as_children_ flag.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}

    /// This is set to true once Panes from the panes_ field have been added as
    /// children to the ContainerPane.
    bool were_panes_added_as_children_ = false;

    void ObservePanes_();
    void UnobservePanes_();
    void AddPanesAsChildren_();
    void RemovePanesAsChildren_();
};

typedef std::shared_ptr<ContainerPane> ContainerPanePtr;
