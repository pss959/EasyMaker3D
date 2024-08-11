//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Action.h"
#include "Parser/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RadialMenuInfo);

/// The RadialMenuInfo struct packages up information about a radial menu. It
/// is derived from Parser::Object so the settings can be read from and written
/// to files.
///
/// \ingroup Items
class RadialMenuInfo : public Parser::Object {
  public:
    /// Enum indicating the number of buttons in the menu.
    enum class Count : size_t {
        kCount2 = 2,
        kCount4 = 4,
        kCount8 = 8,
    };

    static const size_t kMaxCount = 8;  ///< Maximum number of buttons.

    /// Creates an instance with default values.
    static RadialMenuInfoPtr CreateDefault();

    /// Returns the count of active buttons in the menu.
    Count  GetCount() const { return count_; }

    /// Returns the Action associated with the indexed button. Asserts if the
    /// index is more than kMaxCount.
    Action GetButtonAction(size_t index) const;

    void   SetCount(Count count) { count_ = count; }
    void   SetButtonAction(size_t index, Action action);

    /// Copies values from another instance.
    void CopyFrom(const RadialMenuInfo &from) { CopyContentsFrom(from, true); }

  protected:
    RadialMenuInfo() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Count> count_;
    Parser::VField<Str>      actions_;
    ///@}

    friend class Parser::Registry;
};
