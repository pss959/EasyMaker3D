#include "Panes/Pane.h"

#include "Math/Linear.h"
#include "Math/ToString.h"
#include "Panes/IPaneInteractor.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/ClickableWidget.h"

void Pane::AddFields() {
    AddField(min_size_.Init("min_size",           Vector2f(1, 1)));
    AddField(resize_flags_.Init("resize_flags"));
    AddField(background_.Init("background"));
    AddField(border_.Init("border"));

    SG::Node::AddFields();
}

void Pane::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        if (auto &background = background_.GetValue())
            GetAuxParent().AddChild(background);
        if (auto border = border_.GetValue()) {
            GetAuxParent().AddChild(border);
            // Offset the border to avoid Z-fighting
            border->SetTranslation(Vector3f(0, 0, TK::kPaneZOffset));
        }

        // Assume the layout needs to be computed at least once.
        was_layout_changed_ = true;
    }
}

const Vector2f & Pane::GetBaseSize() const {
    // Recompute the base size if necessary.
    if (was_layout_changed_) {
        const Vector2f new_base_size = ComputeBaseSize();
        if (new_base_size != base_size_) {
            const_cast<Pane *>(this)->base_size_ = new_base_size;
            KLOG('p', "Base size for " << GetDesc() << " now = " << base_size_);
        }
    }
    return base_size_;
}

void Pane::SetLayoutSize(const Vector2f &size) {
    ASSERTM(size[0] > 0 && size[1] > 0, "for " + GetDesc());
    ASSERTM(size[0] >= base_size_[0] && size[1] >= base_size_[1],
            "for " + GetDesc());
    if (layout_size_ != size) {
        layout_size_ = size;
        KLOG('p', "Layout size for " << GetDesc() << " now " << size);
        if (auto border = border_.GetValue())
            border->SetSize(size);
    }

    // Let any derived class update the layout size in contained Panes and
    // respond to the new size.
    LayOutSubPanes();
    UpdateForLayoutSize(size);

    was_layout_changed_ = false;
    KLOG('q', "Cleared layout change flag for " << GetDesc());
}

WidgetPtr Pane::GetTouchedWidget(const TouchInfo &info,
                                 float &closest_distance) {
    WidgetPtr best_widget;
    if (const auto interactor = GetInteractor()) {
        const auto widget = interactor->GetActivationWidget();
        float dist;
        if (widget && widget->IsTouched(info, dist) &&
            dist < closest_distance) {
            closest_distance = dist;
            best_widget = widget;
        }
    }

    // Try enabled sub-Panes as well.
    for (auto &pane: GetSubPanes()) {
        if (pane->IsEnabled()) {
            auto widget = pane->GetTouchedWidget(info, closest_distance);
            if (widget)
                best_widget = widget;
        }
    }

    return best_widget;
}

void Pane::GetFocusableSubPanes(Pane::PaneVec &panes) const {
    for (auto &sub_pane: GetSubPanes()) {
        if (sub_pane->GetInteractor())
            panes.push_back(sub_pane);
        sub_pane->GetFocusableSubPanes(panes);
    }
}

PanePtr Pane::FindSubPane(const Str &name) const {
    PanePtr found;
    for (const auto &pane: GetSubPanes()) {
        if (pane->GetName() == name)
            found = pane;
        else
            found = pane->FindSubPane(name);
        if (found)
            break;
    }
    return found;
}

// LCOV_EXCL_START [debug only]
Str Pane::ToString(bool is_brief) const {
    auto tostr2 = [&](const Vector2f &v){ return Math::ToString(v, .01f); };
    auto tostr3 = [&](const Vector3f &v){ return tostr2(Vector2f(v[0], v[1])); };
    const Str base_size_star = was_layout_changed_ ? "*" : "";

    Str s = (IsEnabled() ? "" : "x:") + GetDesc();

    if (is_brief) {
        // Add the upper-left point and size relative to the parent's
        // rectangle. This is derived from the reverse of the math in
        // ContainerPane::PositionSubPane().

        // Trans = rel_center - (.5,.5)
        // Trans = (UL + COFF) / parent_size - (.5,.5)
        // Trans = (UL + (.5,-.5) * child_size) / parent_size - (.5,.5)
        // Trans + (.5,.5) = (UL + (.5,-.5) * child_size) / parent_size
        // parent_size * (Trans + (.5,.5)) - (.5,-.5) * child_size = UL

        const Vector2f rel_size    = ToVector2f(GetScale());
        const Vector2f child_size  = GetLayoutSize();
        const Vector2f parent_size = child_size / rel_size;

        const Vector2f center      = ToVector2f(GetTranslation());
        const Vector2f upper_left(
            parent_size[0] * (center[0] + .5f) - .5f * child_size[0],
            parent_size[1] * (center[1] + .5f) + .5f * child_size[1]);
        s += " " + tostr2(upper_left) + " / " + tostr2(rel_size);
    }
    else {
        s +=
            Str(HasBackground() ? " BG" : "") +
            " MS="  + tostr2(GetMinSize()) +
            " BS="  + tostr2(GetCurrentBaseSize()) + base_size_star +
            " LS="  + tostr2(GetLayoutSize()) +
            " SC="  + tostr3(GetScale()) +
            " TR="  + tostr3(GetTranslation()) +
            " RS=[" + GetResizeFlags().ToString() + "]";
    }
    return s;
}
// LCOV_EXCL_STOP

void Pane::SetMinSize(const Vector2f &size) {
    if (min_size_ != size) {
        KLOG('p', "MinSize for " << GetDesc() << " now " << size);
        min_size_ = size;
        MarkLayoutAsChanged();
    }
}

void Pane::MarkLayoutAsChanged() {
    if (! was_layout_changed_) {
        KLOG('q', GetDesc() << " marked as changed");
        was_layout_changed_ = true;
        layout_changed_.Notify();
    }
}
