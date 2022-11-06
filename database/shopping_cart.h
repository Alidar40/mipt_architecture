#ifndef SHOPPING_CART_H
#define SHOPPING_CART_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class ShoppingCart{
        private:
            long _id;
            long _user_id;
            std::string _created;

        public:

            static ShoppingCart fromJSON(const std::string & str);

            long               get_id() const;
            long               get_user_id() const;
            const std::string &get_created() const;

            long&        id();
            long&        user_id();
            std::string &created();

            static void init();
            static ShoppingCart read_by_id(long id);
            static ShoppingCart read_by_user_id(long user_id);
            static std::vector<ShoppingCart> read_all();
            static std::vector<ShoppingCart> read_all_by_user_id(long user_id);
            void add_to_cart(long shopping_item_id);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif