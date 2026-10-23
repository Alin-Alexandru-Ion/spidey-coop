// Co-op / camera: Select and render independent camera behavior in the shared world.
#pragma once
// Lookaround's letterbox is local presentation, not a shared mission scene.
inline bool CoOp_LocalAim(bool joined,bool aiming,bool scriptedScene) {
    return joined && aiming && !scriptedScene;
}
