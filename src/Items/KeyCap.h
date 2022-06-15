#pragma once

#include "Base/Memory.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(KeyCap);

/// The KeyCap class is used by the VirtualKeyboard for each key.
///
/// \ingroup Items
class KeyCap : public SG::Node {
  public:
    /// A name is required for a KeyCap; it is used for the key label.
    virtual bool IsNameRequired() const override { return true; }

    /// Returns the keycap's width, which is 1 for a square keycap.
    float GetWidth() const { return width_; }

    /// Returns the text that the keycap produces when hit, modified by the
    /// is_shifted flag.
    std::string GetText(bool is_shifted);

    /// Updates the displayed label text to match the returned text, modified
    /// by the is_shifted flag.
    void UpdateLabelText(bool is_shifted);

  protected:
    KeyCap() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>       width_{"width", 1};
    Parser::TField<std::string> text_{"text"};
    Parser::TField<std::string> shifted_text_{"shifted_text"};
    ///@}

    friend class Parser::Registry;
};
