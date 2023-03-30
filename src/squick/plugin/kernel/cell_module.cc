
#include "cell_module.h"
#include <squick/struct/excel.h>

CellModule::CellModule(IPluginManager *p) { pm_ = p; }

CellModule::~CellModule() {}

bool CellModule::Start() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_event_ = pm_->FindModule<IEventModule>();

    return true;
}

bool CellModule::AfterStart() {
    m_kernel_->AddClassCallBack(excel::Player::ThisName(), this, &CellModule::OnObjectEvent);

    // NF SYNC
    bool bCell = false;
    std::shared_ptr<IClass> xServerLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xServerLogicClass) {
        const std::vector<std::string> &strIdList = xServerLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            const int nCell = 1; // m_element_->GetPropertyInt32(strId, SquickProtocol::Server::Cell());
            if (pm_->GetAppID() == serverID && nCell == 1) {
                bCell = true;
                break;
            }
        }
    }

    if (!bCell) {
        return true;
    }
    /*
    //init all scene
    List<std::shared_ptr<SceneCellInfo>> cellList;

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(SquickProtocol::Scene::ThisName());
    if (xLogicClass)
    {
            const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

            for (int i = 0; i < strIdList.size(); ++i)
            {
                    const std::string& strId = strIdList[i];

                    int sceneID = lexical_cast<int>(strId);
                    TMAP_SCENE_INFO::iterator it = mtCellInfoMap.find(sceneID);
                    if (it == mtCellInfoMap.end())
                    {
                            TMAP_GROUP_INFO groupInfo;
                            groupInfo.insert(TMAP_GROUP_INFO::value_type(0, std::map<Guid, std::shared_ptr<SceneCellInfo>>()));
                            mtCellInfoMap.insert(TMAP_SCENE_INFO::value_type(sceneID, groupInfo));
                    }
            }

            TMAP_SCENE_INFO::iterator it = mtCellInfoMap.begin();
            for (; it != mtCellInfoMap.end(); it++)
            {
                    //init all cell, start from position 0
                    //the default group's id is 0
                    int sceneID = it->first;
                    Vector2 vLeftBot = m_element_->GetPropertyVector2(std::to_string(sceneID), SquickProtocol::Scene::LeftBot());
                    Vector2 vRightTop = m_element_->GetPropertyVector2(std::to_string(sceneID), SquickProtocol::Scene::RightTop());

                    TMAP_GROUP_INFO::iterator itGroup = it->second.find(0);

                    for (int nPosX = vLeftBot.X(); nPosX < vRightTop.X(); nPosX += nCellWidth)
                    {
                            for (int nPosY = vLeftBot.Y(); nPosY < vRightTop.Y(); nPosY += nCellWidth)
                            {
                                    Guid gridID = ComputeCellID(nPosX, 0, nPosY);
                                    std::shared_ptr<SceneCellInfo> pCellInfo = std::shared_ptr<SceneCellInfo>(new SceneCellInfo(it->first,
    itGroup->first, gridID));

                                    itGroup->second.insert(TMAP_CELL_INFO::value_type(gridID, pCellInfo));
                                    cellList.Add(pCellInfo);
                            }
                    }
            }
    }

    //all grids must connect together
    std::shared_ptr<SceneCellInfo> pCellInfo = nullptr;
    for (cellList.First(pCellInfo); pCellInfo != nullptr; cellList.Next(pCellInfo))
    {
            std::shared_ptr<SceneCellInfo> aroundGrid[ECELL_DIRECTION_MAXCOUNT] = { 0 };
            Guid cellID[ECELL_DIRECTION_MAXCOUNT];

            for (int i = 0; i <ECELL_DIRECTION_MAXCOUNT; ++i)
            {
                    cellID[i] = ComputeCellID(pCellInfo->GetID(), (ECELL_DIRECTION)i);
                    aroundGrid[i] = GetCellInfo(pCellInfo->GetSceneID(), pCellInfo->GetGroupID(), cellID[i]);
            }

            pCellInfo->Start(aroundGrid);

            pCellInfo = nullptr;
    }

    //prepare group cell pool to enhance the performance when requesting a new group
    */
    return true;
}

bool CellModule::BeforeDestory() {
    mtCellInfoMap.clear();

    return false;
}

bool CellModule::Destory() { return false; }

bool CellModule::Update() { return false; }

