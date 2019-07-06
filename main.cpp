#include "crow_all.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/filesystem.hpp>
#include <string>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include "nlohmann/json.hpp"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using mongocxx::cursor;

using namespace std;
using namespace crow;
using namespace crow::mustache;
using njson = nlohmann::json;
using bsoncxx::stdx::string_view;

void getView(response &res, const string &filename, context &x) {
  res.set_header("Content-Type", "text/html");
  auto text = load("../public/" + filename + ".html").render(x);
  res.write(text);
  res.end();
}

void sendFile(response &res, string filename, string contentType){
  ifstream in("../public/" + filename, ifstream::in);
  if(in){
    ostringstream contents;
    contents << in.rdbuf();
    in.close();
    res.set_header("Content-Type", contentType);
    res.write(contents.str());
  } else {
    res.code = 404;
    res.write("Not found");
  }
  res.end();
}

void sendHtml(response &res, string filename){
  sendFile(res, filename + ".html", "text/html");
}

void sendImage(response &res, string filename){
  sendFile(res, "images/" + filename, "image/jpeg");
}

void sendScript(response &res, string filename){
  sendFile(res, "scripts/" + filename, "text/javascript");
}

void sendStyle(response &res, string filename){
  sendFile(res, "styles/" + filename, "text/css");
}

void notFound(response &res, const string &message){
  res.code = 404;
  res.write(message + ": Not Found");
  res.end();
}

