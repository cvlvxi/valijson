#pragma once

#include <iostream>
#include <json11.hpp>
#include <json.hpp>
#include <picojson.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <optional>
#include <valijson/utils/file_utils.hpp>

using std::experimental::nullopt;
using std::experimental::optional;


namespace valijson {
    namespace utils {
        template <typename T> optional<T> loadDocumentExperimental(const std::string &path);


        template<>
        optional<json11::Json> loadDocumentExperimental(const std::string &path) {
            // Load schema JSON from file
            std::string file;
            if (!valijson::utils::loadFile(path, file)) {
                std::cerr << "Failed to load json from file '" << path << "'." << std::endl;
                return nullopt;
            }

            // Parse schema
            std::string err;
            json11::Json document = json11::Json::parse(file, err);
            if (!err.empty()) {
                std::cerr << "json11 failed to parse the document:" << std::endl
                          << "Parse error: " << err << std::endl;
                return nullopt;
            }

            return document;
        }

        template<>  optional<Json::Value> loadDocumentExperimental(const std::string &path)
        {
            // Load schema JSON from file
            std::string file;
            if (!loadFile(path, file)) {
                std::cerr << "Failed to load json from file '" << path << "'." << std::endl;
                return nullopt;
            }

            const auto fileLength = static_cast<int>(file.length());
            Json::CharReaderBuilder builder;
            const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            std::string err;
            Json::Value document;
            if (!reader->parse(file.c_str(), file.c_str() + fileLength, &document, &err)) {
                std::cerr << "Jsoncpp parser failed to parse the document:" << std::endl << err;
                return nullopt;
            }
            return document;
        }



        template<>
        optional<nlohmann::json> loadDocumentExperimental(const std::string &path)
        {
            // Load schema JSON from file
            std::string file;
            if (!loadFile(path, file)) {
                std::cerr << "Failed to load json from file '" << path << "'."
                          << std::endl;
                return nullopt;
            }

            // Parse schema
#if VALIJSON_USE_EXCEPTION
            try {
        document = nlohmann::json::parse(file);
    } catch (std::invalid_argument const& exception) {
        std::cerr << "nlohmann::json failed to parse the document\n"
            << "Parse error:" << exception.what() << "\n";
        return false;
    }
#else
            nlohmann::json document = nlohmann::json::parse(file, nullptr, false);
            if (document.is_discarded()) {
                std::cerr << "nlohmann::json failed to parse the document.";
                return nullopt;
            }
#endif

            return document;
        }

        template <>
        optional<picojson::value> loadDocumentExperimental(const std::string &path)
        {
            // Load schema JSON from file
            std::string file;
            if (!loadFile(path, file)) {
                std::cerr << "Failed to load json from file '" << path << "'." << std::endl;
                return nullopt;
            }

            // Parse schema
            picojson::value document;
            std::string err = picojson::parse(document, file);
            if (!err.empty()) {
                std::cerr << "PicoJson failed to parse the document:" << std::endl
                          << "Parse error: " << err << std::endl;
                return nullopt;
            }

            return document;
        }


        template <>
        optional<Poco::Dynamic::Var> loadDocumentExperimental(const std::string &path)
        {
            // Load schema JSON from file
            std::string file;
            if (!loadFile(path, file)) {
                std::cerr << "Failed to load json from file '" << path << "'."
                          << std::endl;
                return nullopt;
            }

            // Parse schema
            Poco::Dynamic::Var document;
            try {
                document = Poco::JSON::Parser().parse(file);
            } catch (Poco::Exception const& exception) {
                std::cerr << "Poco::JSON failed to parse the document\n"
                          << "Parse error:" << exception.what() << "\n";
                return nullopt;
            }

            return document;
        }

    }
}