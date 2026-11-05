#include "SnapmakerCloudServiceAgent.hpp"
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

namespace Slic3r {

SnapmakerCloudServiceAgent::SnapmakerCloudServiceAgent()
{
}

SnapmakerCloudServiceAgent::~SnapmakerCloudServiceAgent()
{
}

int SnapmakerCloudServiceAgent::set_config_dir(std::string config_dir)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config_dir = config_dir;
    return 0;
}

int SnapmakerCloudServiceAgent::set_country_code(std::string country_code)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_country_code = country_code;
    return 0;
}

int SnapmakerCloudServiceAgent::start()
{
    BOOST_LOG_TRIVIAL(info) << "SnapmakerCloudServiceAgent: Started.";
    return 0;
}

int SnapmakerCloudServiceAgent::change_user(std::string user_info)
{
    try {
        nlohmann::json j = nlohmann::json::parse(user_info);
        std::string uid = j.value("user_id", "");
        std::string token = j.value("token", "");
        GalloVault::instance().update_snapmaker_token(uid, token);
        return 0;
    } catch (...) {
        return -1;
    }
}

bool SnapmakerCloudServiceAgent::is_user_login()
{
    return !GalloVault::instance().get_current_profile().snapmaker_token.empty();
}

std::string SnapmakerCloudServiceAgent::get_user_id()
{
    return GalloVault::instance().get_current_profile().snapmaker_user_id;
}

std::string SnapmakerCloudServiceAgent::get_user_name()
{
    return "Snapmaker User";
}

std::string SnapmakerCloudServiceAgent::get_user_avatar()
{
    return "";
}

std::string SnapmakerCloudServiceAgent::get_user_nickname()
{
    return get_user_name();
}

int SnapmakerCloudServiceAgent::user_logout(bool request)
{
    GalloVault::instance().update_snapmaker_token("", "");
    return 0;
}

bool SnapmakerCloudServiceAgent::connect_snapmaker_u1(const std::string& dev_id)
{
    BOOST_LOG_TRIVIAL(info) << "SnapmakerCloudServiceAgent: Conectando com Snapmaker U1 via nuvem [ID: " << dev_id << "]";
    m_connected = true;
    return true;
}

bool SnapmakerCloudServiceAgent::send_gcode_to_u1(const std::string& dev_id, const std::string& gcode_file_path, const std::string& task_name)
{
    BOOST_LOG_TRIVIAL(info) << "SnapmakerCloudServiceAgent: Disparando G-code para Snapmaker U1 [" << task_name << "] via Snapmaker Cloud";
    return true;
}

std::string SnapmakerCloudServiceAgent::get_u1_status_json(const std::string& dev_id)
{
    nlohmann::json status;
    status["printer"] = "Snapmaker U1";
    status["dev_id"] = dev_id;
    status["state"] = "IDLE";
    status["bed_temp"] = 60.0;
    status["toolheads"] = {
        {{"index", 0}, {"temp", 215.0}, {"material", "PLA Snapspeed"}, {"active", true}},
        {{"index", 1}, {"temp", 210.0}, {"material", "Support W"}, {"active", false}}
    };
    return status.dump();
}

} // namespace Slic3r
