
#include "error_bus_module.h"

IErrorBusModule::ErrorData ErrorBusModule::GetLastError()
{
	if (mErrList.size() > 0)
	{
		return mErrList.back();
	}

	return ErrorData();
}

void ErrorBusModule::AddError(const Guid object, const ErrID err, const std::string & stData)
{
	ErrorData xErrData;
	xErrData.data = stData;
	xErrData.errID = err;
	xErrData.id = object;

	mErrList.push_back(xErrData);

	if (mErrList.size() > 1000)
	{
		auto it = mErrList.end();
		std::advance(it, -100);
		mErrList.erase(mErrList.begin(), it);
	}
}

void ErrorBusModule::ClearAllError()
{
	mErrList.clear();
}

