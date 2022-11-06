#ifndef SHOPPING_ITEM_HANDLER_H
#define SHOPPING_ITEM_HANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/shopping_item.h"

class ShoppingItemHandler : public HTTPRequestHandler
{
private:
public:
    ShoppingItemHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        
        if (startsWith(request.getURI(), "/shopping-item/search"))
        {
            try
            {
                std::string name = form.get("name");
                auto results = database::ShoppingItem::search(name);
                Poco::JSON::Array arr;
                for (auto s : results)
                    arr.add(s.toJSON());
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);
            }
            catch (...)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                response.send();
                return;
            }
            return;
        }
        else if (startsWith(request.getURI(), "/shopping-item/all"))
        {
            try
            {
                auto results = database::ShoppingItem::read_all();
                Poco::JSON::Array arr;
                for (auto s : results)
                    arr.add(s.toJSON());
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);
            }
            catch (...)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                response.send();
                return;
            }
            return;
        }
        else if (startsWith(request.getURI(), "/shopping-item"))
        {
            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            {
                if (
                    form.has("sku") && \
                    form.has("name") && \
                    form.has("price")
                )
                {
                    database::ShoppingItem shoppingItem;
                    shoppingItem.sku() = form.get("sku");
                    shoppingItem.name() = form.get("name");
                    shoppingItem.price() = std::stod(form.get("price"));
                    try
                    {
                        shoppingItem.save_to_mysql();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << shoppingItem.get_id();
                        return;
                    }
                    catch (...)
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        std::ostream &ostr = response.send();
                        ostr << "database error";
                        response.send();
                        return;
                    }
                }
            }
            else
            {
                if (form.has("sku"))
                {
                    std::string sku = form.get("sku").c_str();
                    try
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        database::ShoppingItem result = database::ShoppingItem::read_by_sku(sku);
                        Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                        return;
                    }
                    catch (...)
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        std::ostream &ostr = response.send();
                        ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                        response.send();
                        return;
                    }
                }
            }
        }
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "request error";
        response.send();
    }

private:
    std::string _format;
};
#endif // !SHOPPING_ITEM_HANDLER_H