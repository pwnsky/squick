

#include "property_manager.h"
#include "property.h"

PropertyManager::~PropertyManager() { ClearAll(); }

bool PropertyManager::RegisterCallback(const std::string &propertyName, const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    SQUICK_SHARE_PTR<IProperty> pProperty = this->GetElement(propertyName);
    if (pProperty) {
        pProperty->RegisterCallback(cb);
        return true;
    }

    return false;
}

SQUICK_SHARE_PTR<IProperty> PropertyManager::AddProperty(const Guid &self, SQUICK_SHARE_PTR<IProperty> pProperty) {
    const std::string &propertyName = pProperty->GetKey();
    SQUICK_SHARE_PTR<IProperty> pOldProperty = this->GetElement(propertyName);
    if (!pOldProperty) {
        SQUICK_SHARE_PTR<IProperty> pNewProperty(SQUICK_NEW Property(self, propertyName, pProperty->GetType()));

        pNewProperty->SetPublic(pProperty->GetPublic());
        pNewProperty->SetPrivate(pProperty->GetPrivate());
        pNewProperty->SetSave(pProperty->GetSave());
        pNewProperty->SetCache(pProperty->GetCache());
        pNewProperty->SetRef(pProperty->GetRef());
        pNewProperty->SetUpload(pProperty->GetUpload());

        this->AddElement(propertyName, pNewProperty);
    }

    return pOldProperty;
}

SQUICK_SHARE_PTR<IProperty> PropertyManager::AddProperty(const Guid &self, const std::string &propertyName, const DATA_TYPE varType) {
    SQUICK_SHARE_PTR<IProperty> pProperty = this->GetElement(propertyName);
    if (!pProperty) {
        pProperty = SQUICK_SHARE_PTR<IProperty>(SQUICK_NEW Property(self, propertyName, varType));

        this->AddElement(propertyName, pProperty);
    }

    return pProperty;
}

bool PropertyManager::SetProperty(const std::string &propertyName, const SquickData &TData) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        pProperty->SetValue(TData);

        return true;
    }

    return false;
}

const Guid &PropertyManager::Self() { return mSelf; }

std::string PropertyManager::ToString() {
    std::string s;
    std::stringstream stream;
    SQUICK_SHARE_PTR<IProperty> pProperty = First(s);
    while (pProperty) {
        stream << s << ":" << pProperty->ToString() << "|";
        pProperty = Next(s);
    }

    return stream.str();
}

bool PropertyManager::SetPropertyInt(const std::string &propertyName, const INT64 nValue) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetInt(nValue);
    }

    return false;
}

bool PropertyManager::SetPropertyFloat(const std::string &propertyName, const double dwValue) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetFloat(dwValue);
    }

    return false;
}

bool PropertyManager::SetPropertyString(const std::string &propertyName, const std::string &value) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetString(value);
    }

    return false;
}

bool PropertyManager::SetPropertyObject(const std::string &propertyName, const Guid &obj) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetObject(obj);
    }

    return false;
}

bool PropertyManager::SetPropertyVector2(const std::string &propertyName, const Vector2 &value) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetVector2(value);
    }

    return false;
}

bool PropertyManager::SetPropertyVector3(const std::string &propertyName, const Vector3 &value) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->SetVector3(value);
    }

    return false;
}

INT64 PropertyManager::GetPropertyInt(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    return pProperty ? pProperty->GetInt() : 0;
}

int PropertyManager::GetPropertyInt32(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    return pProperty ? pProperty->GetInt32() : 0;
}

double PropertyManager::GetPropertyFloat(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    return pProperty ? pProperty->GetFloat() : 0.0;
}

const std::string &PropertyManager::GetPropertyString(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->GetString();
    }

    return NULL_STR;
}

const Guid &PropertyManager::GetPropertyObject(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->GetObject();
    }

    return NULL_OBJECT;
}

const Vector2 &PropertyManager::GetPropertyVector2(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->GetVector2();
    }

    return NULL_VECTOR2;
}

const Vector3 &PropertyManager::GetPropertyVector3(const std::string &propertyName) {
    SQUICK_SHARE_PTR<IProperty> pProperty = GetElement(propertyName);
    if (pProperty) {
        return pProperty->GetVector3();
    }

    return NULL_VECTOR3;
}
