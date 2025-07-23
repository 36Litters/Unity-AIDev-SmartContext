#include "ClaudeAnalyzer.h"
#include <curl/curl.h>
#include <iostream>
#include <memory>

namespace UnityContextGen {
namespace AI {

// CURL response callback
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

class HTTPClient::Impl {
public:
    Impl() : curl(nullptr), timeout_seconds(30) {
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }
    
    ~Impl() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }
    
    HTTPResponse post(const std::string& url, const std::string& body,
                     const std::vector<std::pair<std::string, std::string>>& headers) {
        HTTPResponse response;
        response.success = false;
        
        if (!curl) {
            response.error_message = "CURL not initialized";
            return response;
        }
        
        // Reset CURL handle
        curl_easy_reset(curl);
        
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Set POST request
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        
        // Set headers
        struct curl_slist* header_list = nullptr;
        for (const auto& header : headers) {
            std::string header_str = header.first + ": " + header.second;
            header_list = curl_slist_append(header_list, header_str.c_str());
        }
        if (header_list) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        }
        
        // Set response callback
        std::string response_body;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        
        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);
        
        // Set user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
        
        // Disable SSL verification for development (should be enabled in production)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        // Perform request
        CURLcode res = curl_easy_perform(curl);
        
        // Clean up headers
        if (header_list) {
            curl_slist_free_all(header_list);
        }
        
        if (res != CURLE_OK) {
            response.error_message = "CURL error: " + std::string(curl_easy_strerror(res));
            return response;
        }
        
        // Get response code
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        response.status_code = static_cast<int>(response_code);
        response.body = response_body;
        response.success = (response_code >= 200 && response_code < 300);
        
        if (!response.success) {
            response.error_message = "HTTP error " + std::to_string(response_code);
        }
        
        return response;
    }
    
    void setTimeout(int seconds) {
        timeout_seconds = seconds;
    }
    
    void setUserAgent(const std::string& ua) {
        user_agent = ua;
    }
    
private:
    CURL* curl;
    int timeout_seconds;
    std::string user_agent = "UnityContextGenerator/1.0";
};

HTTPClient::HTTPClient() : m_impl(std::make_unique<Impl>()) {
}

HTTPClient::~HTTPClient() = default;

HTTPClient::HTTPResponse HTTPClient::post(const std::string& url, 
                                         const std::string& body,
                                         const std::vector<std::pair<std::string, std::string>>& headers) {
    return m_impl->post(url, body, headers);
}

void HTTPClient::setTimeout(int seconds) {
    m_impl->setTimeout(seconds);
}

void HTTPClient::setUserAgent(const std::string& user_agent) {
    m_impl->setUserAgent(user_agent);
}

} // namespace AI
} // namespace UnityContextGen