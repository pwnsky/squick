#pragma once

#include <squick/core/i_module.h>

class INavigationModule : public IModule {
  public:
    virtual bool ExistNavigation(INT64 scendId) = 0;

    virtual bool RemoveNavigation(INT64 scendId) = 0;

    virtual int FindPath(INT64 scendId, const Vector3 &start, const Vector3 &end, std::vector<Vector3> &paths) = 0;

    virtual int FindRandomPointAroundCircle(INT64 scendId, const Vector3 &centerPos, std::vector<Vector3> &points, NFINT32 max_points, float maxRadius) = 0;

    virtual int Raycast(INT64 scendId, const Vector3 &start, const Vector3 &end, std::vector<Vector3> &hitPointVec) = 0;
};
