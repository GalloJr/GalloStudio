#include "GalloVault.hpp"
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")
#endif

namespace Slic3r {

GalloVault& GalloVault::instance()
{
    static GalloVault instance;
    return instance;
}

bool GalloVault::initialize(const std::string& config_dir)
{
    m_config_dir = config_dir;
    m_initialized = true;
    return load_profile(m_profile);
}

std::string GalloVault::get_vault_file_path() const
{
    boost::filesystem::path p(m_config_dir);
    p /= "gallo_vault.dat";
    return p.string();
}

std::vector<uint8_t> GalloVault::encrypt_data(const std::string& plain_text)
{
    std::vector<uint8_t> result;
    if (plain_text.empty())
        return result;

#ifdef _WIN32
    DATA_BLOB data_in;
    DATA_BLOB data_out;
    data_in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plain_text.data()));
    data_in.cbData = static_cast<DWORD>(plain_text.size());

    if (CryptProtectData(&data_in, L"GalloStudioLocalVault", nullptr, nullptr, nullptr, 0, &data_out)) {
        result.assign(data_out.pbData, data_out.pbData + data_out.cbData);
        LocalFree(data_out.pbData);
    } else {
        BOOST_LOG_TRIVIAL(error) << "GalloVault: CryptProtectData failed with error " << GetLastError();
    }
#else
    // Fallback simples para não-Windows
    result.assign(plain_text.begin(), plain_text.end());
    for (auto& b : result) b ^= 0x5A;
#endif

    return result;
}

std::string GalloVault::decrypt_data(const std::vector<uint8_t>& cipher_text)
{
    if (cipher_text.empty())
        return "";

#ifdef _WIN32
    DATA_BLOB data_in;
    DATA_BLOB data_out;
    data_in.pbData = const_cast<BYTE*>(cipher_text.data());
    data_in.cbData = static_cast<DWORD>(cipher_text.size());

    if (CryptUnprotectData(&data_in, nullptr, nullptr, nullptr, nullptr, 0, &data_out)) {
        std::string plain(reinterpret_cast<char*>(data_out.pbData), data_out.cbData);
        LocalFree(data_out.pbData);
        return plain;
    } else {
        BOOST_LOG_TRIVIAL(error) << "GalloVault: CryptUnprotectData failed with error " << GetLastError();
        return "";
    }
#else
    std::string plain;
    plain.reserve(cipher_text.size());
    for (auto b : cipher_text) plain.push_back(static_cast<char>(b ^ 0x5A));
    return plain;
#endif
}

bool GalloVault::save_profile(const GalloUserProfile& profile)
{
    m_profile = profile;
    if (m_config_dir.empty())
        return false;

    nlohmann::json root;
    root["google_id"] = profile.google_id;
    root["google_email"] = profile.google_email;
    root["google_name"] = profile.google_name;
    root["google_avatar_url"] = profile.google_avatar_url;
    root["google_refresh_token"] = profile.google_refresh_token;

    root["bambu_user_id"] = profile.bambu_user_id;
    root["bambu_token"] = profile.bambu_token;
    root["bambu_refresh_token"] = profile.bambu_refresh_token;

    root["snapmaker_user_id"] = profile.snapmaker_user_id;
    root["snapmaker_token"] = profile.snapmaker_token;

    nlohmann::json printers_arr = nlohmann::json::array();
    for (const auto& prn : profile.printers) {
        nlohmann::json pj;
        pj["brand"] = prn.brand;
        pj["model"] = prn.model;
        pj["dev_id"] = prn.dev_id;
        pj["name"] = prn.name;
        pj["ip_address"] = prn.ip_address;
        pj["access_code"] = prn.access_code;
        pj["is_cloud_connected"] = prn.is_cloud_connected;
        pj["extra_data"] = prn.extra_data;
        printers_arr.push_back(pj);
    }
    root["printers"] = printers_arr;

    std::string serialized = root.dump();
    std::vector<uint8_t> encrypted = encrypt_data(serialized);
    if (encrypted.empty())
        return false;

    std::string path = get_vault_file_path();
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open()) {
        BOOST_LOG_TRIVIAL(error) << "GalloVault: Failed to open vault file for writing: " << path;
        return false;
    }

    ofs.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
    ofs.close();
    BOOST_LOG_TRIVIAL(info) << "GalloVault: Profile successfully saved to " << path;
    return true;
}

