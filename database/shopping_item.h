#ifndef SHOPPING_ITEM_H
#define SHOPPING_ITEM_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class ShoppingItem{
        private:
            long _id;
            std::string _sku;
            std::string _name;
            double _price;

        public:

            static ShoppingItem fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_sku() const;
            const std::string &get_name() const;
            double get_price() const;

            long&        id();
            std::string &sku();
            std::string &name();
            double &price();

            static void init();
            static ShoppingItem read_by_id(long id);
            static ShoppingItem read_by_sku(std::string sku);
            static std::vector<ShoppingItem> read_all();
            static std::vector<ShoppingItem> search(std::string name);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif