#include "web.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void downloadMonsterData(const std::string& url)
{
    try
    {
        std::ostringstream os;
        curlpp::Cleanup cl;
        curlpp::Easy req;

        req.setOpt<curlpp::options::WriteStream>(&os);
        req.setOpt<curlpp::options::Url>(url);
        req.perform();

        json j = json::parse(os.str());

        if (!j.contains("count"))
        {
            std::cerr << "Failed to download data from " << url << std::endl;
            return;
        }

        int monster_count = j["count"];

        std::cout << "Found " << monster_count << " monsters. Downloading...\n";

        json monster = j["results"];

        for (int i = 0; i < monster_count; ++i)
        {
            std::string name = monster[i]["index"];
            std::string m_url = monster[i]["url"];

            std::ofstream m_file(std::string("data/monsters/") + name);

            if (m_file.is_open())
            {
                os.str("");
                os.clear();
                
                req.setOpt<curlpp::options::WriteStream>(&os);
                req.setOpt<curlpp::options::Url>(std::string("https://www.dnd5eapi.co") + m_url);
                req.perform();

                json temp = json::parse(os.str());

                m_file << temp.dump(4);

                m_file.close();
            }
            else
            {
                std::cerr << "Failed to create file for " << name << "\nDownloaded " << i << " monsters" << std::endl;
                break;
            }

            if ((i % 50 == 0 && i > 0) || i == monster_count - 1) std::cout << "Downloaded " << i << " monsters...\n";
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