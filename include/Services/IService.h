#pragma once

class IService
{
public:
    virtual ~IService() = default;
    virtual void Init() = 0;  // Like Initialize()
    virtual void Update() {}  // Optional update
    virtual void Clean() = 0; // Like Terminate()
};