const bool CellModule::CreateGroupCell(const int &sceneID, const int &groupID) {
    /*
    //we would have a group cell pool
    TMAP_SCENE_INFO::iterator it = mtCellInfoMap.find(sceneID);
    if (it != mtCellInfoMap.end())
    {
            TMAP_GROUP_INFO::iterator itNewGroup = it->second.find(groupID);
            TMAP_GROUP_INFO::iterator itGroup0 = it->second.find(0);
            if (itNewGroup == it->second.end()
                    && itGroup0 != it->second.end())
            {
                    std::map<Guid, std::shared_ptr<SceneCellInfo>> newGroup;

                    List<std::shared_ptr<SceneCellInfo>> cellList;

                    TMAP_CELL_INFO::iterator itCell0 = itGroup0->second.begin();
                    for (itCell0; itCell0 != itGroup0->second.end(); itCell0++)
                    {
                            std::shared_ptr<SceneCellInfo> pCellInfo0 = itCell0->second;
                            std::shared_ptr<SceneCellInfo> pNewCellInfo = std::shared_ptr<SceneCellInfo>(new SceneCellInfo(*pCellInfo0));

                            newGroup.insert(TMAP_CELL_INFO::value_type(pNewCellInfo->GetID(), pNewCellInfo));
                            cellList.Add(pNewCellInfo);
                    }

                    it->second.insert(TMAP_GROUP_INFO::value_type(groupID, newGroup));

                    //all grids must connect together
                    std::shared_ptr<SceneCellInfo> pCellInfo = nullptr;
                    for (cellList.First(pCellInfo); pCellInfo != nullptr; cellList.Next(pCellInfo))
                    {
                            std::shared_ptr<SceneCellInfo> aroundCell[ECELL_DIRECTION_MAXCOUNT] = { 0 };
                            Guid cellID[ECELL_DIRECTION_MAXCOUNT];

                            for (int i = 0; i <ECELL_DIRECTION_MAXCOUNT; ++i)
                            {
                                    cellID[i] = ComputeCellID(pCellInfo->GetID(), (ECELL_DIRECTION)i);
                                    aroundCell[i] = GetCellInfo(pCellInfo->GetSceneID(), pCellInfo->GetGroupID(), cellID[i]);
                            }

                            pCellInfo->Start(aroundCell);

                            pCellInfo = nullptr;
                    }

            }
    }
    */
    return false;
}

const bool CellModule::DestroyGroupCell(const int &sceneID, const int &groupID) {
    TMAP_SCENE_INFO::iterator it = mtCellInfoMap.find(sceneID);
    if (it != mtCellInfoMap.end()) {
        TMAP_GROUP_INFO::iterator itGroup = it->second.find(groupID);
        if (itGroup != it->second.end()) {
            it->second.erase(groupID);
        }
    }

    return true;
}

const Guid CellModule::OnObjectMove(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromCell, const Guid &toCell) {
    if (fromCell == toCell) {
        return toCell;
    }

    OnMoveEvent(self, sceneID, groupID, fromCell, toCell);

    return toCell;
}

const Guid CellModule::OnObjectEntry(const Guid &self, const int &sceneID, const int &groupID, const Guid &toCell) {
    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, toCell);
    if (pCellInfo) {
        OnMoveInEvent(self, sceneID, groupID, toCell);
    }

    return toCell;
}

const Guid CellModule::OnObjectLeave(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromCell) {
    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, fromCell);
    if (pCellInfo) {
        OnMoveOutEvent(self, sceneID, groupID, fromCell);
    }

    return Guid();
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, list);
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const Guid &noSelf, ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, list, noSelf);
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const bool bPlayer, ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, list, bPlayer);
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const bool bPlayer, const Guid &noSelf,
                                   ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, list, bPlayer, noSelf);
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, const std::string &className, DataList &list,
                                   ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, className, list);
}

bool CellModule::GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, const std::string &className, DataList &list, const Guid &noSelf,
                                   ECELL_AROUND around) {
    return m_kernel_->GetGroupObjectList(sceneID, groupID, className, list, noSelf);
}

const Guid CellModule::ComputeCellID(const int nX, const int nY, const int nZ) {
    int nGridXIndex = nX / nCellWidth;
    int nGridZIndex = nZ / nCellWidth;

    return Guid(nGridXIndex, nGridZIndex);
}

const Guid CellModule::ComputeCellID(const Vector3 &vec) { return ComputeCellID((int)vec.X(), (int)vec.Y(), (int)vec.Z()); }

