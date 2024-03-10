

#include "navigation_data_module.h"
#include <squick/plugin/world/i_event_module.h>
/*
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"
*/

bool NavigationDataModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_event_ = pm_->FindModule<IEventModule>();
    m_scene_ = pm_->FindModule<ISceneModule>();

    return true;
}

bool NavigationDataModule::AfterStart() {
    /*
    rapidjson::Document document;
    document.Parse(jsonData.c_str());

    rapidjson::Value& tielConfigData = document["tileConfig"];
    rapidjson::Value& navigationData = document["data"];
    for (auto itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
    {
        printf("Type of member %s is %d\n", itr->name.GetString(), itr->value.GetType());
    }
    */

    // public Dictionary<int, Dictionary<Guid, Voxel>> data;
    auto sceneElement = m_class_->GetElement(excel::Scene::ThisName());
    auto sceneList = sceneElement->GetIDList();
    for (auto it = sceneList.begin(); it != sceneList.end(); ++it) {
        const std::string &sceneID = *it;
        const int scene = std::atoi(sceneID.c_str());
        const std::string &sceneName = m_element_->GetPropertyString(sceneID, excel::Scene::SceneName());

        std::string jsonData;
        pm_->GetFileContent("../config/ini/scene/" + sceneName + ".json", jsonData);
        if (jsonData.empty()) {
            continue;
        }

        auto groupData = ParseDefaultMapData(scene, jsonData, true);
        mGroupNavigationData.AddElement(Guid(scene, 0), groupData);
    }

    return true;
}

const std::string &NavigationDataModule::GetDefaultMapData(const int scene) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, 0));
    if (data) {
        return data->originalData;
    }

    return NULL_STR;
}

const std::shared_ptr<GroupNavigationData> NavigationDataModule::GetMapData(const int scene, const int group) {
    return mGroupNavigationData.GetElement(Guid(scene, group));
}

const std::shared_ptr<Voxel> NavigationDataModule::GetMapData(const int scene, const int group, const int x, int z) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, group));
    if (data) {
        return data->data.GetElement(Guid(x, z));
    }

    return nullptr;
}

bool NavigationDataModule::SetMapDataOccupyItem(const int scene, const int group, const int x, const int z, const std::string &item) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, group));
    if (data) {
        auto voxel = data->data.GetElement(Guid(x, z));
        if (voxel) {
            voxel->item = item;

            return true;
        }
    }

    return false;
}

bool NavigationDataModule::SetMapDataMovable(const int scene, const int group, const int x, int z, const int movable) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, group));
    if (data) {
        auto voxel = data->data.GetElement(Guid(x, z));
        if (voxel) {
            voxel->movable = movable;

            return true;
        }
    }

    return false;
}

bool NavigationDataModule::SetMapDataOccupy(const int scene, const int group, const int x, int z, const Guid occupy) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, group));
    if (data) {
        auto voxel = data->data.GetElement(Guid(x, z));
        if (voxel) {
            voxel->occupyObject = occupy;

            return true;
        }
    }

    return false;
}

bool NavigationDataModule::SetMapDataLayer(const int scene, const int group, const int x, int z, const int layer) {
    auto data = mGroupNavigationData.GetElement(Guid(scene, group));
    if (data) {
        auto voxel = data->data.GetElement(Guid(x, z));
        if (voxel) {
            voxel->layer = layer;

            return true;
        }
    }

    return false;
}

bool NavigationDataModule::Destory() { return true; }

bool NavigationDataModule::Update() { return true; }

const std::shared_ptr<GroupNavigationData> NavigationDataModule::GetMapData(const int scene) { return GetMapData(scene, 0); }
