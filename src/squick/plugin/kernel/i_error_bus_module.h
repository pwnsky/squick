


#ifndef SQUICK_INTF_ERROR_BUS_MODULE_H
#define SQUICK_INTF_ERROR_BUS_MODULE_H

#include <iostream>
#include <squick/core/guid.h>
#include <squick/core/i_module.h>


class IErrorBusModule
    : public IModule
{

public:
	enum ErrID
	{
		ERR_NONE,
	};

	struct ErrorData
	{
		ErrorData()
		{
			errID = ErrID::ERR_NONE;
		}

		Guid id;
		ErrID errID;
		std::string data;
	};

	virtual ErrorData GetLastError() = 0;

	virtual void AddError(const Guid object, const ErrID err, const std::string& stData) = 0;

	virtual void ClearAllError() = 0;
};

#endif