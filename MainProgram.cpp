import tweepy
import threading
import time
import logging

# Twitter API credentials
CONSUMER_KEY = 'your_consumer_key'
CONSUMER_SECRET = 'your_consumer_secret'
ACCESS_TOKEN = 'your_access_token'
ACCESS_TOKEN_SECRET = 'your_access_token_secret'

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')


# Setup Tweepy API
def setup_twitter_api():
    auth = tweepy.OAuth1UserHandler(CONSUMER_KEY, CONSUMER_SECRET, ACCESS_TOKEN, ACCESS_TOKEN_SECRET)
    return tweepy.API(auth)


# Function to post a tweet
def post_tweet(api, status):
    try:
        api.update_status(status=status)
        logging.info(f"Tweet posted: {status}")
    except tweepy.TweepError as e:
        logging.error(f"Error posting tweet: {e}")


# Real-time tweet listener
class StreamListener(tweepy.StreamListener):
    def on_status(self, status):
        logging.info(f"Tweet received: {status.text}")

    def on_error(self, status_code):
        if status_code == 420:
            logging.error(f"Rate limit exceeded: {status_code}")
            return False
        else:
            logging.error(f"Error code: {status_code}")
            return True


# Function to start tracking real-time tweets
def start_tracking(api, keywords):
    stream_listener = StreamListener()
    stream = tweepy.Stream(auth=api.auth, listener=stream_listener)
    stream.filter(track=keywords, is_async=True)
    return stream


# Function to stop tracking real-time tweets
def stop_tracking(stream):
    stream.disconnect()


def simulate_tweet_posting(api, tweets):
    for tweet in tweets:
        post_tweet(api, tweet)
        time.sleep(2)  # Simulate some delay


def main():
    api = setup_twitter_api()

    # List of tweets for simulation
    tweets = [
        "Hello, Twitter from Python!",
        "This is a test tweet from my Python bot.",
        "Building bots is fun in Python!"
    ]

    # Simulate posting tweets in a separate thread
    tweet_thread = threading.Thread(target=simulate_tweet_posting, args=(api, tweets))
    tweet_thread.start()

    # Keywords to track
    keywords = ["example", "test"]

    # Start tracking keywords
    stream = start_tracking(api, keywords)

    # Wait for a while before stopping the stream
    try:
        time.sleep(30)  # Adjust duration based on your needs
    except KeyboardInterrupt:
        logging.info("Interrupted by user")

    stop_tracking(stream)

    # Wait for tweet thread to finish
    tweet_thread.join()


if __name__ == "__main__":
    main()
