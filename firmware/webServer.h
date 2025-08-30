// Copyright (c) 2025 Mark Godwin.
// SPDX-License-Identifier: MIT

#pragma once

#include "dhcpserver/dhcpserver.h"
#include "dnsserver/dnsserver.h"
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <string.h>
#include "bufferOutput.h"

class DeviceConfig;
class ServiceControl;
class IWifiConnection;
struct CgiContext;
struct fs_file;

typedef std::map<std::string, std::string> CgiParams;
template<typename T>
using CgiSubscribeFunc = std::function<T(const CgiParams &)>;
using CgiSubscribeResultFunc = std::function<uint32_t(const CgiParams &, char *buffer, uint32_t length)>;

typedef std::function<uint16_t(char *buffer, int len, uint16_t tagPart, uint16_t *nextPart)> SsiSubscribeFunc;


/// @brief Rather bizarre and very stunted web-server
/// @remarks This isn't how a sane person would handle web requests in a microcontroller
class WebServer
{
    public:
        WebServer(std::shared_ptr<DeviceConfig> config, std::shared_ptr<IWifiConnection> WifiConnection);

        void Start();

        uint16_t HandleRequest(fs_file *file, const char* uri, int iNumParams, char **pcParam, char **pcValue, CgiContext *ctx);

        void AddRequestHandler(std::string url, CgiSubscribeResultFunc &&callback);
        void RemoveRequestHandler(std::string url);

        void AddResponseHandler(std::string tag, SsiSubscribeFunc &&callback);
        void RemoveResponseHandler(std::string tag);

    private:

        static uint16_t HandleResponseEntry(const char *tag, char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart, void *connectionState);
        uint16_t HandleResponse(const char *tag, char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart, const CgiContext *cgiContext);

        std::shared_ptr<DeviceConfig> _config;
        std::shared_ptr<IWifiConnection> _wifiConnection;

        std::map<std::string, CgiSubscribeResultFunc> _requestSubscriptions;
        std::map<std::string, SsiSubscribeFunc> _responseSubscriptions;

};

/// @brief Subscription to a CGI callback from the webserver. Used to process incoming request parameters
class CgiSubscription
{
    public:
        CgiSubscription(std::shared_ptr<WebServer> webInterface, std::string url, CgiSubscribeResultFunc &&callback)
        :   _webInterface(std::move(webInterface)),
             _url(std::move(url))
        {
            _webInterface->AddRequestHandler(_url, std::move(callback));
        }

        CgiSubscription(CgiSubscription &&other)
        :   _webInterface(std::move(other._webInterface)),
            _url(std::move(other._url))
        {
        }

        ~CgiSubscription()
        {
            if(_webInterface)
            {
                DBG_PRINT("Removing request handler for %s\n", _url.c_str());
                _webInterface->RemoveRequestHandler(_url);
            }
        }
    private:
        CgiSubscription(const CgiSubscription &) = delete;

        std::shared_ptr<WebServer> _webInterface;
        std::string _url;
};

template<typename T>
static CgiSubscription MakeCgiSubscription(std::shared_ptr<WebServer> webInterface, std::string url, CgiSubscribeFunc<T> &&callback)
{
    return CgiSubscription(webInterface, url, [cb = std::move(callback)](const CgiParams &params, char *buffer, uint32_t length)
{
    BufferOutput output(buffer, length);
    output.Append(cb(params));
    return output.BytesWritten();
    });
}

/// @brief Subscription to a Server-side include callback from the webserver. Used to replace fixed tags in the output data.
class SsiSubscription
{
    public:
        SsiSubscription(std::shared_ptr<WebServer> webInterface, std::string tag, SsiSubscribeFunc &&callback)
        :   _webInterface(webInterface),
             _tag(std::move(tag))
        {
            _webInterface->AddResponseHandler(_tag, std::forward<SsiSubscribeFunc>(callback));
        }

        SsiSubscription(SsiSubscription &&other)
        :   _webInterface(std::move(other._webInterface)),
            _tag(std::move(other._tag))
        {
        }


        ~SsiSubscription()
        {
            if(_webInterface)
                _webInterface->RemoveResponseHandler(_tag);
        }

    private:
        SsiSubscription(const SsiSubscription &) = delete;
        std::shared_ptr<WebServer> _webInterface;
        std::string _tag;


};