bool GalloVault::load_profile(GalloUserProfile& out_profile)
{
    std::string path = get_vault_file_path();
    if (!boost::filesystem::exists(path)) {
        return false;
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return false;

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    std::string plain = decrypt_data(buffer);
    if (plain.empty())
        return false;

    try {
        nlohmann::json root = nlohmann::json::parse(plain);
        out_profile.google_id = root.value("google_id", "");
        out_profile.google_email = root.value("google_email", "");
        out_profile.google_name = root.value("google_name", "");
        out_profile.google_avatar_url = root.value("google_avatar_url", "");
        out_profile.google_refresh_token = root.value("google_refresh_token", "");

        out_profile.bambu_user_id = root.value("bambu_user_id", "");
        out_profile.bambu_token = root.value("bambu_token", "");
        out_profile.bambu_refresh_token = root.value("bambu_refresh_token", "");

        out_profile.snapmaker_user_id = root.value("snapmaker_user_id", "");
        out_profile.snapmaker_token = root.value("snapmaker_token", "");

        out_profile.printers.clear();
        if (root.contains("printers") && root["printers"].is_array()) {
            for (const auto& pj : root["printers"]) {
                GalloPrinterRecord prn;
                prn.brand = pj.value("brand", "");
                prn.model = pj.value("model", "");
                prn.dev_id = pj.value("dev_id", "");
                prn.name = pj.value("name", "");
                prn.ip_address = pj.value("ip_address", "");
                prn.access_code = pj.value("access_code", "");
                prn.is_cloud_connected = pj.value("is_cloud_connected", true);
                prn.extra_data = pj.value("extra_data", "");
                out_profile.printers.push_back(prn);
            }
        }
        m_profile = out_profile;
        BOOST_LOG_TRIVIAL(info) << "GalloVault: Profile loaded for Google User: " << out_profile.google_email;
        return true;
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "GalloVault: JSON parse error while loading vault: " << e.what();
        return false;
    }
}

void GalloVault::clear_profile()
{
    m_profile = GalloUserProfile();
    std::string path = get_vault_file_path();
    if (boost::filesystem::exists(path)) {
        boost::system::error_code ec;
        boost::filesystem::remove(path, ec);
    }
}

bool GalloVault::update_bambu_token(const std::string& user_id, const std::string& token, const std::string& refresh_token)
{
    m_profile.bambu_user_id = user_id;
    m_profile.bambu_token = token;
    m_profile.bambu_refresh_token = refresh_token;
    return save_profile(m_profile);
}

bool GalloVault::update_snapmaker_token(const std::string& user_id, const std::string& token)
{
    m_profile.snapmaker_user_id = user_id;
    m_profile.snapmaker_token = token;
    return save_profile(m_profile);
}

bool GalloVault::add_or_update_printer(const GalloPrinterRecord& printer)
{
    for (auto& existing : m_profile.printers) {
        if (existing.dev_id == printer.dev_id) {
            existing = printer;
            return save_profile(m_profile);
        }
    }
    m_profile.printers.push_back(printer);
    return save_profile(m_profile);
}

bool GalloVault::remove_printer(const std::string& dev_id)
{
    auto it = std::remove_if(m_profile.printers.begin(), m_profile.printers.end(),
                             [&dev_id](const GalloPrinterRecord& r) { return r.dev_id == dev_id; });
    if (it != m_profile.printers.end()) {
        m_profile.printers.erase(it, m_profile.printers.end());
        return save_profile(m_profile);
    }
    return false;
}

} // namespace Slic3r
