#pragma once

#include "ICloudServiceAgent.hpp"

namespace Slic3r {

/**
 * GalloBaseCloudServiceAgent - Fornece implementações padrão no-op para
 * todos os métodos de ICloudServiceAgent, permitindo que os agentes
 * especializados sobrescrevam apenas o que for necessário.
 */
class GalloBaseCloudServiceAgent : public ICloudServiceAgent {
public:
    virtual ~GalloBaseCloudServiceAgent() = default;

    // Lifecycle
    int init_log() override { return 0; }
    int set_config_dir(std::string config_dir) override { return 0; }
    int set_cert_file(std::string folder, std::string filename) override { return 0; }
    int set_country_code(std::string country_code) override { return 0; }
    int start() override { return 0; }

    // User Session
    int change_user(std::string user_info) override { return 0; }
    bool is_user_login() override { return false; }
    int user_logout(bool request = false) override { return 0; }
    std::string get_user_id() override { return ""; }
    std::string get_user_name() override { return ""; }
    std::string get_user_avatar() override { return ""; }
    std::string get_user_nickname() override { return get_user_name(); }

    // Login UI Support
    std::string build_login_cmd() override { return ""; }
    std::string build_logout_cmd() override { return ""; }
    std::string build_login_info() override { return ""; }

    // Token Access
    std::string get_access_token() const override { return ""; }
    std::string get_refresh_token() const override { return ""; }
    bool ensure_token_fresh(const std::string& reason) override { return true; }

    // Server Connectivity
    std::string get_cloud_service_host() override { return ""; }
    std::string get_cloud_login_url(const std::string& language = "") override { return ""; }
    int connect_server() override { return 0; }
    bool is_server_connected() override { return false; }
    int refresh_connection() override { return 0; }
    int start_subscribe(std::string module) override { return 0; }
    int stop_subscribe(std::string module) override { return 0; }
    int add_subscribe(std::vector<std::string> dev_list) override { return 0; }
    int del_subscribe(std::vector<std::string> dev_list) override { return 0; }
    void enable_multi_machine(bool enable) override {}

    // Settings Synchronization
    int get_user_presets(std::map<std::string, std::map<std::string, std::string>>* user_presets) override { return 0; }
    std::string request_setting_id(std::string name, std::map<std::string, std::string>* values_map, unsigned int* http_code) override { return ""; }
    int put_setting(std::string setting_id, std::string name, std::map<std::string, std::string>* values_map, unsigned int* http_code, bool force = false) override { return 0; }
    int get_setting_list(std::string bundle_version, ProgressFn pro_fn = nullptr, WasCancelledFn cancel_fn = nullptr) override { return 0; }
    int get_setting_list2(std::string bundle_version, CheckFn chk_fn, ProgressFn pro_fn = nullptr, WasCancelledFn cancel_fn = nullptr) override { return 0; }
    int delete_setting(std::string setting_id) override { return 0; }

    // Cloud User Services
    int get_my_message(int type, int after, int limit, unsigned int* http_code, std::string* http_body) override { return 0; }
    int check_user_task_report(int* task_id, bool* printable) override { return 0; }
    int get_user_print_info(unsigned int* http_code, std::string* http_body) override { return 0; }
    int get_user_tasks(TaskQueryParams params, std::string* http_body) override { return 0; }
    int get_printer_firmware(std::string dev_id, unsigned* http_code, std::string* http_body) override { return 0; }
    int get_task_plate_index(std::string task_id, int* plate_index) override { return 0; }
    int get_user_info(int* identifier) override { return 0; }
    int get_subtask_info(std::string subtask_id, std::string* task_json, unsigned int* http_code, std::string* http_body) override { return 0; }
    int get_slice_info(std::string project_id, std::string profile_id, int plate_index, std::string* slice_json) override { return 0; }
    int query_bind_status(std::vector<std::string> query_list, unsigned int* http_code, std::string* http_body) override { return 0; }
    int modify_printer_name(std::string dev_id, std::string dev_name) override { return 0; }

    // Model Mall & Publishing
    int get_camera_url(std::string dev_id, std::function<void(std::string)> callback) override { return 0; }
    int get_design_staffpick(int offset, int limit, std::function<void(std::string)> callback) override { return 0; }
    int start_publish(PublishParams params, OnUpdateStatusFn update_fn, WasCancelledFn cancel_fn, std::string* out) override { return 0; }
    int get_model_publish_url(std::string* url) override { return 0; }
    int get_subtask(BBLModelTask* task, OnGetSubTaskFn getsub_fn) override { return 0; }
    int get_model_mall_home_url(std::string* url) override { return 0; }
    int get_model_mall_detail_url(std::string* url, std::string id) override { return 0; }
    int get_my_profile(std::string token, unsigned int* http_code, std::string* http_body) override { return 0; }
    int get_my_token(std::string ticket, unsigned int* http_code, std::string* http_body) override { return 0; }

    // Analytics & Tracking
    int track_enable(bool enable) override { return 0; }
    int track_remove_files() override { return 0; }
    int track_event(std::string evt_key, std::string content) override { return 0; }
    int track_header(std::string header) override { return 0; }
    int track_update_property(std::string name, std::string value, std::string type = "string") override { return 0; }
    int track_get_property(std::string name, std::string& value, std::string type = "string") override { return 0; }
    bool get_track_enable() override { return false; }

    // Ratings & Reviews
    int put_model_mall_rating(int design_id, int score, std::string content, std::vector<std::string> images, unsigned int& http_code, std::string& http_error) override { return 0; }
    int get_oss_config(std::string& config, std::string country_code, unsigned int& http_code, std::string& http_error) override { return 0; }
    int put_rating_picture_oss(std::string& config, std::string& pic_oss_path, std::string model_id, int profile_id, unsigned int& http_code, std::string& http_error) override { return 0; }
    int get_model_mall_rating_result(int job_id, std::string& rating_result, unsigned int& http_code, std::string& http_error) override { return 0; }

    // Extra Features
    int get_mw_user_preference(std::function<void(std::string)> callback) override { return 0; }
    int get_mw_user_4ulist(int seed, int limit, std::function<void(std::string)> callback) override { return 0; }
    std::string get_version() override { return "1.0.0"; }

    // Callbacks
    int set_on_server_connected_fn(AppOnServerConnectedFn fn) override { return 0; }
    int set_on_http_error_fn(AppOnHttpErrorFn fn) override { return 0; }
    int set_get_country_code_fn(GetCountryCodeFn fn) override { return 0; }
    int set_queue_on_main_fn(QueueOnMainFn fn) override { return 0; }
};

} // namespace Slic3r
