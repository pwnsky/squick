
#include "property.h"
#include <complex>

Property::Property() {
    mbPublic = false;
    mbPrivate = false;
    mbSave = false;
    mbCache = false;
    mbRef = false;
    mbForce = false;
    mbUpload = false;

    mSelf = Guid();
    eType = TDATA_UNKNOWN;

    msPropertyName = "";
}

Property::Property(const Guid &self, const std::string &propertyName, const DATA_TYPE varType) {
    mbPublic = false;
    mbPrivate = false;
    mbSave = false;
    mbCache = false;
    mbRef = false;
    mbUpload = false;

    mSelf = self;

    msPropertyName = propertyName;
    eType = varType;
}

Property::~Property() {
    for (TPROPERTYCALLBACKEX::iterator iter = mtPropertyCallback.begin(); iter != mtPropertyCallback.end(); ++iter) {
        iter->reset();
    }

    mtPropertyCallback.clear();
    mxData.reset();
}

void Property::SetValue(const SquickData &xData) {
    if (eType != xData.GetType() || xData.GetType() == DATA_TYPE::TDATA_UNKNOWN) {
        return;
    }

    if (xData.IsNullValue()) {
        return;
    }

    if (nullptr == mxData) {
        mxData = std::shared_ptr<SquickData>(new SquickData(xData));
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->variantData = xData.variantData;
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->variantData = xData.variantData;

        SquickData newValue;
        newValue = *mxData;

        OnEventHandler(oldValue, newValue, 0);
    }
}

void Property::SetValue(const IProperty *property) { SetValue(property->GetValue()); }

const SquickData &Property::GetValue() const {
    if (mxData) {
        return *mxData;
    }

    return NULL_TDATA;
}

const std::string &Property::GetKey() const { return msPropertyName; }

INT64 Property::GetInt() const {
    if (!mxData) {
        return 0;
    }

    return mxData->GetInt();
}

int Property::GetInt32() const {
    if (!mxData) {
        return 0;
    }

    return (int)mxData->GetInt();
}

double Property::GetFloat() const {
    if (!mxData) {
        return 0.0;
    }

    return mxData->GetFloat();
}

const std::string &Property::GetString() const {
    if (!mxData) {
        return NULL_STR;
    }

    return mxData->GetString();
}

const Guid &Property::GetObject() const {
    if (!mxData) {
        return NULL_OBJECT;
    }

    return mxData->GetObject();
}

const Vector2 &Property::GetVector2() const {
    if (!mxData) {
        return NULL_VECTOR2;
    }

    return mxData->GetVector2();
}

const Vector3 &Property::GetVector3() const {
    if (!mxData) {
        return NULL_VECTOR3;
    }

    return mxData->GetVector3();
}

void Property::RegisterCallback(const PROPERTY_EVENT_FUNCTOR_PTR &cb) { mtPropertyCallback.push_back(cb); }

int Property::OnEventHandler(const SquickData &oldVar, const SquickData &newVar, const INT64 reason) {
    if (mtPropertyCallback.size() <= 0) {
        return 0;
    }

    TPROPERTYCALLBACKEX::iterator it = mtPropertyCallback.begin();
    TPROPERTYCALLBACKEX::iterator end = mtPropertyCallback.end();
    for (; it != end; ++it) {

        PROPERTY_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        PROPERTY_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(mSelf, msPropertyName, oldVar, newVar, reason);
    }

    return 0;
}

bool Property::SetInt(const INT64 value, const INT64 reason) {
    if (eType != TDATA_INT) {
        return false;
    }

    if (!mxData) {

        if (0 == value) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_INT));
        mxData->SetInt(0);
    }

    if (value == mxData->GetInt()) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetInt(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetInt(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::SetFloat(const double value, const INT64 reason) {
    if (eType != TDATA_FLOAT) {
        return false;
    }

    if (!mxData) {

        if (IsZeroDouble(value)) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_FLOAT));
        mxData->SetFloat(0.0);
    }

    if (IsZeroDouble(value - mxData->GetFloat())) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetFloat(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetFloat(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::SetString(const std::string &value, const INT64 reason) {
    if (eType != TDATA_STRING) {
        return false;
    }

    if (!mxData) {

        if (value.empty()) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_STRING));
        mxData->SetString(NULL_STR);
    }

    if (value == mxData->GetString()) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetString(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetString(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::SetObject(const Guid &value, const INT64 reason) {
    if (eType != TDATA_OBJECT) {
        return false;
    }

    if (!mxData) {

        if (value.IsNull()) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_OBJECT));
        mxData->SetObject(Guid());
    }

    if (value == mxData->GetObject()) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetObject(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetObject(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::SetVector2(const Vector2 &value, const INT64 reason) {
    if (eType != TDATA_VECTOR2) {
        return false;
    }

    if (!mxData) {

        if (value.IsZero()) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_VECTOR2));
        mxData->SetVector2(Vector2());
    }

    if (value == mxData->GetVector2()) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetVector2(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetVector2(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::SetVector3(const Vector3 &value, const INT64 reason) {
    if (eType != TDATA_VECTOR3) {
        return false;
    }

    if (!mxData) {

        if (value.IsZero()) {
            return false;
        }

        mxData = std::shared_ptr<SquickData>(new SquickData(TDATA_VECTOR3));
        mxData->SetVector3(Vector3());
    }

    if (value == mxData->GetVector3()) {
        return false;
    }

    if (mtPropertyCallback.size() == 0) {
        mxData->SetVector3(value);
    } else {
        SquickData oldValue;
        oldValue = *mxData;

        mxData->SetVector3(value);

        OnEventHandler(oldValue, *mxData, reason);
    }

    return true;
}

bool Property::Changed() const { return !(GetValue().IsNullValue()); }

const DATA_TYPE Property::GetType() const { return eType; }

const bool Property::GeUsed() const {
    if (mxData) {
        return true;
    }

    return false;
}

std::string Property::ToString() {
    std::string strData;
    const DATA_TYPE eType = GetType();
    switch (eType) {
    case TDATA_INT:
        strData = lexical_cast<std::string>(GetInt());
        break;
    case TDATA_FLOAT:
        strData = lexical_cast<std::string>(GetFloat());
        break;
    case TDATA_STRING:
        strData = GetString();
        break;
    case TDATA_OBJECT:
        strData = GetObject().ToString();
        break;
    case TDATA_VECTOR2:
        strData = GetVector2().ToString();
        break;
    case TDATA_VECTOR3:
        strData = GetVector3().ToString();
        break;
    default:
        strData = NULL_STR;
        break;
    }

    return strData;
}

void Property::ToMemoryCounterString(std::string &data) {
    data.append(this->mSelf.ToString());
    data.append(":");
    data.append(this->msPropertyName);
}

bool Property::FromString(const std::string &strData) {
    try {
        switch (GetType()) {
        case TDATA_INT:
            SetInt(lexical_cast<int64_t>(strData));
            break;

        case TDATA_FLOAT:
            SetFloat(lexical_cast<float>(strData));
            break;

        case TDATA_STRING:
            SetString(strData);
            break;

        case TDATA_OBJECT: {
            Guid xID;
            xID.FromString(strData);
            SetObject(xID);
        } break;

        case TDATA_VECTOR2: {
            Vector2 v;
            v.FromString(strData);
            SetVector2(v);
        } break;

        case TDATA_VECTOR3: {
            Vector3 v;
            v.FromString(strData);
            SetVector3(v);
        } break;

        default:
            break;
        }

        return true;
    } catch (...) {
        return false;
    }

    return false;
}

bool Property::DeSerialization() {
    bool bRet = false;

    const DATA_TYPE eType = GetType();
    if (eType == TDATA_STRING && nullptr != mxData && !mxData->IsNullValue()) {
        DataList xDataList;
        const std::string &strData = mxData->GetString();

        xDataList.Split(strData.c_str(), ";");
        if (xDataList.GetCount() <= 0) {
            return bRet;
        }

        DataList xTemDataList;
        xTemDataList.Split(xDataList.String(0).c_str(), ",");
        const int nSubDataLength = xTemDataList.GetCount();

        if (xDataList.GetCount() == 1 && nSubDataLength == 1) {
            // most of property value only one value
            return bRet;
        }

        if (nullptr == mxEmbeddedList) {
            mxEmbeddedList = std::shared_ptr<List<std::string>>(new List<std::string>());
        } else {
            mxEmbeddedList->ClearAll();
        }

        for (int i = 0; i < xDataList.GetCount(); ++i) {
            if (xDataList.String(i).empty()) {
                SQUICK_ASSERT(0, strData, __FILE__, __FUNCTION__);
            }

            mxEmbeddedList->Add(xDataList.String(i));
        }

        ////////////////////////////////////////

        if (nSubDataLength < 2 || nSubDataLength > 2) {
            return bRet;
        }

        if (nullptr == mxEmbeddedMap) {
            mxEmbeddedMap = std::shared_ptr<MapEx<std::string, std::string>>(new MapEx<std::string, std::string>());
        } else {
            mxEmbeddedMap->ClearAll();
        }

        for (int i = 0; i < xDataList.GetCount(); ++i) {
            DataList xTemDataList;
            const std::string &strTemData = xDataList.String(i);
            xTemDataList.Split(strTemData.c_str(), ",");
            {
                if (xTemDataList.GetCount() != nSubDataLength) {
                    SQUICK_ASSERT(0, strTemData, __FILE__, __FUNCTION__);
                }

                const std::string &strKey = xTemDataList.String(0);
                const std::string &value = xTemDataList.String(1);

                if (strKey.empty() || value.empty()) {
                    SQUICK_ASSERT(0, strTemData, __FILE__, __FUNCTION__);
                }

                mxEmbeddedMap->AddElement(strKey, std::shared_ptr<std::string>(new std::string(value)));
            }
        }

        bRet = true;
    }

    return bRet;
}

const std::shared_ptr<List<std::string>> Property::GetEmbeddedList() const { return this->mxEmbeddedList; }

const std::shared_ptr<MapEx<std::string, std::string>> Property::GetEmbeddedMap() const { return this->mxEmbeddedMap; }
