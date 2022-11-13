#include "shopping_item.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void ShoppingItem::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `ShoppingItem` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`sku` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`price` DECIMAL(10, 2),"
                        << "PRIMARY KEY (`id`), UNIQUE KEY `s` (`sku`), KEY `n` (`name`));",
                now;
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

    Poco::JSON::Object::Ptr ShoppingItem::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("sku", _sku);
        root->set("name", _name);
        root->set("price", _price);

        return root;
    }

    ShoppingItem ShoppingItem::fromJSON(const std::string &str)
    {
        ShoppingItem shoppingItem;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        shoppingItem.id() = object->getValue<long>("id");
        shoppingItem.sku() = object->getValue<std::string>("sku");
        shoppingItem.name() = object->getValue<std::string>("name");
        shoppingItem.price() = object->getValue<double>("price");

        return shoppingItem;
    }

    ShoppingItem ShoppingItem::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            ShoppingItem a;
            select << "SELECT id, sku, name, price FROM ShoppingItem where id=?",
                into(a._id),
                into(a._sku),
                into(a._name),
                into(a._price),
                use(id),
                range(0, 1); //  iterate over result set one row at a time
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return a;
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

    ShoppingItem ShoppingItem::read_by_sku(std::string sku)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            ShoppingItem a;
            select << "SELECT id, sku, name, price FROM ShoppingItem where sku=?",
                into(a._id),
                into(a._sku),
                into(a._name),
                into(a._price),
                use(sku),
                range(0, 1); //  iterate over result set one row at a time
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return a;
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

    std::vector<ShoppingItem> ShoppingItem::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<ShoppingItem> result;
            ShoppingItem a;
            select << "SELECT id, sku, name, price FROM ShoppingItem",
                into(a._id),
                into(a._sku),
                into(a._name),
                into(a._price),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())
                result.push_back(a);
            }
            return result;
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

    std::vector<ShoppingItem> ShoppingItem::search(std::string name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<ShoppingItem> result;
            ShoppingItem a;
            name+="%";
            select << "SELECT id, sku, name, price FROM ShoppingItem where name LIKE ?",
                into(a._id),
                into(a._sku),
                into(a._name),
                into(a._price),
                use(name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())  result.push_back(a);
            }
            return result;
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

   
    void ShoppingItem::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO ShoppingItem (sku, name, price) VALUES(?, ?, ?)",
                use(_sku),
                use(_name),
                use(_price);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
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

    long ShoppingItem::get_id() const
    {
        return _id;
    }

    const std::string &ShoppingItem::get_sku() const
    {
        return _sku;
    }

    const std::string &ShoppingItem::get_name() const
    {
        return _name;
    }

    double ShoppingItem::get_price() const
    {
        return _price;
    }

    long &ShoppingItem::id()
    {
        return _id;
    }

    std::string &ShoppingItem::sku()
    {
        return _sku;
    }

    std::string &ShoppingItem::name()
    {
        return _name;
    }

    double &ShoppingItem::price()
    {
        return _price;
    }

}