#include "asset_manager.h"
#include <ghc/filesystem.hpp>
#include <spdlog/spdlog.h>

std::string AssetManager::GetWorkingDirectory()
{
    return ASSETS_DIR;
}

std::string AssetManager::GetAssetPath(const std::string& filename)
{
    ghc::filesystem::path dir(ASSETS_DIR);
    dir /= filename;
    if (!ghc::filesystem::exists(dir)) {
        spdlog::error("File not exist: {}.", dir.generic_string());
    }
    return dir.generic_string();
}