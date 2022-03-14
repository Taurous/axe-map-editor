#include "web.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <filesystem>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

constexpr long HTTP_OK = 200;

std::string getApiURL(const std::string& query)
{
    return std::string("https://www.dnd5eapi.co") + query;
}

void downloadResource(const std::string& resource)
{
    try
    {
        std::ostringstream os; // Write data from curlpp into ostringstream
        curlpp::Cleanup cl; // Will init and terminate curlpp library RAII style
        curlpp::Easy req;

        std::string url = getApiURL(resource);

        // Download data from URL
        req.setOpt<curlpp::options::WriteStream>(&os);
        req.setOpt<curlpp::options::Url>(url);
        req.perform();

        // Make sure http request had no errors
        auto response = curlpp::infos::ResponseCode::get(req);
        if (response != HTTP_OK)
        {
            std::cerr << "Failed to connect to URL: " << url << "\n\tCode: " << response << "\n";
            return;
        }

        // Create directory for api data if it does not exist
        auto resource_filename = std::filesystem::current_path().concat(resource).concat(".json");
        auto resource_dir = resource_filename;
        resource_dir.remove_filename();
        if (!std::filesystem::exists(resource_dir))
        {
            std::cout << "Path: " << resource_dir << " does not exist! Creating directories...\n";
            std::filesystem::create_directories(resource_dir);
        }

        // Parse downloaded json data
        json j = json::parse(os.str());

        // Save to file
        std::ofstream m_file(resource_filename);
        if (!m_file.is_open())
        {
            std::cerr << "Failed to create file for " << resource_filename << std::endl;
            return;
        }
        m_file << j.dump(4);
        m_file.close();

        std::cout << "Downloaded \"" << resource << ".json\"" << std::endl;
    }
    catch(curlpp::RuntimeError& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(curlpp::LogicError& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void downloadResources(const std::string& resource_list)
{
    try
    {
        std::ostringstream os; // Write data from curlpp into ostringstream
        curlpp::Cleanup cl; // Will init and terminate curlpp library RAII style
        curlpp::Easy req;

        std::string url = getApiURL(resource_list);

        // Download data from URL
        req.setOpt<curlpp::options::WriteStream>(&os);
        req.setOpt<curlpp::options::Url>(url);
        req.perform();

        // Make sure http request had no errors
        auto response = curlpp::infos::ResponseCode::get(req);
        if (response != HTTP_OK)
        {
            std::cerr << "Failed to connect to URL: " << url << "\n\tCode: " << response << "\n";
            return;
        }

        // Parse downloaded json data
        json j = json::parse(os.str());

        // Get count of how many resources to download
        if (!j.contains("count"))
        {
            std::cerr << "Resource \"" << resource_list << "\" is not resource_list" << std::endl;
            return;
        }

        // Create directory for api data if it does not exist
        auto resource_dir = std::filesystem::current_path().concat(resource_list);
        if (!std::filesystem::exists(resource_dir))
        {
            std::cout << "Path: " << resource_dir << " does not exist! Creating directories...\n";
            std::filesystem::create_directories(resource_dir);
        }

        int resource_count = j["count"];
        std::cout << "Found " << resource_count << " resources. Downloading...\n";
        json resource = j["results"];

        // Loop through each resource and download it
        for (int i = 0; i < resource_count; ++i)
        {
            std::string r_name = resource[i]["index"];
            std::string r_url = resource[i]["url"];

            auto r_path = resource_dir;
            r_path.append(r_name + ".json");

            //Save to file
            std::ofstream m_file(r_path);
            if (!m_file.is_open())
            {
                std::cerr << "Failed to create file for " << r_path << std::endl;
                break;
            }

            os.str("");
            os.clear();
            
            req.setOpt<curlpp::options::WriteStream>(&os);
            req.setOpt<curlpp::options::Url>(getApiURL(r_url));
            req.perform();

            json temp = json::parse(os.str());

            m_file << temp.dump(4);
            m_file.close();

            // Display progress
            double perc = (double(i + 1) / double(resource_count)) * 100.0;
            int flat_perc = (int)perc;

            std::cout << "\rProgress - " << flat_perc << "%" << std::flush;

            if (i == resource_count - 1) std::cout << "\nDownloaded " << i + 1 << " resources!" << std::endl;
        }
    }
    catch(curlpp::RuntimeError& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(curlpp::LogicError& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void printWebRequest(const std::string& url)
{
    try
    {
        curlpp::Cleanup cl;

        curlpp::Easy req;

        req.setOpt<curlpp::options::Url>(url);

        req.perform();
    }
    catch(curlpp::RuntimeError& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(curlpp::LogicError& e)
    {
        std::cerr << e.what() << '\n';
    }
}