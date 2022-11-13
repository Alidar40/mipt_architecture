#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class User{
        private:
            long _id;
            std::string _first_name;
            std::string _last_name;
            std::string _email;
            std::string _password;
            std::string _type; // Admin, shop worker or buyer

        public:

            static User fromJSON(const std::string & str);
            static void preload(const std::string & str);

            long             get_id() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;
            const std::string &get_password() const;
            const std::string &get_type() const;

            long&        id();
            std::string &first_name();
            std::string &last_name();
            std::string &email();
            std::string &password();
            std::string &type();

            static void init();
            static User read_by_id(long id);
            static User read_by_email(std::string email);
            static std::vector<User> read_all();
            static std::vector<User> search(std::string first_name,std::string last_name);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif