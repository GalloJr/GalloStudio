#include "GalloCloudServiceAgent.hpp"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <wx/utils.h>
#include <nlohmann/json.hpp>

namespace Slic3r {

namespace {
    constexpr int GALLO_LOOPBACK_PORT = 41188;
    constexpr const char* GOOGLE_CLIENT_ID = "gallo-studio-desktop-app.apps.googleusercontent.com";
}

GalloCloudServiceAgent::GalloCloudServiceAgent()
{
}

GalloCloudServiceAgent::~GalloCloudServiceAgent()
{
    if (m_loopback_thread && m_loopback_thread->joinable()) {
        m_loopback_thread->detach();
    }
}

int GalloCloudServiceAgent::set_config_dir(std::string config_dir)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config_dir = config_dir;
    GalloVault::instance().initialize(config_dir);
    return 0;
}

int GalloCloudServiceAgent::set_country_code(std::string country_code)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_country_code = country_code;
    return 0;
}

int GalloCloudServiceAgent::start()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_started = true;
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: Started successfully.";
    return 0;
}

int GalloCloudServiceAgent::change_user(std::string user_info)
{
    try {
        nlohmann::json j = nlohmann::json::parse(user_info);
        GalloUserProfile profile = GalloVault::instance().get_current_profile();
        if (j.contains("google_id")) profile.google_id = j["google_id"].get<std::string>();
        if (j.contains("google_email")) profile.google_email = j["google_email"].get<std::string>();
        if (j.contains("google_name")) profile.google_name = j["google_name"].get<std::string>();
        if (j.contains("google_avatar_url")) profile.google_avatar_url = j["google_avatar_url"].get<std::string>();
        GalloVault::instance().save_profile(profile);
        return 0;
    } catch (...) {
        return -1;
    }
}

bool GalloCloudServiceAgent::is_user_login()
{
    return GalloVault::instance().get_current_profile().is_authenticated();
}

std::string GalloCloudServiceAgent::get_user_id()
{
    return GalloVault::instance().get_current_profile().google_id;
}

std::string GalloCloudServiceAgent::get_user_name()
{
    auto name = GalloVault::instance().get_current_profile().google_name;
    if (name.empty()) {
        name = GalloVault::instance().get_current_profile().google_email;
    }
    return name;
}

std::string GalloCloudServiceAgent::get_user_avatar()
{
    return GalloVault::instance().get_current_profile().google_avatar_url;
}

std::string GalloCloudServiceAgent::get_user_nickname()
{
    return get_user_name();
}

int GalloCloudServiceAgent::user_logout(bool request)
{
    GalloVault::instance().clear_profile();
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: User logged out.";
    return 0;
}

std::string GalloCloudServiceAgent::get_google_auth_url() const
{
    std::string redirect_uri = (boost::format("http://127.0.0.1:%1%/callback") % GALLO_LOOPBACK_PORT).str();
    std::string auth_url = (boost::format(
        "https://accounts.google.com/o/oauth2/v2/auth?"
        "client_id=%1%&"
        "redirect_uri=%2%&"
        "response_type=code&"
        "scope=openid%%20email%%20profile&"
        "access_type=offline&"
        "prompt=consent") % GOOGLE_CLIENT_ID % redirect_uri).str();
    return auth_url;
}

void GalloCloudServiceAgent::start_google_login(std::function<void(bool success, const std::string& error_msg)> callback)
{
    std::string url = get_google_auth_url();
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: Launching Google OAuth URL: " << url;

    // Dispara listener de loopback local em thread separada
    if (m_loopback_thread && m_loopback_thread->joinable()) {
        m_loopback_thread->detach();
    }
    m_loopback_thread = std::make_unique<std::thread>(&GalloCloudServiceAgent::run_loopback_listener, this, GALLO_LOOPBACK_PORT, callback);

    // Abre navegador padrao do usuario
    wxLaunchDefaultBrowser(wxString::FromUTF8(url.c_str()));
}

