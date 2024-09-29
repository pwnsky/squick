#include "game.h"
#include <struct/struct.h>
namespace game::logic {

Game::Game() {}

Game::~Game() {}

void Game::Awake() {}

void Game::Start() {}

void Game::Update() {}

void Game::Destroy() {}

void Game::PlayerJoin(const Guid &player) {}

void Game::PlayerQuit(const Guid &player) {}

void Game::AllPlayerJoined() {}
} // namespace game::logic