int main(int argc, char* argv[]) {
  crow::SimpleApp app;
  set_base(".");

  mongocxx::instance inst{};
  string mongoConnect = std::string("mongodb://root:Dhuma777@ds241647.mlab.com:41647/ngx-adibro");
  mongocxx::client conn{mongocxx::uri{mongoConnect}};
  mongocxx::database db = conn["ngx-adibro"];
  auto collection = db["blogs"];

  auto index_spec = document{} << "uid" << 1 << finalize;
  collection.create_index(std::move(index_spec));

 // auto collection = conn["ngx-adibro"]["contacts"];

//  CROW_ROUTE(app, "/styles/<string>")
//    ([](const request &req, response &res, string filename){
//      sendStyle(res, filename);
//    });

//  CROW_ROUTE(app, "/scripts/<string>")
//    ([](const request &req, response &res, string filename){
//      sendScript(res, filename);
//    });

//  CROW_ROUTE(app, "/images/<string>")
 //   ([](const request &req, response &res, string filename){
 //     sendImage(res, filename);
 //   });

 // CROW_ROUTE(app, "/about")
 //   ([](const request &req, response &res){
 //     sendHtml(res, "about");
 //   });

 // CROW_ROUTE(app, "/contact/<string>")
 //   ([&collection](const request &req, response &res, string email){
 //     auto doc = collection.find_one(make_document(kvp("email", email)));
 //     crow::json::wvalue dto;
 //     dto["contact"] = json::load(bsoncxx::to_json(doc.value().view()));
 //     getView(res, "contact", dto);
 //   });

 // CROW_ROUTE(app, "/contact/<string>/<string>")
 //   ([&collection](const request &req, response &res, string firstname, string lastname){
 //     auto doc = collection.find_one(
 //       make_document(kvp("firstName", firstname), kvp("lastName", lastname)));
 //     if(!doc){
 //       return notFound(res, "Contact");
 //     }
 //     crow::json::wvalue dto;
 //     dto["contact"] = json::load(bsoncxx::to_json(doc.value().view()));
 //     getView(res, "contact", dto);
 //   });

  CROW_ROUTE(app, "/api/posts")
    ([&collection](const request &req){
      mongocxx::options::find opts{};
     // opts.projection(document{} << "_id" << 0 << finalize);
      auto docs = collection.find({}, opts);
      crow::json::wvalue dto;
      vector<crow::json::rvalue> posts;

     for(auto doc : docs){   
     posts.push_back(json::load(bsoncxx::to_json(doc)));
      }
      dto["posts"] = posts;
      crow::response res;
      res.set_header("Content-Type","application/json");
      return crow::json::dump(dto);
    });

 CROW_ROUTE(app, "/api/admin/posts").methods("POST"_method)
	([&collection](const request &req) { 
	auto x = crow::json::load(req.body);
	if (!x || x["uname"] != "adibro" || x["password"] != "Dhuma777.") {
	return crow::response(400);
	}
	mongocxx::options::find opts{};
	//opts.projection(document{} << "_id" << 0 << finalize);
	auto docs = collection.find({}, opts);
	crow::json::wvalue dto;
	vector<crow::json::rvalue> posts;
	for(auto doc : docs) {
	posts.push_back(json::load(bsoncxx::to_json(doc)));
	}
	dto["posts"] = posts;
	crow:: response res;
	res.set_header("Content-Type", "application/json");
	return crow::response(dto);
   });

 CROW_ROUTE(app, "/api/posts/<string>")
	 ([&collection](const request &req, string a){
	  mongocxx::options::find opts{};
	  opts.projection(document{} << "_id" << 0 << finalize);
	  auto doc = collection.find_one(document{} << "_id" << bsoncxx::oid(a) << finalize, opts);
	  crow::json::rvalue res;
	  res = json::load(bsoncxx::to_json(doc->view()));
	  crow::json::wvalue resultingJson;
	  resultingJson["posts"] = res; 
	  return crow::response(resultingJson);
	  });


//  CROW_ROUTE(app, "/api/contacts")
 //   ([&collection](const request &req){
  //    auto skipVal = req.url_params.get("skip");
  //    auto limitVal = req.url_params.get("limit");
  //    int skip = skipVal? stoi(skipVal): 0;
  //    int limit = limitVal? stoi(limitVal): 10;

   //   mongocxx::options::find opts;
   //   opts.skip(skip);
   //   opts.limit(limit);
    //  auto docs = collection.find({}, opts);
  //    vector<crow::json::rvalue> contacts;
  //    contacts.reserve(10);

   //   for(auto doc : docs){
   //     contacts.push_back(json::load(bsoncxx::to_json(doc)));
   //   }
   //   crow::json::wvalue dto;
   //   dto["contacts"] = contacts;
   //   return crow::response{dto};
   // });

 // CROW_ROUTE(app, "/add/<int>/<int>")
 //   ([](const request &req, response &res, int a, int b){
 //       res.set_header("Content-Type","text/plain");
 //       ostringstream os;
 //       os << "Integer: " << a << " + " << b << " = " << a + b << "\n";
 //       res.write(os.str());
 //       res.end();
 //   });

 // CROW_ROUTE(app, "/add/<double>/<double>")
 //   ([](const request &req, response &res, double a, double b){
 //       res.set_header("Content-Type","text/plain");
 //       ostringstream os;
 //       os << "Double: " << a << " + " << b << " = " << a + b << "\n";
 //       res.write(os.str());
 //       res.end();
 //   });

 // CROW_ROUTE(app, "/add/<string>/<string>")
 //   ([](const request &req, response &res, string a, string b){
 //       res.set_header("Content-Type","text/plain");
 //       ostringstream os;
 //       os << "String: " << a << " + " << b << " = " << a + b << "\n";
 //       res.write(os.str());
 //       res.end();
 //   });

 // CROW_ROUTE(app, "/query")
 //   ([](const request &req, response &res){
 //     auto firstname = req.url_params.get("firstname");
 //     auto lastname = req.url_params.get("lastname");
 //     ostringstream os;
 //     os << "Hello "<< (firstname? firstname: "") <<
 //       " " << (lastname? lastname: "") << endl;
 //     res.set_header("Content-Type", "text/plain");
 //     res.write(os.str());
 //     res.end();
 //   });

 // CROW_ROUTE(app, "/rest_test").methods(HTTPMethod::Post, HTTPMethod::Get,
 // HTTPMethod::Put)
 //   ([](const request &req, response &res){
 //     string method = method_name(req.method);
 //     res.set_header("Content-Type", "text/plain");
 //     res.write(method + " rest_test");
 //     res.end();
 //   });
 //
    CROW_ROUTE(app, "/api/admin/save/post").methods("POST"_method)
    ([&collection](const crow::request& req) {
	njson x = njson::parse(req.body);
	if(x.empty())
		return crow::response(400);
	auto docs = collection.find({});
	mongocxx::options::find opts;

	//njson dx = bsoncxx::to_json(*docs.end());
	//int i = (collection.count({}) == 0) ? 1 : std::stoi(dx["uid"].get<std::string>()) + 1;
	cout << "middle";
	x.erase("uname");
	x.erase("password");
	int i = collection.count({});	
	cout << i << "iii";
	auto doc = collection.insert_one(bsoncxx::from_json(x.dump()));
	return crow::response(200);
	});

    CROW_ROUTE(app, "/api/admin/edit/post").methods("POST"_method)
    ([&collection](const crow::request& req) {
     auto x = njson::parse(req.body);
     //auto uid = x["uid"].get<std::string>();
     auto title = x["title"].get<std::string>();
     auto subtitle = x["subtitle"].get<std::string>();
     auto content = x["content"].get<std::string>();    

     if(x.empty()) {
     return crow::response(400);
     }
 collection.update_one(document{} << "_id" << bsoncxx::oid(x["id"].get<std::string>()) << finalize, document{} << "$set" << open_document << "title" << title << "subtitle" << subtitle << "content" << content << close_document << finalize);
     return crow::response(200);     
     });    

    CROW_ROUTE(app, "/api/admin/delete/post").methods("POST"_method)
	([&collection](const crow::request& req){
	 auto x = njson::parse(req.body);
	 if (x.empty()) {
	 return crow::response(400);
	 }
	 cout << "param" << x["id"].get<std::string>() << endl;
	collection.delete_one(document{} << "_id" << bsoncxx::oid(x["id"].get<std::string>()) << finalize);
	return crow::response(200);
	 });

  CROW_ROUTE(app, "/")
    ([](const request &req, response &res){
  	res.set_header("Content-Type", "text-plain");
	res.write("Hello World!!!");
	res.end();
    });

  char* port = getenv("PORT");
  uint16_t iPort = static_cast<uint16_t>(port != NULL? stoi(port): 18080);
  cout << "PORT = " << iPort << "\n";
  app.port(iPort).multithreaded().run();
}