const Guid CellModule::ComputeCellID(const Guid &selfGrid, ECELL_DIRECTION eDirection) {
    switch (eDirection) {
    case ECELL_TOP:
        return Guid(selfGrid.nHead64, selfGrid.nData64 + 1);
        break;
    case ECELL_DOWN:
        return Guid(selfGrid.nHead64, selfGrid.nData64 - 1);
        break;
    case ECELL_LEFT:
        return Guid(selfGrid.nHead64 - 1, selfGrid.nData64);
        break;
    case ECELL_RIGHT:
        return Guid(selfGrid.nHead64 + 1, selfGrid.nData64);
        break;
    case ECELL_LEFT_TOP:
        return Guid(selfGrid.nHead64 - 1, selfGrid.nData64 + 1);
        break;
    case ECELL_LEFT_DOWN:
        return Guid(selfGrid.nHead64 - 1, selfGrid.nData64 - 1);
        break;
    case ECELL_RIGHT_TOP:
        return Guid(selfGrid.nHead64 + 1, selfGrid.nData64 + 1);
        break;
    case ECELL_RIGHT_DOWN:
        return Guid(selfGrid.nHead64 + 1, selfGrid.nData64 - 1);
        break;
    default:
        break;
    }
    return Guid();
}

const Guid CellModule::GetStepLenth(const Guid &selfGrid, const Guid &otherGrid) {
    return Guid(std::abs(otherGrid.nHead64 - selfGrid.nHead64), std::abs(otherGrid.nData64 - selfGrid.nData64));
}

const int CellModule::GetAroundCell(const int &sceneID, const int &groupID, const Guid &selfGrid, std::shared_ptr<SceneCellInfo> *gridList,
                                    ECELL_AROUND around /*= ECELL_AROUND_9 */) {
    std::shared_ptr<SceneCellInfo> pGridInfo = GetCellInfo(sceneID, groupID, selfGrid);
    return GetAroundCell(pGridInfo, gridList, around);
}

const int CellModule::GetAroundCell(std::shared_ptr<SceneCellInfo> pGridInfo, std::shared_ptr<SceneCellInfo> *gridList,
                                    ECELL_AROUND around /*= ECELL_AROUND_9 */) {
    int nObjectCount = 0;

    if (!pGridInfo) {
        return nObjectCount;
    }

    gridList[ECELL_DIRECTION_MAXCOUNT] = pGridInfo;

    nObjectCount += pGridInfo->Count();

    switch (around) {
    case ECELL_AROUND_9: {
        for (int i = 0; i < ECELL_DIRECTION_MAXCOUNT; i++) {
            std::shared_ptr<SceneCellInfo> pInfo = pGridInfo->GetConnectCell((ECELL_DIRECTION)i);
            if (pInfo) {
                gridList[i] = pInfo;
                nObjectCount += pInfo->Count();
            }
        }
    } break;
    case ECELL_AROUND_16:
        break;
    case ECELL_AROUND_25:
        break;
    case ECELL_AROUND_ALL:
        break;
    default:
        break;
    }

    return nObjectCount;
}

const int CellModule::GetAroundObject(const int &sceneID, const int &groupID, const Guid &selfCell, DataList &objectList,
                                      ECELL_AROUND around /*= ECELL_AROUND_9 */) {
    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, selfCell);
    if (pCellInfo) {
        return GetAroundObject(pCellInfo, objectList, around);
    }
    return 0;
}

const int CellModule::GetAroundObject(std::shared_ptr<SceneCellInfo> pCellInfo, DataList &objectList, ECELL_AROUND around /*= ECELL_AROUND_9 */) {
    if (!pCellInfo) {
        return 0;
    }

    std::shared_ptr<SceneCellInfo> aroundCell[ECELL_DIRECTION_MAXCOUNT];
    if (GetAroundCell(pCellInfo, aroundCell, around) > 0) {
        for (int i = 0; i < ECELL_DIRECTION_MAXCOUNT; i++) {
            std::shared_ptr<SceneCellInfo> pCellInfo = aroundCell[i];
            if (pCellInfo) {
                Guid ident;
                bool bRet = pCellInfo->First(ident);
                while (bRet) {
                    objectList.Add(ident);
                    bRet = pCellInfo->Next(ident);
                }
            }
        }
    }

    return objectList.GetCount();
}

int CellModule::AddMoveEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) {
    mMoveEventHandler.push_back(functorPtr);

    return 0;
}

int CellModule::AddMoveInEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) {
    mMoveInEventHandler.push_back(functorPtr);
    return 0;
}

