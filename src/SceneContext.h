#pragma once

#include "Frustum.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/TextNode.h"

//! The SceneContext struct encapsulates an SG::Scene and all of the important
//! Nodes in it that are needed by the application.
struct SceneContext {
    SG::ScenePtr    scene;
    Frustum         frustum;
    SG::NodePtr     left_controller;
    SG::NodePtr     right_controller;
    SG::TextNodePtr debug_text;

    // XXXX More...
};
