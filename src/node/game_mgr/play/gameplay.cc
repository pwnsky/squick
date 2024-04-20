#include "gameplay.h"
#include <struct/struct.h>
namespace gameplay_manager::play {

Gameplay::Gameplay() {}

Gameplay::~Gameplay() {}

void Gameplay::Awake() {}

void Gameplay::Start() {}

void Gameplay::Update() {}

void Gameplay::Destroy() {}

void Gameplay::PlayerJoin(const Guid &player) {}

void Gameplay::PlayerQuit(const Guid &player) {}

void Gameplay::AllPlayerJoined() {}
} // namespace gameplay_manager::play