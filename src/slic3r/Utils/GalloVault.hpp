#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace Slic3r {

struct GalloPrinterRecord {
    std::string brand;        // "bambu" ou "snapmaker"
    std::string model;        // "Bambu Lab A1", "Snapmaker U1", etc.
    std::string dev_id;       // Serial number ou cloud device id
    std::string name;         // Nome customizado dado pelo usuario
    std::string ip_address;   // IP local opcional
    std::string access_code;  // Access code para LAN mode (Bambu) ou local API
    bool is_cloud_connected { true };
    std::string extra_data;   // JSON string com metadados adicionais
};

struct GalloUserProfile {
    std::string google_id;
    std::string google_email;
    std::string google_name;
    std::string google_avatar_url;
    std::string google_refresh_token;

    // Bambu Cloud Linked Credentials
    std::string bambu_user_id;
    std::string bambu_token;
    std::string bambu_refresh_token;

    // Snapmaker Cloud Linked Credentials
    std::string snapmaker_user_id;
    std::string snapmaker_token;

    // Impressoras da frota vinculadas a esta conta
    std::vector<GalloPrinterRecord> printers;

    bool is_authenticated() const {
        return !google_id.empty();
    }
};

class GalloVault {
public:
    static GalloVault& instance();

    bool initialize(const std::string& config_dir);
    bool save_profile(const GalloUserProfile& profile);
    bool load_profile(GalloUserProfile& out_profile);
    void clear_profile();

    bool update_bambu_token(const std::string& user_id, const std::string& token, const std::string& refresh_token);
    bool update_snapmaker_token(const std::string& user_id, const std::string& token);
    bool add_or_update_printer(const GalloPrinterRecord& printer);
    bool remove_printer(const std::string& dev_id);

    const GalloUserProfile& get_current_profile() const { return m_profile; }

private:
    GalloVault() = default;
    ~GalloVault() = default;
    GalloVault(const GalloVault&) = delete;
    GalloVault& operator=(const GalloVault&) = delete;

    std::string get_vault_file_path() const;
    std::vector<uint8_t> encrypt_data(const std::string& plain_text);
    std::string decrypt_data(const std::vector<uint8_t>& cipher_text);

    std::string m_config_dir;
    GalloUserProfile m_profile;
    bool m_initialized { false };
};

} // namespace Slic3r
