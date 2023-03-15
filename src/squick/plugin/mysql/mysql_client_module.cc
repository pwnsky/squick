
#include "mysql_client_module.h"
#include <mysqlx/xdevapi.h>
#include <string>
using namespace ::mysqlx;

MysqlModule::MysqlModule(IPluginManager* p)
{
    m_bIsUpdate = true;
    pPluginManager = p;

    srand((unsigned)time(NULL));
}
MysqlModule::~MysqlModule() {

}


bool MysqlModule::Start() {
    dout << "Mysql Module Started!  ---------------------------- \n";
    
    return true;
}

bool MysqlModule::AfterStart() {
    Connect("");
    return true;
}

bool MysqlModule::Update() {
    return true;
}

bool MysqlModule::Destory() {
    return true;
}

bool MysqlModule::Connect(std::string url) {
    //std::string curl =;
    try {
        SessionSettings s("mysqlx://root@127.0.0.1");
        Session sess(s);// ("mysqlx://root@127.0.0.1");
        
        dout << "Session accepted, creating collection..." << endl;
        Schema sch = sess.getSchema("test");
        Collection coll = sch.createCollection("c1", true);
        coll.remove("true").execute();
        {
            DbDoc doc(R"({ "name": "foo", "age": 1 })");
            Result add =
                coll.add(doc)
                .add(R"({ "name": "bar", "age": 2, "toys": [ "car", "ball" ] })")
                .add(R"({ "name": "bar", "age": 2, "toys": [ "car", "ball" ] })")
                .add(R"({
                 "name": "baz",
                  "age": 3,
                 "date": { "day": 20, "month": "Apr" }
              })")
                .add(R"({ "_id": "myuuid-1", "name": "foo", "age": 7 })")
                .execute();

            std::list<std::string> ids = add.getGeneratedIds();
            for (std::string id : ids)
                cout << "- added doc with id: " << id << endl;
        }
    }
    catch (const mysqlx::Error& err)
    {
        cout << "ERROR: " << err << endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        cout << "STD EXCEPTION: " << ex.what() << endl;
        return 1;
    }
    catch (const char* ex)
    {
        cout << "EXCEPTION: " << ex << endl;
        return 1;
    }
    catch (std::bad_exception ex)
    {
        cout << "EXCEPTION: " << ex.what() << endl;
        return 1;
    }

    


    return true;
}

void MysqlModule::Test() {

}