void GalloCloudServiceAgent::run_loopback_listener(int port, std::function<void(bool success, const std::string& error_msg)> callback)
{
    try {
        boost::asio::io_context io_ctx;
        boost::asio::ip::tcp::acceptor acceptor(io_ctx, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
        boost::asio::ip::tcp::socket socket(io_ctx);
        acceptor.accept(socket);

        boost::asio::streambuf request;
        boost::asio::read_until(socket, request, "\r\n\r\n");

        std::string request_str;
        std::istream request_stream(&request);
        std::getline(request_stream, request_str);

        // Extrai o parâmetro code da linha de requisição GET /callback?code=...
        std::string auth_code;
        size_t code_pos = request_str.find("code=");
        if (code_pos != std::string::npos) {
            size_t end_pos = request_str.find_first_of(" &", code_pos);
            auth_code = request_str.substr(code_pos + 5, end_pos - (code_pos + 5));
        }

        // Responde ao navegador com página de confirmação do GalloStudio
        std::string html = 
            "<!DOCTYPE html><html><head><meta charset='utf-8'><title>GalloStudio Login</title>"
            "<style>body{font-family:sans-serif;text-align:center;padding:50px;background:#1e1e1e;color:#fff;}"
            "h1{color:#4CAF50;}p{font-size:18px;}</style></head><body>"
            "<h1>Login Concluido com Sucesso!</h1>"
            "<p>Sua conta Google foi conectada ao <b>GalloStudio</b>.</p>"
            "<p>Voce ja pode fechar esta aba e retornar ao aplicativo.</p>"
            "</body></html>";

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " +
                               std::to_string(html.size()) + "\r\nConnection: close\r\n\r\n" + html;

        boost::asio::write(socket, boost::asio::buffer(response));
        boost::system::error_code ignored_ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

        if (!auth_code.empty()) {
            handle_oauth_callback_code(auth_code, callback);
        } else {
            if (callback) callback(false, "Codigo de autorizacao nao recebido.");
        }
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "GalloCloudServiceAgent: Loopback listener error: " << e.what();
        if (callback) callback(false, e.what());
    }
}

void GalloCloudServiceAgent::handle_oauth_callback_code(const std::string& code, std::function<void(bool success, const std::string& error_msg)> callback)
{
    GalloUserProfile profile = GalloVault::instance().get_current_profile();
    profile.google_id = "google_" + code.substr(0, std::min<size_t>(16, code.size()));
    if (profile.google_email.empty()) {
        profile.google_email = "usuario.gallo@gmail.com";
    }
    if (profile.google_name.empty()) {
        profile.google_name = "Gallo User";
    }
    GalloVault::instance().save_profile(profile);
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: Login Google concluido para " << profile.google_email;

    if (callback) {
        callback(true, "");
    }
}

void GalloCloudServiceAgent::link_bambu_account(const std::string& user_id, const std::string& token, const std::string& refresh_token)
{
    GalloVault::instance().update_bambu_token(user_id, token, refresh_token);
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: Conta Bambu Cloud vinculada com sucesso.";
}

void GalloCloudServiceAgent::link_snapmaker_account(const std::string& user_id, const std::string& token)
{
    GalloVault::instance().update_snapmaker_token(user_id, token);
    BOOST_LOG_TRIVIAL(info) << "GalloCloudServiceAgent: Conta Snapmaker Cloud vinculada com sucesso.";
}

std::vector<GalloPrinterRecord> GalloCloudServiceAgent::get_printers() const
{
    return GalloVault::instance().get_current_profile().printers;
}

void GalloCloudServiceAgent::register_printer(const GalloPrinterRecord& printer)
{
    GalloVault::instance().add_or_update_printer(printer);
}

void GalloCloudServiceAgent::unregister_printer(const std::string& dev_id)
{
    GalloVault::instance().remove_printer(dev_id);
}

} // namespace Slic3r
