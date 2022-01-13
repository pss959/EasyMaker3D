#pragma once

#include <memory>
#include <string>

#include "Enums/Action.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

/// The RadialMenuInfo struct packages up information about a radial menu. It
/// is derived from Parser::Object so the settings can be read from and written
/// to files.
class RadialMenuInfo : public Parser::Object {
  public:
    /// Enum indicating the number of buttons in the menu.
    enum class Count {
        kCount1 = 1,
        kCount2 = 2,
        kCount4 = 4,
        kCount8 = 8,
    };

    static const size_t kMaxCount = 8;  ///< Maximum number of buttons.

    virtual void AddFields() override;

    /// Returns the count of active buttons in the menu.
    Count  GetCount() const { return count_; }

    /// Returns the Action associated with the indexed button. Asserts if the
    /// index is more than kMaxCount.
    Action GetButtonAction(size_t index) const;

    void   SetCount(Count count) { count_ = count; }
    void   SetButtonAction(size_t index, Action action);

    /// Copies values from another instance.
    void CopyFrom(const RadialMenuInfo &from) { CopyContentsFrom(from, false); }

  protected:
    RadialMenuInfo() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Count>    count_{"count", Count::kCount1};
    Parser::VField<std::string> actions_{"actions"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<RadialMenuInfo> RadialMenuInfoPtr;
