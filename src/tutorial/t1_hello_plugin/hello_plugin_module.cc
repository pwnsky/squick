
#include "hello_plugin_module.h"

bool HelloWorld1::Start()
{
	std::cout << typeid(HelloWorld1).name() << std::endl;

    
    std::cout << "Hello, world1, Start" << std::endl;

    return true;
}

bool HelloWorld1::AfterStart()
{
    
    std::cout << "Hello, world1, AfterStart" << std::endl;


	DataList dataList;
	dataList.Add("1");
	dataList.AddFloat(2.0f);
	dataList.AddObject(Guid(3,3));
	dataList.AddVector2(Vector2(4.0f, 4.0f));

	for (int i = 0; i < dataList.GetCount(); ++i)
	{
		std::cout << dataList.ToString(i) << std::endl;
	}


    return true;
}

// Update
bool HelloWorld1::Update()
{
    
    //std::cout << "Hello, world1, Update" << std::endl;

    return true;
}

bool HelloWorld1::BeforeDestory()
{
    std::cout << "Hello, world1, BeforeDestory1111" << std::endl;

    system("PAUSE");

    
    std::cout << "Hello, world1, BeforeDestory" << std::endl;

    return true;
}

bool HelloWorld1::Destory()
{
    
    std::cout << "Hello, world1, Destory" << std::endl;

    return true;
}
