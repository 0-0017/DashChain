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
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ripemd.h>
#include <openssl/provider.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <random>
#include <atomic>
#include <cstdint> 
#include <fstream>
#include <algorithm> // For std::copy
#include <cstring> // For std::memcpy
#include <nlohmann/json.hpp>
using json = nlohmann::json;


class util
{
public:

	/* Time Stamp Function, Sets current timestamp */
	unsigned long long TimeStamp();
	std::vector<uint8_t> shaHash(const unsigned char* data, bool isString = false);
	unsigned char* ripemd(const unsigned char pubKey[80]);


	/* To String Method Using Templates for Generic Typing */
	template <typename T>
	std::string toString(const T& value) {
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

		return ss.str();
	}

	template <typename T>
	unsigned char* toUnsignedChar(const T& value) {
		unsigned char* result = nullptr;

		/* Case 1: Handle std::string */
		if constexpr (std::is_same_v<T, std::string>) {
			result = new unsigned char[value.length() + 1];
			std::memcpy(result, value.c_str(), value.length() + 1);
		}
		/* Case 2: Handle std::vector<uint8_t> */
		else if constexpr (std::is_same_v <T, std::vector<uint8_t>> || std::is_same_v <T, std::vector<unsigned char>>) {
			result = new unsigned char[value.size()];
			std::memcpy(result, value.data(), value.size());
		}
		/* Case 3: Handle std::vector<nlohmann::json> */
		else if constexpr (std::is_same_v<T, std::vector<nlohmann::json>>) {
			std::string jsonStr;
			for (const auto& elem : value) {
				jsonStr += elem.dump(); // Convert JSON to string and concatenate
			}
			result = new unsigned char[jsonStr.length() + 1];
			std::memcpy(result, jsonStr.c_str(), jsonStr.length() + 1); // Copy to unsigned char*
		}
		/* Case 4: Handle unsigned long long */
		else if constexpr (std::is_same_v<T, unsigned long long>) {
			result = new unsigned char[sizeof(unsigned long long)];
			std::memcpy(result, &value, sizeof(unsigned long long));
		}
		/* Unsupported type */
		else {
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>
				|| std::is_same_v<T, std::vector<uint32_t>> || std::is_same_v<T, std::vector<nlohmann::json>>,
				"Unsupported type for toUnsignedChar");
		}

		return result;
	}


	template<typename T>
	const char* toConstChar(const T& input) {
		if constexpr (std::is_same_v<T, std::string>) {
			return input.c_str();
		}
		else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
			return reinterpret_cast<const char*>(input.data());
		}
		else {
			// You can add more specialized cases for other types if needed
			static_assert(std::false_type::value, "Unsupported type for toConstChar");
		}
	}
	/* Random Number Generator Function, Generates 17 Digit Random Number */
	std::string genRandNum();

	/* Prompt User for password used to create keys */
	std::string getPasswordFromUser();

	/* Base58 Encoding, Commonly used so letters/numbers dont get confused */
	const std::string base58_chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	std::string base58_encode(const unsigned char* bytes, size_t size);

};

#endif

