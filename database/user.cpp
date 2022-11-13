#include "user.h"
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
#include <fstream>
#include <future>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            // (re)create table
            for (auto &hint : database::Database::get_all_hints())
            {
                Statement create_stmt(session);
                create_stmt << "CREATE TABLE IF NOT EXISTS `User` (`id` INT NOT NULL AUTO_INCREMENT,"
                            << "`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                            << "`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                            << "`password` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                            << "`type` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                            << "PRIMARY KEY (`id`),KEY `fn` (`first_name`),KEY `ln` (`last_name`));"
                            << hint,
                    now;
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
    
    void User::preload(const std::string &file)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            std::string json;
            std::ifstream is(file);
            std::istream_iterator<char> eos;
            std::istream_iterator<char> iit(is);
            while (iit != eos)
                json.push_back(*(iit++));
            is.close();

            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(json);
            Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

            size_t i{0};
            for (i = 0; i < arr->size(); ++i)
            {
                Poco::JSON::Object::Ptr object = arr->getObject(i);
                std::string first_name = object->getValue<std::string>("first_name");
                std::string last_name = object->getValue<std::string>("last_name");
                std::string email = object->getValue<std::string>("email");
                std::string password = object->getValue<std::string>("password");
                std::string type = object->getValue<std::string>("type");

                std::string hint = database::Database::sharding_hint(email);

                Poco::Data::Statement insert(session);
                insert << "INSERT INTO User (first_name,last_name,email,password,type) VALUES(?, ?, ?, ?, ?)" << hint,
                    Poco::Data::Keywords::use(first_name),
                    Poco::Data::Keywords::use(last_name),
                    Poco::Data::Keywords::use(email),
                    Poco::Data::Keywords::use(password),
                    Poco::Data::Keywords::use(type);

                insert.execute();
            }

            std::cout << "Inserted " << i << " records" << std::endl;
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

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("password", _password);
        root->set("type", _type);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.password() = object->getValue<std::string>("password");
        user.type() = object->getValue<std::string>("type");

        return user;
    }

    User User::read_by_id(long id)
    {
        try
        {
            User result;
            // get all hints for shards
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<User>> futures;

            // map phase in parallel
            for (const std::string &hint : hints)
            {
                auto handle = std::async(std::launch::async, [id, hint]() -> User
                                        {
                                            Poco::Data::Session session = database::Database::get().create_session();
                                            Statement select(session);
                                            User a;
                                            std::string select_str = "SELECT id, first_name, last_name, email, password, type FROM User WHERE id='";
                                            select_str += id;
                                            select_str += "';";
                                            select_str += hint;
                                            select << select_str;
                                
                                            select.execute();
                                            Poco::Data::RecordSet rs(select);
                                            if (!rs.moveFirst()) throw std::logic_error("not found");

                                            a.id() = rs[0].convert<long>();
                                            a.first_name() = rs[1].convert<std::string>();
                                            a.last_name() = rs[2].convert<std::string>();
                                            a.email() = rs[3].convert<std::string>();
                                            a.type() = rs[4].convert<std::string>();

                                            return a;
                                        });

                futures.emplace_back(std::move(handle));
            }

            for (std::future<User> &res : futures)
            {
                bool exception_caught = false;
                try
                {
                    result = res.get();
                }
                catch (std::exception& e)
                {
                    exception_caught = true;
                }

                if (!exception_caught)
                {
                    return result;
                }
            }

            throw std::logic_error("not found");
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

    User User::read_by_email(std::string email)
    {
        try
        {
            User result;
            // get all hints for shards
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<User>> futures;

            // map phase in parallel
            for (const std::string &hint : hints)
            {
                auto handle = std::async(std::launch::async, [email, hint]() -> User
                                        {
                                            Poco::Data::Session session = database::Database::get().create_session();
                                            Statement select(session);
                                            User a;
                                            std::string select_str = "SELECT id, first_name, last_name, email, type FROM User WHERE email='";
                                            select_str += email;
                                            select_str += "';";
                                            select_str += hint;
                                            select << select_str;
                                
                                            select.execute();
                                            Poco::Data::RecordSet rs(select);
                                            if (!rs.moveFirst()) throw std::logic_error("not found");
                                            a.id() = rs[0].convert<long>();
                                            a.first_name() = rs[1].convert<std::string>();
                                            a.last_name() = rs[2].convert<std::string>();
                                            a.email() = rs[3].convert<std::string>();
                                            a.type() = rs[4].convert<std::string>();

                                            return a;
                                        });

                futures.emplace_back(std::move(handle));
            }

            for (std::future<User> &res : futures)
            {
                bool exception_caught = false;
                try
                {
                    result = res.get();
                }
                catch (std::exception& e)
                {
                    exception_caught = true;
                }

                if (!exception_caught)
                {
                    return result;
                }
            }

            throw std::logic_error("not found");
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            std::cout << e.displayText() << std::endl;
            std::cout << e.message() << std::endl;
            throw;
        }
    }

    std::vector<User> User::read_all()
    {
        try
        {
            // get all hints for shards
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<std::vector<User>>> futures;
            std::vector<User> result;

            // map phase in parallel
            for (const std::string &hint : hints)
            {
                auto handle = std::async(std::launch::async, [hint]() -> std::vector<User>
                                        {
                                            Poco::Data::Session session = database::Database::get().create_session();
                                            Statement select(session);
                                            std::vector<User> result;
                                            User a;
                                            select << "SELECT id, first_name, last_name, email, password, type FROM User;" << hint,
                                                into(a._id),
                                                into(a._first_name),
                                                into(a._last_name),
                                                into(a._email),
                                                into(a._password),
                                                into(a._type),
                                                range(0, 1); //  iterate over result set one row at a time
                                
                                            while (!select.done())
                                            {
                                                if(select.execute())
                                                result.push_back(a);
                                            }
                                            return result;
                                        });

                futures.emplace_back(std::move(handle));
            }

            for (std::future<std::vector<User>> &res : futures)
            {
                std::vector<User> a = res.get();
                std::copy(std::begin(a),
                std::end(a),
                std::back_inserter(result));
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

    std::vector<User> User::search(std::string first_name_mask, std::string last_name_mask)
    {
        try
        {
            std::vector<User> result;
            // get all hints for shards
            std::vector<std::string> hints = database::Database::get_all_hints();

            std::vector<std::future<std::vector<User>>> futures;

            // map phase in parallel
            for (const std::string &hint : hints)
            {
                auto handle = std::async(std::launch::async, [first_name_mask, last_name_mask, hint]() -> std::vector<User>
                                        {
                                            Poco::Data::Session session = database::Database::get().create_session();
                                            Statement select(session);
                                            std::vector<User> result;

                                            std::string select_str = "SELECT id, first_name, last_name, email, type FROM User where first_name LIKE '";
                                            select_str += first_name_mask;
                                            select_str += "' AND last_name LIKE '";
                                            select_str += last_name_mask;
                                            select_str += "';";
                                            select_str += hint;
                                            select << select_str;

                                            select.execute();
                                            Poco::Data::RecordSet rs(select);
                                            
                                            bool more = rs.moveFirst();
                                            while (more)
                                            {
                                                User a;
                                                a.id() = rs[0].convert<long>();
                                                a.first_name() = rs[1].convert<std::string>();
                                                a.last_name() = rs[2].convert<std::string>();
                                                a.email() = rs[3].convert<std::string>();
                                                a.type() = rs[4].convert<std::string>();
                                                result.push_back(a);
                                                more = rs.moveNext();
                                            }
                                            return result;
                                        });

                futures.emplace_back(std::move(handle));
            }

            // reduce phase
            // get values
            for (std::future<std::vector<User>> &res : futures)
            {
                std::vector<User> v = res.get();
                std::copy(std::begin(v),
                        std::end(v),
                        std::back_inserter(result));
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

   
    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            std::string hint = database::Database::sharding_hint(_email);

            insert << "INSERT INTO User (first_name,last_name,email,password,type) VALUES(?, ?, ?, ?, ?)" << hint,
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_password),
                use(_type);

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

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }


    const std::string &User::get_type() const
    {
        return _type;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::password()
    {
        return _password;
    }

    std::string &User::type()
    {
        return _type;
    }
}