

#ifndef SQUICK_INTF_DATA_TAIL_MODULE_H
#define SQUICK_INTF_DATA_TAIL_MODULE_H

#include <iostream>
#include <squick/core/i_module.h>

class IDataTailModule
    : public IModule
{
public:

	virtual void LogObjectData(const Guid& self) = 0;

    virtual void StartTrail(const Guid& self) = 0;

};


#endif
