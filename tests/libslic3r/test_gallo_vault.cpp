#include <catch2/catch.hpp>
#include "slic3r/Utils/GalloVault.hpp"
#include <boost/filesystem.hpp>

using namespace Slic3r;

TEST_CASE("GalloVault stores and retrieves Google identity with cloud tokens", "[GalloVault]") {
    boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("gallo_test_%%%%-%%%%");
    boost::filesystem::create_directories(temp_dir);

    GalloVault& vault = GalloVault::instance();
    REQUIRE(vault.initialize(temp_dir.string()));

    SECTION("New vault is unauthenticated by default") {
        vault.clear_profile();
        GalloUserProfile loaded;
        REQUIRE_FALSE(vault.load_profile(loaded));
        REQUIRE_FALSE(loaded.is_authenticated());
    }

    SECTION("Vault successfully saves and recovers Google User and Cloud tokens") {
        GalloUserProfile profile;
        profile.google_id = "google_user_987654";
        profile.google_email = "renato.gallo@example.com";
        profile.google_name = "Renato Gallo";
        profile.google_avatar_url = "https://lh3.googleusercontent.com/a/avatar123";

        // Tokens para as nuvens oficiais
        profile.bambu_user_id = "bambu_usr_112233";
        profile.bambu_token = "jwt_bambu_access_token_secret_xyz";
        profile.bambu_refresh_token = "jwt_bambu_refresh_token_secret_abc";

        profile.snapmaker_user_id = "snap_usr_445566";
        profile.snapmaker_token = "snapmaker_api_cloud_token_def";

        // Cadastra as impressoras do usuário: Bambu Lab A1 e Snapmaker U1
        GalloPrinterRecord bambu_a1;
        bambu_a1.brand = "bambu";
        bambu_a1.model = "Bambu Lab A1";
        bambu_a1.dev_id = "01P00A123456789";
        bambu_a1.name = "Minha Bambu A1";
        bambu_a1.is_cloud_connected = true;
        profile.printers.push_back(bambu_a1);

        GalloPrinterRecord snap_u1;
        snap_u1.brand = "snapmaker";
        snap_u1.model = "Snapmaker U1";
        snap_u1.dev_id = "SM-U1-987654321";
        snap_u1.name = "Minha Snapmaker U1";
        snap_u1.is_cloud_connected = true;
        profile.printers.push_back(snap_u1);

        REQUIRE(vault.save_profile(profile));

        // Carrega do disco e valida decifragem dos dados
        GalloUserProfile recovered;
        REQUIRE(vault.load_profile(recovered));
        REQUIRE(recovered.is_authenticated());
        CHECK(recovered.google_id == "google_user_987654");
        CHECK(recovered.google_email == "renato.gallo@example.com");
        CHECK(recovered.google_name == "Renato Gallo");
        CHECK(recovered.bambu_token == "jwt_bambu_access_token_secret_xyz");
        CHECK(recovered.snapmaker_token == "snapmaker_api_cloud_token_def");
        REQUIRE(recovered.printers.size() == 2);
        CHECK(recovered.printers[0].model == "Bambu Lab A1");
        CHECK(recovered.printers[1].model == "Snapmaker U1");
    }

    SECTION("Vault allows updating printer list dynamically") {
        GalloPrinterRecord extra;
        extra.brand = "bambu";
        extra.model = "Bambu Lab A1 mini";
        extra.dev_id = "01P00M998877665";
        extra.name = "A1 mini Backup";
        vault.add_or_update_printer(extra);

        const auto& current = vault.get_current_profile();
        bool found = false;
        for (const auto& p : current.printers) {
            if (p.dev_id == "01P00M998877665") {
                found = true;
                break;
            }
        }
        CHECK(found);

        vault.remove_printer("01P00M998877665");
        bool still_found = false;
        for (const auto& p : vault.get_current_profile().printers) {
            if (p.dev_id == "01P00M998877665") {
                still_found = true;
                break;
            }
        }
        CHECK_FALSE(still_found);
    }

    // Cleanup
    boost::system::error_code ec;
    boost::filesystem::remove_all(temp_dir, ec);
}
