#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// ContainerPane is an abstract base class for Pane classes that contain one
/// or more other Pane instances. It stores a collection of sub-panes in the
/// "panes" field.
class ContainerPane : public Pane {
  public:
    virtual ~ContainerPane();

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

    /// Removes the given Pane. Asserts if it is not found.
    void RemovePane(const PanePtr &pane);

    /// Replaces all contained Panes with new ones.
    void ReplacePanes(const std::vector<PanePtr> &panes);

    /// Redefines this to let the derived class lay out contained panes.
    virtual void SetSize(const Vector2f &size) override;

  protected:
    ContainerPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Derived classes must implement this to lay out contained Panes when the
    /// size of the ContainerPane changes. The new size is provided.
    virtual void LayOutPanes(const Vector2f &size) = 0;

    /// Returns the SG::Node to add panes to as extra children. The base class
    /// defines this to return the ContainerPane itself.
    virtual SG::Node & GetExtraChildParent() { return *this; }

    /// Allows derived classes to hide the panes field so it can be reserved
    /// for internal use only.
    void HidePanesField() { panes_.SetHidden(true); }

    /// Returns a Range2f rectangle representing the relative size and position
    /// of a sub Pane within a ContainerPane. The ContainerPane size, sub Pane
    /// size, and upper-left corner position are provided. Note that the
    /// resulting rectangle may lie partially or fully outside the container,
    /// since Panes in a ClipPane may be outside the clip rectangle.
    static Range2f ComputeSubPaneRect(const Vector2f &kcontainer_pane_size,
                                      const Vector2f &sub_pane_size,
                                      const Point2f &upper_left);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}

    void ObservePanes_();
    void UnobservePanes_();

    /// Offsets each contained pane to move it in front.
    void OffsetPanes_();
};

typedef std::shared_ptr<ContainerPane> ContainerPanePtr;
