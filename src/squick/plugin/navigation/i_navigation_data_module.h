#pragma once

#include <squick/core/i_module.h>
#include <third_party/ajson/ajson.hpp>
#include <third_party/nlohmann/json.hpp>

using json = nlohmann::json;

class TileConfig
{
public:
    int maxLayer = 10;
    int accumulationLayer = 3;
    int mapSize = 32;

    int cellSizeX = 2;
    int cellSizeZ = 2;
    float layerHeight = 1.0f;
};

AJSON(TileConfig, maxLayer, accumulationLayer, mapSize, cellSizeX, cellSizeZ, layerHeight)

class Voxel
{
public:
    int layer = 1;
    string name;
    string item;
    int movable = 0;
    int breakable = 0;
    int stair_h = 0;
    int stair_v = 0;
    //expand
    Guid occupyObject;
    int x = 0;
    int z = 0;
};
AJSON(Voxel, layer, name, item, movable, breakable, stair_h, stair_v)

class GroupNavigationData
{
private:
    GroupNavigationData() {}
public:
    GroupNavigationData(const int scene, const int group)
    {
        this->sceneID = scene;
        this->groupID = group;
    }

    int sceneID;
    int groupID;

    std::string originalData; 

    TileConfig tileConfig;
    //<x, z> ==> data
    MapEx<Guid, Voxel> data;
};

class INavigationDataModule : public IModule
{
public:
    virtual const std::string& GetDefaultMapData(const int scene) = 0;

    static SQUICK_SHARE_PTR<GroupNavigationData> ParseDefaultMapData(const int scene, const std::string& sceneMapData, const bool saveOriginalData = false)
    {
        auto groupData = SQUICK_SHARE_PTR<GroupNavigationData>(SQUICK_NEW GroupNavigationData(scene, 0));
        if (!sceneMapData.empty())
        {
            auto map = json::parse(sceneMapData);
            auto tileConfig = map["tileConfig"];
            auto data = map["data"];

            if (saveOriginalData)
			{
				groupData->originalData = sceneMapData;
			}

            std::string tileConfigValue = tileConfig.dump();
            ajson::load_from_buff(groupData->tileConfig, tileConfigValue.c_str(), tileConfigValue.length());
            for (auto it = data.begin(); it != data.end(); ++it)
            {
                const std::string& cellID = it.key();
                const std::string& cellData = it.value().dump();
                Guid posID(cellID);
                auto voxelData = groupData->data.GetElement(posID);
                if (voxelData == nullptr)
                {
                    voxelData = SQUICK_SHARE_PTR<Voxel>(SQUICK_NEW Voxel());
                    ajson::load_from_buff(*voxelData, cellData.c_str(), cellData.length());
                    voxelData->x = (int)posID.GetHead();
                    voxelData->z = (int)posID.GetData();

                    groupData->data.AddElement(posID, voxelData);
                }
            }
        }

        return groupData;
    }

	virtual const SQUICK_SHARE_PTR<GroupNavigationData> GetMapData(const int scene) = 0;
	virtual const SQUICK_SHARE_PTR<GroupNavigationData> GetMapData(const int scene, const int group) = 0;
    virtual const SQUICK_SHARE_PTR<Voxel> GetMapData(const int scene, const int group, const int x, int z) = 0;

    //modify map data in run time
    virtual bool SetMapDataOccupyItem(const int scene, const int group, const int x, const int z, const std::string& item) = 0;
    virtual bool SetMapDataMovable(const int scene, const int group, const int x, int z, const int movable) = 0;
    virtual bool SetMapDataOccupy(const int scene, const int group, const int x, int z, const Guid occupy) = 0;
    virtual bool SetMapDataLayer(const int scene, const int group, const int x, int z, const int layer) = 0;

    //register callback function for modifying the map data

};
