#pragma once

namespace Parser { class Parser; }

namespace SG {

//! This performs one-time initialization to set up SG.
void Init();

//! Registers all concrete SG types with the given Parser instance so they can
//! be parsed.
void RegisterTypes(Parser::Parser &parser);

}  // namespace SG
