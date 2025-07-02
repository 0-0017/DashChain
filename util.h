#ifndef UTIL
#define UTIL

/*-- util.h ---------------------------------------------------------------

  This header file defines the util class for this project.
  It provides utility functions for:

  - Timestamp generation
  - Hashing using SHA and RIPEMD algorithms
  - Generic type conversion to std::string and unsigned char*
  - JSON serialization and handling of various data types
  - Base58 encoding for secure key representations
  - Random number generation
  - Secure password input from the user

-------------------------------------------------------------------------*/

#include <iostream>
#include <chrono>
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/provider.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <queue>
#include <tuple>
#include <random>
#include <atomic>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <memory>
#include <algorithm> // For std::copy
#include <cstring> // For std::mem-copy
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class util
{
public:

	/* Time Stamp Function, Sets current timestamp */
	static unsigned long long TimeStamp();
	static bool shaHash(const std::string &message, std::vector<unsigned char> &hash);
	static bool ripemd(const std::vector<unsigned char> &input, std::vector<unsigned char> &hash);


	/* To String Method Using Templates for Generic Typing */
	template <typename T>
	static std::string toString(const T& value) {
		std::stringstream ss;

		/* Check if the input is a vector<uint8_t> */
		if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
			for (const auto& elem : value) {
				ss << static_cast<int>(elem) << " "; // Convert uint8_t to int for printing
			}
		}
		/* Check if the input is a vector<nlohmann::json> */
		else if constexpr (std::is_same_v<T, std::vector<nlohmann::json>>) {
			for (const auto& elem : value) {
				ss << elem.dump() << " "; // Use the dump() method to convert json to string
			}
		}
		else {
			ss << value; // Use the default behavior for other types
		}

		logCall("UTIL", "toString()", true);
		return ss.str();
	}

	template <typename T>
	static std::unique_ptr<unsigned char[]> toUnsignedChar(const T& value) {
		std::unique_ptr<unsigned char[]> result = nullptr;

		/* Case 1: Handle std::string */
		if constexpr (std::is_same_v<T, std::string>) {
			result.reset(new unsigned char[value.length()]);
			std::memcpy(result.get(), value.c_str(), value.length());
		}
		/* Case 2: Handle std::vector<uint8_t> */
		else if constexpr (std::is_same_v <T, std::vector<uint8_t>> || std::is_same_v <T, std::vector<unsigned char>>) {
			result.reset(new unsigned char[value.size()]);
			std::memcpy(result, value.data(), value.size());
		}
		/* Case 3: Handle std::vector<nlohmann::json> */
		else if constexpr (std::is_same_v<T, std::vector<nlohmann::json>>) {
			std::string jsonStr;
			for (const auto& elem : value) {
				jsonStr += elem.dump(); // Convert JSON to string and concatenate
			}
			result.reset(new unsigned char[jsonStr.length()]);
			std::memcpy(result.get(), jsonStr.c_str(), jsonStr.length()); // Copy to unsigned char*
		}
		/* Case 4: Handle unsigned long-long */
		else if constexpr (std::is_same_v<T, unsigned long long>) {
			result.reset(new unsigned char[sizeof(unsigned long long)]);
			std::memcpy(result.get(), &value, sizeof(unsigned long long));
		}
		/* Unsupported type */
		else {
			logCall("UTIL", "Unsupported type for toUnsignedChar()", true);
			std::cerr << "Unsupported type for toUnsignedChar!\n";
		}

		return result;
	}


	template<typename T>
	static const char* toConstChar(const T& input) {
		if constexpr (std::is_same_v<T, std::string>) {
			return input.c_str();
		}
		else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
			return reinterpret_cast<const char*>(input.data());
		}
		else {
			// You can add more specialized cases for other types if needed
			logCall("UTIL", "toConstChar()", false, "Unsupported type for toConstChar");
			std::cerr << "Unsupported type for toConstChar\n";
		}
	}


	static void logCall(const std::string& className, const std::string& methodName, bool success, const std::string& error = "NONE") {
		std::ofstream logFile("log.txt", std::ios::app);

		if (!logFile) {
			std::cerr << "Error opening log file." << std::endl;
			return;
		}

		// Get current timestamp
		std::time_t now = std::time(nullptr);
		char timestamp[20];
		std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

		// Write log entry
		logFile << timestamp << " | " << className << " | " << methodName << " | "
				<< (success ? "SUCCESS" : "FAIL") << " | " << error << std::endl;
	}

	/* Random Number Generator Function, Generates 17 Digit Random Number */
	static std::string genRandNum();

	/* Prompt User for password used to create keys */
	static std::string getPasswordFromUser();

	/* Base58 Encoding, Commonly used so letters/numbers don't get confused */
	static std::string base58_encode(const unsigned char* bytes, size_t size);

};

#endif
