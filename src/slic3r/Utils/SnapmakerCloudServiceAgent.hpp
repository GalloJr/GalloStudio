#pragma once

#include "GalloBaseCloudServiceAgent.hpp"
#include "CloudProvider.hpp"
#include "GalloVault.hpp"
#include <mutex>
#include <string>
#include <vector>

namespace Slic3r {

class SnapmakerCloudServiceAgent : public GalloBaseCloudServiceAgent {
public:
    SnapmakerCloudServiceAgent();
    virtual ~SnapmakerCloudServiceAgent();

    // Identificação do provedor
    std::string get_id() const override { return SNAPMAKER_CLOUD_PROVIDER; }

    // Lifecycle
    int set_config_dir(std::string config_dir) override;
    int set_country_code(std::string country_code) override;
    int start() override;

    // User Session
    int change_user(std::string user_info) override;
    bool is_user_login() override;
    int user_logout(bool request = false) override;
    std::string get_user_id() override;
    std::string get_user_name() override;
    std::string get_user_avatar() override;
    std::string get_user_nickname() override;

    // Snapmaker U1 Cloud Operations
    bool connect_snapmaker_u1(const std::string& dev_id);
    bool send_gcode_to_u1(const std::string& dev_id, const std::string& gcode_file_path, const std::string& task_name);
    std::string get_u1_status_json(const std::string& dev_id);

private:
    mutable std::mutex m_mutex;
    std::string m_config_dir;
    std::string m_country_code;
    bool m_connected { false };
};

} // namespace Slic3r
