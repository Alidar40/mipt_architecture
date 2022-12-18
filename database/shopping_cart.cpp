#include "shopping_cart.h"
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

    void ShoppingCart::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session_write();

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `ShoppingCart` ("
                        << "`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`user_id` INT NOT NULL,"
                        << "`created` DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,"
                        << "PRIMARY KEY (`id`),"
                        << "FOREIGN KEY (`user_id`) REFERENCES `User`(`id`) ON UPDATE CASCADE ON DELETE CASCADE);",
                now;

            create_stmt << "CREATE TABLE IF NOT EXISTS `ShoppingItem_ShoppingCart` ("
                        << "`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`shopping_item_id` INT NOT NULL,"
                        << "`shopping_cart_id` INT NOT NULL,"
                        << "PRIMARY KEY (`id`),"
                        << "FOREIGN KEY (`shopping_item_id`) REFERENCES `ShoppingItem`(`id`) ON UPDATE CASCADE ON DELETE CASCADE,"
                        << "FOREIGN KEY (`shopping_cart_id`) REFERENCES `ShoppingCart`(`id`) ON UPDATE CASCADE ON DELETE CASCADE);",
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

    Poco::JSON::Object::Ptr ShoppingCart::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("user_id", _user_id);
        root->set("created", _created);

        return root;
    }

    ShoppingCart ShoppingCart::fromJSON(const std::string &str)
    {
        ShoppingCart shoppingCart;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        shoppingCart.id() = object->getValue<long>("id");
        shoppingCart.user_id() = object->getValue<long>("user_id");
        shoppingCart.created() = object->getValue<std::string>("created");

        return shoppingCart;
    }

    ShoppingCart ShoppingCart::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Poco::Data::Statement select(session);
            ShoppingCart a;
            select << "SELECT id, user_id, created FROM ShoppingCart WHERE id LIKE ?",
                into(a._id),
                into(a._user_id),
                into(a._created),
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

    ShoppingCart ShoppingCart::read_by_user_id(long user_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Poco::Data::Statement select(session);
            ShoppingCart a;
            select << "SELECT id, user_id, created FROM ShoppingCart WHERE user_id LIKE ? ORDER BY created DESC LIMIT 1",
                into(a._id),
                into(a._user_id),
                into(a._created),
                use(user_id),
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

    std::vector<ShoppingCart> ShoppingCart::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Statement select(session);
            std::vector<ShoppingCart> result;
            ShoppingCart a;
            select << "SELECT id, user_id, created FROM ShoppingCart",
                into(a._id),
                into(a._user_id),
                into(a._created),
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

    std::vector<ShoppingCart> ShoppingCart::read_all_by_user_id(long user_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Statement select(session);
            std::vector<ShoppingCart> result;
            ShoppingCart a;
            select << "SELECT id, user_id, created FROM ShoppingCart WHERE user_id LIKE ?",
                into(a._id),
                into(a._user_id),
                into(a._created),
                use(user_id),
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

    void ShoppingCart::add_to_cart(long shopping_item_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_write();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO ShoppingItem_ShoppingCart (shopping_item_id, shopping_cart_id) VALUES(?, ?)",
                use(_id),
                use(shopping_item_id);

            insert.execute();

            std::cout << "inserted in shopping cart:" << _id << ", item:" << shopping_item_id << std::endl;
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

    void ShoppingCart::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session_write();
            Poco::Data::Statement insert(session);

            // insert << "INSERT INTO ShoppingCart (user_id, created) VALUES(?, ?, ?, ?)",
            //     use(_user_id),
            //     use(_created);
            insert << "INSERT INTO ShoppingCart (user_id) VALUES(?)",
                use(_user_id);

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

    long ShoppingCart::get_id() const
    {
        return _id;
    }

    long ShoppingCart::get_user_id() const
    {
        return _user_id;
    }

    const std::string &ShoppingCart::get_created() const
    {
        return _created;
    }

    long &ShoppingCart::id()
    {
        return _id;
    }

    long &ShoppingCart::user_id()
    {
        return _user_id;
    }

    std::string &ShoppingCart::created()
    {
        return _created;
    }

}