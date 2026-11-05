#pragma once

#include "GalloBaseCloudServiceAgent.hpp"
#include "GalloVault.hpp"
#include "CloudProvider.hpp"
#include <mutex>
#include <thread>
#include <functional>
#include <boost/asio.hpp>

namespace Slic3r {

class GalloCloudServiceAgent : public GalloBaseCloudServiceAgent {
public:
    GalloCloudServiceAgent();
    virtual ~GalloCloudServiceAgent();

    // Identificação do provedor
    std::string get_id() const override { return GALLO_CLOUD_PROVIDER; }

    // Lifecycle
    int set_config_dir(std::string config_dir) override;
    int set_country_code(std::string country_code) override;
    int start() override;

    // Sessão do Usuário
    int change_user(std::string user_info) override;
    bool is_user_login() override;
    int user_logout(bool request = false) override;
    std::string get_user_id() override;
    std::string get_user_name() override;
    std::string get_user_avatar() override;
    std::string get_user_nickname() override;

    // Google Sign-In & Multi-Cloud Linking
    std::string get_google_auth_url() const;
    void start_google_login(std::function<void(bool success, const std::string& error_msg)> callback);
    void link_bambu_account(const std::string& user_id, const std::string& token, const std::string& refresh_token);
    void link_snapmaker_account(const std::string& user_id, const std::string& token);

    // Impressoras da frota (Bambu Lab A1 + Snapmaker U1)
    std::vector<GalloPrinterRecord> get_printers() const;
    void register_printer(const GalloPrinterRecord& printer);
    void unregister_printer(const std::string& dev_id);

private:
    void run_loopback_listener(int port, std::function<void(bool success, const std::string& error_msg)> callback);
    void handle_oauth_callback_code(const std::string& code, std::function<void(bool success, const std::string& error_msg)> callback);

    mutable std::mutex m_mutex;
    std::string m_config_dir;
    std::string m_country_code;
    bool m_started { false };
    std::unique_ptr<std::thread> m_loopback_thread;
};

} // namespace Slic3r
