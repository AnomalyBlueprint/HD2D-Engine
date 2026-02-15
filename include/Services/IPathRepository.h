#pragma once
#include "Services/IService.h"
#include <string>

class IPathRepository : public IService
{
public:
    virtual ~IPathRepository() = default;
    virtual std::string GetPath(int assetID) = 0;
};