int CellModule::AddMoveOutEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) {
    mMoveOutEventHandler.push_back(functorPtr);
    return 0;
}

int CellModule::OnObjectEvent(const Guid &self, const std::string &classNames, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
        m_kernel_->AddPropertyCallBack(self, excel::IObject::Position(), this, &CellModule::OnPositionEvent);
    }

    return 0;
}

int CellModule::OnPositionEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar, const INT64 reason) {
    const int sceneID = m_kernel_->GetPropertyInt32(self, excel::IObject::SceneID());
    const int groupID = m_kernel_->GetPropertyInt32(self, excel::IObject::GroupID());
    const Vector3 &oldVec = oldVar.GetVector3();
    const Vector3 &newVec = newVar.GetVector3();

    Guid fromCell = ComputeCellID(oldVec);
    Guid toCell = ComputeCellID(newVec);
    OnObjectMove(self, sceneID, groupID, fromCell, toCell);
    return 0;
}

std::shared_ptr<SceneCellInfo> CellModule::GetConnectCell(const int &sceneID, const int &groupID, const Guid &selfGrid, ECELL_DIRECTION eDirection) {
    std::shared_ptr<SceneCellInfo> pGridInfo = GetCellInfo(sceneID, groupID, selfGrid);
    if (pGridInfo) {
        return pGridInfo->GetConnectCell(eDirection);
    }

    return NULL;
}

std::shared_ptr<SceneCellInfo> CellModule::GetCellInfo(const int &sceneID, const int &groupID, const Guid &selfGrid) {
    TMAP_SCENE_INFO::iterator it = mtCellInfoMap.find(sceneID);
    if (it != mtCellInfoMap.end()) {
        TMAP_GROUP_INFO::iterator itGroup = it->second.find(groupID);
        if (itGroup != it->second.end()) {
            TMAP_CELL_INFO::iterator itCell = itGroup->second.find(selfGrid);
            if (itCell != itGroup->second.end()) {
                return itCell->second;
            }
        }
    }

    return NULL;
}

bool CellModule::OnMoveEvent(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromCell, const Guid &toCell) {
    if (groupID <= 0) {
        return false;
    }

    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, toCell);
    if (pCellInfo) {
        pCellInfo->Add(self);
    }

    Guid lastCell = fromCell;

    if (!fromCell.IsNull()) {
        std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, fromCell);
        if (pCellInfo) {
            if (!pCellInfo->Exist(self)) {
                lastCell = Guid();
            }
        } else {
            lastCell = Guid();
        }
    }

    // BC
    for (int i = 0; i < mMoveEventHandler.size(); ++i) {
        CELL_MOVE_EVENT_FUNCTOR_PTR pFunPtr = mMoveEventHandler[i];
        CELL_MOVE_EVENT_FUNCTOR *pFun = pFunPtr.get();
        pFun->operator()(self, sceneID, groupID, lastCell, toCell);
    }
    return true;
}

bool CellModule::OnMoveInEvent(const Guid &self, const int &sceneID, const int &groupID, const Guid &toCell) {
    if (groupID <= 0) {
        return false;
    }

    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, toCell);
    if (pCellInfo) {
        pCellInfo->Add(self);
    }

    // BC
    for (int i = 0; i < mMoveInEventHandler.size(); ++i) {
        CELL_MOVE_EVENT_FUNCTOR_PTR pFunPtr = mMoveEventHandler[i];
        CELL_MOVE_EVENT_FUNCTOR *pFun = pFunPtr.get();
        pFun->operator()(self, sceneID, groupID, Guid(), toCell);
    }

    return true;
}

bool CellModule::OnMoveOutEvent(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromCell) {
    if (groupID <= 0) {
        return false;
    }

    std::shared_ptr<SceneCellInfo> pCellInfo = GetCellInfo(sceneID, groupID, fromCell);
    if (pCellInfo) {
        pCellInfo->Remove(self);
    }

    // BC
    for (int i = 0; i < mMoveOutEventHandler.size(); ++i) {
        CELL_MOVE_EVENT_FUNCTOR_PTR pFunPtr = mMoveEventHandler[i];
        CELL_MOVE_EVENT_FUNCTOR *pFun = pFunPtr.get();
        pFun->operator()(self, sceneID, groupID, fromCell, Guid());
    }

    return true;
}

int CellModule::BeforeLeaveSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) { return 0; }

int CellModule::AfterEnterSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) { return 0; }
