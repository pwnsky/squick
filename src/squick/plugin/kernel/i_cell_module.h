

#ifndef SQUICK_INTF_CELL_MODULE_H
#define SQUICK_INTF_CELL_MODULE_H

#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <squick/core/guid.h>
#include <squick/core/i_module.h>

class SceneCellInfo;

enum ECELL_DIRECTION {
    ECELL_TOP = 0,
    ECELL_DOWN = 1,
    ECELL_LEFT = 2,
    ECELL_RIGHT = 3,

    ECELL_LEFT_TOP = 4,
    ECELL_LEFT_DOWN = 5,
    ECELL_RIGHT_TOP = 6,
    ECELL_RIGHT_DOWN = 7,

    ECELL_DIRECTION_MAXCOUNT = 8,
};

enum ECELL_AROUND {
    ECELL_SELF,
    ECELL_AROUND_9,
    ECELL_AROUND_16,
    ECELL_AROUND_25,
    ECELL_AROUND_ALL,
};
// id, scene_id, group_id, move_out_cell_id, move_in_cell_id
typedef std::function<int(const Guid &, const int &, const int &, const Guid &, const Guid &)> CELL_MOVE_EVENT_FUNCTOR;
typedef SQUICK_SHARE_PTR<CELL_MOVE_EVENT_FUNCTOR> CELL_MOVE_EVENT_FUNCTOR_PTR; // EVENT

class ICellModule : public IModule {
  public:
    virtual ~ICellModule() {}

    virtual const Guid ComputeCellID(const int nX, const int nY, const int nZ) = 0;
    virtual const Guid ComputeCellID(const Vector3 &vec) = 0;

    virtual const bool CreateGroupCell(const int &sceneID, const int &groupID) = 0;
    virtual const bool DestroyGroupCell(const int &sceneID, const int &groupID) = 0;

    // the event that a object are moving
    virtual const Guid OnObjectMove(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromGrid, const Guid &toCell) = 0;

    // the event that a object has entried
    virtual const Guid OnObjectEntry(const Guid &self, const int &sceneID, const int &groupID, const Guid &toCell) = 0;

    // the event that a object has leaved
    virtual const Guid OnObjectLeave(const Guid &self, const int &sceneID, const int &groupID, const Guid &fromCell) = 0;

    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, ECELL_AROUND around = ECELL_AROUND_9) = 0;
    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const Guid &noSelf,
                                   ECELL_AROUND around = ECELL_AROUND_9) = 0;
    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const bool bPlayer,
                                   ECELL_AROUND around = ECELL_AROUND_9) = 0;
    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, DataList &list, const bool bPlayer, const Guid &noSelf,
                                   ECELL_AROUND around = ECELL_AROUND_9) = 0;
    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, const std::string &className, DataList &list,
                                   ECELL_AROUND around = ECELL_AROUND_9) = 0;
    virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3 &pos, const std::string &className, DataList &list, const Guid &noSelf,
                                   ECELL_AROUND around = ECELL_AROUND_9) = 0;

    //////////////////////////////////////////////////////////////////////////

    template <typename BaseType>
    bool AddMoveEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int &, const int &, const Guid &, const Guid &)) {
        CELL_MOVE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr(new CELL_MOVE_EVENT_FUNCTOR(functor));

        AddMoveEventCallBack(functorPtr);
        return true;
    }
    template <typename BaseType>
    bool AddMoveInEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int &, const int &, const Guid &, const Guid &)) {
        CELL_MOVE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr(new CELL_MOVE_EVENT_FUNCTOR(functor));

        AddMoveInEventCallBack(functorPtr);
        return true;
    }

    template <typename BaseType>
    bool AddMoveOutEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int &, const int &, const Guid &, const Guid &)) {
        CELL_MOVE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr(new CELL_MOVE_EVENT_FUNCTOR(functor));

        AddMoveOutEventCallBack(functorPtr);
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
  protected:
    virtual int AddMoveEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddMoveInEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddMoveOutEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr) = 0;

  private:
};

#endif
