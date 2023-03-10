#pragma once


#include <squick/core/base.h>
#include <google/protobuf/dynamic_message.h>

#include <squick/plugin/kernel/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/net/export.h>

#include "../server/i_server_module.h"
#include "../client/i_db_module.h"
#include "../lobby/i_player_manager_module.h"
#include "../lobby/i_room_module.h"


namespace game::play {

	typedef std::function<void(const Guid &clientID, const int msgID, const std::string &data)> GAME_PLAY_RECEIVE_FUNCTOR;
	typedef std::shared_ptr<GAME_PLAY_RECEIVE_FUNCTOR> GAME_PLAY_RECEIVE_FUNCTOR_PTR;

	class IGameplayManagerModule : public IModule
	{
	public:
		virtual bool GameplayCreate(int id, const string& key) = 0;
		virtual bool GameplayDestroy(int group_id) = 0;
		virtual bool GameplayPlayerQuit(const Guid& player) = 0;
		
		template<typename BaseType>
		bool AddReceiveCallBack(const int msgID, const int group_id, BaseType* pBase, void (BaseType::* handleReceiver)(const Guid& clientID, const int msgID, const std::string& data))
		{
			GAME_PLAY_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			GAME_PLAY_RECEIVE_FUNCTOR_PTR functorPtr(new GAME_PLAY_RECEIVE_FUNCTOR(functor));

			if (mxReceiveCallBack.find(msgID) == mxReceiveCallBack.end()) // 之前未绑定
			{
				m_pNetModule->RemoveReceiveCallBack(msgID);
				m_pNetModule->AddReceiveCallBack(msgID, this, &IGameplayManagerModule::OnRecv);

				std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR> msgIdMap;
				msgIdMap[group_id] = functorPtr;
				mxReceiveCallBack.insert( std::map<int, std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR>>::value_type(msgID, msgIdMap) );
				return true;
			}

			std::map<int, std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR>>::iterator it = mxReceiveCallBack.find(msgID);
			it->second[group_id] = functorPtr;
			return true;
		}
		
		virtual void OnRecv(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) = 0;

			GAME_PLAY_RECEIVE_FUNCTOR_PTR& GetCallback(int msgID, int group_id) {
			auto& group = mxReceiveCallBack[msgID];
			return group[group_id];
		}

		INetModule* m_pNetModule;
		IClassModule* m_pClassModule;
		IElementModule* m_pElementModule;
		IKernelModule* m_pKernelModule;
		ISceneModule* m_pSceneModule;
		IGameServerNet_ServerModule* m_pGameServerNet_ServerModule;
		IGameServerToDBModule* m_pGameToDBModule;
		INetClientModule* m_pNetClientModule;
		IScheduleModule* m_pScheduleModule;
		IDataTailModule* m_pDataTailModule;
		IEventModule* m_pEventModule;

		player::IPlayerManagerModule* m_pPlayerManagerModule;
		player::IRoomModule* m_pRoomModule;

		
	private:
		std::map<int, std::map<int, GAME_PLAY_RECEIVE_FUNCTOR_PTR> > mxReceiveCallBack;

		
	};
}
