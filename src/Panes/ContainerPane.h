#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// ContainerPane is an abstract base class for all derived Pane classes that
/// contain one or more other Pane instances. It stores a collection of
/// sub-panes in the "panes" field.
class ContainerPane : public Pane {
  public:
    virtual ~ContainerPane();

    virtual void AddFields() override;

    /// Returns a vector of all contained Panes.
    const std::vector<PanePtr> & GetPanes() const { return panes_.GetValue(); }

    /// Searches recursively for the contained Pane with the given name.
    /// Returns null if it is not found.
    PanePtr FindPane(const std::string &name) const;

    /// Searches recursively for the contained Pane with the given type and
    /// name. Asserts if it is not found.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const std::string &name) const {
        auto pane = FindPane(name);
        ASSERTM(pane, "Pane '" + name + "' not found in " + GetDesc());
        auto typed_pane = Util::CastToDerived<T>(pane);
        ASSERTM(typed_pane, "Wrong type for Pane '" + name +
                "' in " + GetDesc());
        return typed_pane;
    }

    /// Searches recursively for the given contained Pane, returning an
    /// SG::NodePath from this Pane to it. Returns an empty NodePath if it is
    /// not found.
    SG::NodePath FindPanePath(const Pane &pane) const;

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

    /// Redefines this to also add the copied Panes as children.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// Redefines this to return all sub-panes so that they are added as Ion
    /// children.
    virtual std::vector<SG::NodePtr> GetExtraIonChildren() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}

    /// This is set to true after ObservePanes_() is called.
    bool were_panes_observed_ = false;

    void ObservePanes_();
    void UnobservePanes_();
};

typedef std::shared_ptr<ContainerPane> ContainerPanePtr;
