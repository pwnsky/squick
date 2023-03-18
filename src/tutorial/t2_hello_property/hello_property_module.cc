
#include "hello_property_module.h"
#include <squick/core/data_list.h>
#include <squick/core/object.h>

bool HelloProperty::Start() {

    std::cout << "Hello, world2, Start" << std::endl;
    return true;
}

int HelloProperty::OnPropertyCallBackEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                           const int64_t reason) {

    std::cout << "OnPropertyCallBackEvent Property: " << propertyName << " OldValue: " << oldVar.GetInt() << " NewValue: " << newVar.GetInt() << std::endl;

    return 0;
}

bool HelloProperty::AfterStart() {
    DataList xData;
    xData.AddInt(111);

    std::cout << "Hello, world2, AfterStart" << std::endl;

    // created a object for this test
    IObject *pObject = new Object(Guid(0, 1), pPluginManager);

    // add a property name is "Hello" for this object
    pObject->GetPropertyManager()->AddProperty(pObject->Self(), "Hello", TDATA_STRING);
    // add a property name is "World" for this object
    pObject->GetPropertyManager()->AddProperty(pObject->Self(), "World", TDATA_INT);

    // set the "world" property value as 1111
    pObject->SetPropertyInt("World", 1111);
    int n1 = pObject->GetPropertyInt32("World");

    // get the "world" property value and printf it
    std::cout << "Property World:" << n1 << std::endl;

    const int nProperty1 = pObject->GetPropertyInt32("World");
    std::cout << "Property World:" << nProperty1 << std::endl;

    // add a call back functin for "world" property
    pObject->AddPropertyCallBack("World", this, &HelloProperty::OnPropertyCallBackEvent);

    ////set the "world" property value as 2222[than the function "HelloWorld2::OnPropertyCallBackEvent" will be called]
    pObject->SetPropertyInt("World", 2222);

    // get the "world" property value and printf it
    int n2 = pObject->GetPropertyInt32("World");
    std::cout << "Property World:" << n2 << std::endl;

    const int nProperty2 = pObject->GetPropertyInt32("World");
    std::cout << "Property World:" << nProperty2 << std::endl;

    return true;
}

bool HelloProperty::Update() {

    // std::cout << "Hello, world2, Update" << std::endl;

    return true;
}

bool HelloProperty::BeforeDestory() {

    std::cout << "Hello, world2, BeforeDestory" << std::endl;

    return true;
}

bool HelloProperty::Destory() {

    std::cout << "Hello, world2, Destory" << std::endl;

    return true;
}
