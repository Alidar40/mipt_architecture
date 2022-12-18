#include "database.h"
#include "../config/config.h"
#include <functional>

using namespace Poco::Data::Keywords;
using Poco::Data::Statement;

namespace database{
    Database::Database(){
        _connection_string_read += "host=";
        _connection_string_read += Config::get().get_read_request_ip();
        _connection_string_read += ";port=";
        _connection_string_read += Config::get().get_port();
        _connection_string_read += ";user=";
        _connection_string_read += Config::get().get_login();
        _connection_string_read += ";db=";
        _connection_string_read += Config::get().get_database();
        _connection_string_read += ";password=";
        _connection_string_read += Config::get().get_password();

        _connection_string_write += "host=";
        _connection_string_write += Config::get().get_write_request_ip();
        _connection_string_write += ";port=";
        _connection_string_write += Config::get().get_port();
        _connection_string_write += ";user=";
        _connection_string_write += Config::get().get_login();
        _connection_string_write += ";db=";
        _connection_string_write += Config::get().get_database();
        _connection_string_write += ";password=";
        _connection_string_write += Config::get().get_password();

        Poco::Data::MySQL::Connector::registerConnector();
    }

    void Database::drop_all_tables() {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_write();
            for (auto &hint : database::Database::get_all_hints())
            {
                Statement si_st_drop_stmt(session);
                si_st_drop_stmt << "DROP TABLE IF EXISTS ShoppingItem_ShoppingCart;" << hint << "\n", now;

                Statement st_drop_stmt(session);
                st_drop_stmt << "DROP TABLE IF EXISTS ShoppingCart;" << hint << "\n", now;

                Statement si_drop_stmt(session);
                si_drop_stmt << "DROP TABLE IF EXISTS ShoppingItem;" << hint << "\n", now;

                Statement u_drop_stmt(session);
                u_drop_stmt << "DROP TABLE IF EXISTS User;" << hint << "\n", now;

                std::cout << "Droping in shard" << hint << std::endl;
            }
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    size_t Database::get_max_shard(){
        return 3;
    }

    std::vector<std::string> Database::get_all_hints(){
        std::vector<std::string> result;
        for(size_t i=0;i<get_max_shard();++i){
            std::string shard_name = "-- sharding:";
            shard_name += std::to_string(i);
            result.push_back(shard_name);
        }
        return result;
    }

    std::string Database::sharding_hint(std::string login){

        std::string key;
        key = login + ';';

        size_t shard_number = std::hash<std::string>{}(key)%get_max_shard();

        std::string result = "-- sharding:";
        result += std::to_string(shard_number);
        return result;
    }

    Database& Database::get(){
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session_read()
    {
        return Poco::Data::Session(Poco::Data::SessionFactory::instance().create(Poco::Data::MySQL::Connector::KEY, _connection_string_read));
    }

    Poco::Data::Session Database::create_session_write()
    {
        return Poco::Data::Session(Poco::Data::SessionFactory::instance().create(Poco::Data::MySQL::Connector::KEY, _connection_string_write));
    }

}