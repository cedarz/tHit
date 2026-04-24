#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>

class AssetManager
{
public:
    static std::string GetAssetPath(const std::string& filename);
    static std::string GetWorkingDirectory();
};



#endif
