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

class TwitterClient {
public:
    TwitterClient(const std::string& consumerKey, const std::string& consumerSecret, const std::string& accessToken, const std::string& accessTokenSecret)
        : consumerKey_(consumerKey), consumerSecret_(consumerSecret), accessToken_(accessToken), accessTokenSecret_(accessTokenSecret) {}

    // Function to post a tweet
    void postTweet(const std::string& status) {
        std::string url = "https://api.twitter.com/1.1/statuses/update.json";
        std::map<std::string, std::string> parameters;
        parameters["status"] = status;

        std::string authorizationHeader = generateAuthorizationHeader("POST", url, parameters);

        CURL* curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, authorizationHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

            std::string postFields = "status=" + curl_easy_escape(curl, status.c_str(), status.length());

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);
        }
    }

private:
    std::string consumerKey_;
    std::string consumerSecret_;
    std::string accessToken_;
    std::string accessTokenSecret_;

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
};

// Function to simulate posting a tweet
void simulateTweetPosting(TwitterClient& client, const std::vector<std::string>& tweets) {
    for (const auto& tweet : tweets) {
        client.postTweet(tweet);
    }
}

int main() {
    const std::string consumerKey = "your_consumer_key";
    const std::string consumerSecret = "your_consumer_secret";
    const std::string accessToken = "your_access_token";
    const std::string accessTokenSecret = "your_access_token_secret";

    TwitterClient twitterClient(consumerKey, consumerSecret, accessToken, accessTokenSecret);

    // List of tweets for simulation
    std::vector<std::string> tweets = {
        "Hello, Twitter!",
        "This is a test tweet from my C++ bot.",
        "Building bots is fun!"
    };

    // Simulate posting tweets in a separate thread
    std::thread postThread(simulateTweetPosting, std::ref(twitterClient), std::cref(tweets));
    postThread.join();

    return 0;
}