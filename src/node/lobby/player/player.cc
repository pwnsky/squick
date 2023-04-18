#include "player.h"

namespace lobby::player {

Player::Player() {
    // dout << "玩家进入游戏\n";
}

Player::~Player() {
    // dout << "玩家对象销毁\n";
}

// 刚进入游戏
void Player::OnEnterGame() {}

// 离线退出
void Player::OnOffline() {}

// 即将销毁
void Player::OnDestroy() {}

} // namespace lobby::player