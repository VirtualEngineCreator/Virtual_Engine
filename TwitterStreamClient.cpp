#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <ctime>
#include <iomanip>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

// Class to handle real-time data streaming from Twitter
class TwitterStreamClient {
public:
    TwitterStreamClient(const std::string& consumerKey, const std::string& consumerSecret, const std::string& accessToken, const std::string& accessTokenSecret)
        : consumerKey_(consumerKey), consumerSecret_(consumerSecret), accessToken_(accessToken), accessTokenSecret_(accessTokenSecret), terminate_(false) {}

    // Function to start tracking real-time data
    void startTracking(const std::string& keywords) {
        std::string url = "https://stream.twitter.com/1.1/statuses/filter.json?track=" + urlEncode(keywords);

        std::string authorizationHeader = generateAuthorizationHeader("POST", url, {});

        CURL* curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, authorizationHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L); // No timeout for streaming

            std::unique_lock<std::mutex> lock(mutex_);
            streamingThread_ = std::thread([curl]() { curl_easy_perform(curl); });
            cv_.wait(lock, [this] { return terminate_; });

            curl_easy_cleanup(curl);
        }
    }

    // Function to stop tracking
    void stopTracking() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            terminate_ = true;
        }
        cv_.notify_one();
        if (streamingThread_.joinable()) {
            streamingThread_.join();
        }
    }

private:
    std::string consumerKey_;
    std::string consumerSecret_;
    std::string accessToken_;
    std::string accessTokenSecret_;
    bool terminate_;
    std::thread streamingThread_;
    std::mutex mutex_;
    std::condition_variable cv_;

    std::string urlEncode(const std::string& value) {
        CURL* curl = curl_easy_init();
        char* encoded = curl_easy_escape(curl, value.c_str(), value.length());
        std::string result(encoded);
        curl_free(encoded);
        curl_easy_cleanup(curl);
        return result;
    }

    std::string generateBaseString(const std::string& method, const std::string& url, const std::map<std::string, std::string>& parameters) {
        std::stringstream ss;
        ss << method << "&" << urlEncode(url) << "&";

        std::vector<std::string> params;
        for (const auto& param : parameters) {
            params.push_back(urlEncode(param.first) + "=" + urlEncode(param.second));
        }
        std::sort(params.begin(), params.end());
        for (size_t i = 0; i < params.size(); ++i) {
            ss << params[i];
            if (i < params.size() - 1) {
                ss << "&";
            }
        }

        return ss.str();
    }

    std::string calculateHMACSHA1(const std::string& data, const std::string& key) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        HMAC_CTX* hmac = HMAC_CTX_new();
        HMAC_Init_ex(hmac, key.c_str(), key.length(), EVP_sha1(), NULL);
        HMAC_Update(hmac, (unsigned char*)data.c_str(), data.length());
        unsigned int len = SHA_DIGEST_LENGTH;
        HMAC_Final(hmac, hash, &len);
        HMAC_CTX_free(hmac);

        std::stringstream ss;
        for (int i = 0; i < len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    std::string generateAuthorizationHeader(const std::string& method, const std::string& url, const std::map<std::string, std::string>& parameters) {
        std::map<std::string, std::string> oauthParameters = {
            {"oauth_consumer_key", consumerKey_},
            {"oauth_nonce", std::to_string(time(NULL))},
            {"oauth_signature_method", "HMAC-SHA1"},
            {"oauth_timestamp", std::to_string(time(NULL))},
            {"oauth_token", accessToken_},
            {"oauth_version", "1.0"}
        };

        std::map<std::string, std::string> allParameters = oauthParameters;
        allParameters.insert(parameters.begin(), parameters.end());

        std::string baseString = generateBaseString(method, url, allParameters);
        std::string signingKey = urlEncode(consumerSecret_) + "&" + urlEncode(accessTokenSecret_);
        std::string signature = calculateHMACSHA1(baseString, signingKey);
        oauthParameters["oauth_signature"] = signature;

        std::stringstream ss;
        ss << "Authorization: OAuth ";
        for (auto it = oauthParameters.begin(); it != oauthParameters.end(); ++it) {
            ss << urlEncode(it->first) << "=\"" << urlEncode(it->second) << "\"";
            if (std::next(it) != oauthParameters.end()) {
                ss << ", ";
            }
        }

        return ss.str();
    }

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((TwitterStreamClient*)userp)->processData(std::string((char*)contents, size * nmemb));
        return size * nmemb;
    }

    void processData(const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "Received Data: " << data << std::endl;
        if (terminate_) {
            cv_.notify_one();
        }
    }
};

// Function to simulate starting and stopping the Twitter stream
void simulateTwitterStream(TwitterStreamClient& client, const std::string& keywords) {
    client.startTracking(keywords);
    std::this_thread::sleep_for(std::chrono::seconds(10)); // Simulate running for 10 seconds
    client.stopTracking();
}

int main() {
    const std::string consumerKey = "your_consumer_key";
    const std::string consumerSecret = "your_consumer_secret";
    const std::string accessToken = "your_access_token";
    const std::string accessTokenSecret = "your_access_token_secret";

    TwitterStreamClient twitterClient(consumerKey, consumerSecret, accessToken, accessTokenSecret);

    std::string keywords = "example, test"; // Specify the keywords to track

    // Start and stop the Twitter stream in a separate thread
    std::thread streamThread(simulateTwitterStream, std::ref(twitterClient), std::cref(keywords));
    streamThread.join();

    return 0;
}