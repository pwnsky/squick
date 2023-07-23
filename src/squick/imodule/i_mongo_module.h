// Au// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-07-23
// Description: High availability of base mongodb interface
#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>


#include <cstdint>
#include <iostream>
#include <vector>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

namespace imodule {
    class IMongoModule : virtual public IModule {
    public:
        bool Connect(std::string url) {
            try {
                // Create an instance.
                mongocxx::instance inst{};

                // Replace the connection string with your MongoDB deployment's connection string.
                const auto uri = mongocxx::uri{ "mongodb://127.0.0.1:9999" };

                // Setup the connection and get a handle on the "admin" database.
                mongocxx::client conn{ uri };
                mongocxx::database db = conn["admin"];

                // Ping the database.
                const auto ping_cmd = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("ping", 1));
                db.run_command(ping_cmd.view());
                std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << std::endl;
            }
            catch (const std::exception& e) {
                // Handle errors.
                std::cout << "Exception: " << e.what() << std::endl;
            }
            return true;
        }
    };
}