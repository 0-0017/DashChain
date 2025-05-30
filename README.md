# Project DASH

Project DASH merges the realms of cryptocurrency and blockchain technology to pioneer innovative investment solutions. This blockchain implementation offers a secure and efficient framework for managing digital transactions, with future plans to integrate AI for enhanced decision-making capabilities.

## Table of Contents

- [Overview](#overview)
- [License](#license)
- [Installation](#installation)
- [Build Instructions](#build-instructions)
- [Key Components](#key-components)
- [Usage](#usage)
- [What's Next](#whats-next)

## Overview

Project DASH utilizes a robust blockchain architecture that emphasizes security, efficiency, and scalability. The project features a comprehensive transaction management system, verification protocols for blocks and transactions, and an intended Delegated Proof of Stake (DPoS) consensus mechanism for governance. Future updates will also include AI-driven analytics to assist in market trend analysis and investment strategy improvements.

**Note:** Currently, this program has only been tested on **Windows systems**. Compatibility with other operating systems has not yet been confirmed.

## Installation

Ensure you have the necessary dependencies installed before proceeding:

- **OpenSSL** � Required for cryptographic functions.
- **C++ Compiler** � Ensure you have a compiler that supports C++17 or later.
- **CMake (Optional)** � Useful for managing builds across different environments.

## Build Instructions

To compile and run Project DASH:

1. Clone the repository:
   ```sh
   git clone https://github.com/0-0017/DashChain
   cd DashChain

2. Compile the program using a C++ compiler:
   ```sh
   g++ -o DashChain source.cpp -lssl -lcrypto

3. Run The Program
   ```sh
   ./DashChain


## Key Components

### Libraries
- **OpenSSL**: Utilized for cryptographic functions to ensure secure transactions and data integrity.

### Blockchain Implementation

#### Block Architecture

The main architecture of Project DASH is built on a robust blockchain implementation, leveraging key components for secure and efficient operations.

**Blockchain:**
- **GenerateBlock**: Creates a new block in the chain.
- **ValidateBlock**: Ensures the integrity and validity of each block.
- **VerifyBlock**: Validates a block before adding it to the chain.
- **TimeStamp**: Assigns a timestamp to blocks for chronological order.
- **Hash**: Utilizes cryptographic hashing for data integrity.
- **Transactions**: Manages and records transactions within the blocks.
- **VersionControl**: Implements version control for the blockchain.
- **BlockHeight**: Implements block height control for the blockchain.

**Blocks:**
Each block within the blockchain is structured with a distinct head and body.

**Head:**
- *Previous Block's Hash*
- *TimeStamp*
- *Merkle Root*
- *DPOS Values* (Delegated Proof of Stake parameters)

**Body:**
- *Current Hash*
- *Transactions*
- *Hash*
- *Block Height*
- *Cryptographic Signatures*
- *Block Version Number*
- *Metadata* (Block Creator, AI, etc.)

### Classes and Functionalities

#### 1. **Block Class (`Block.h`)**
- **Purpose**: Represents a single block in the blockchain.
- **Key Methods**:
    - `serialize()`: Serializes the block data for transmission or storage.
    - `deserialize()`: Deserializes data back into a block object.
    - `calculateHash()`: Generates a hash for the block based on its content.
    - `setMerkleRoot()`: Computes and sets the Merkle root for the transactions in the block.

#### 2. **Blockchain Class (`Blockchain.h`)**
- **Purpose**: Manages the chain of blocks and overall blockchain logic.
- **Key Methods**:
    - `GenerateBlock()`: Creates and adds a new block to the chain.
    - `ValidateBlock()`: Checks the integrity of a block.
    - `verifyBlock()`: Verifies a block before adding it to the chain.
    - `empty()`: Checks if the blockchain is empty.
    - `getTimestamp()`: Retrieves the timestamp of the current blockchain.
    - `getBlockHeight()`: Retrieves the current height of the blockchain.
    - `display()`: Displays the entire blockchain structure.

#### 3. **Wallet Class (`Wallet.h`)**
- **Purpose**: Manages user transactions and balances.
- **Key Methods**:
    - `sendTransaction()`: Initiates a transaction to send cryptocurrency.
    - `receiveTransaction()`: Receives a transaction and verifies it.
    - `getBalance()`: Returns the current balance of the wallet.
    - `verifyTx()`: Validates a transaction to ensure its integrity.

#### 4. **Coin Class (`Coin.h`)**
- **Purpose**: Represents a cryptocurrency coin, encapsulating its properties and functionalities.
- **Key Methods**:
    - `create()`: Creates a new coin with specified properties.
    - `getVotingPrivileges()`: Returns voting privileges for the coin holders.
    - `getCoinDetails()`: Provides detailed information about the coin.

#### 5. **Transaction Class (`Transaction.h`)**
- **Purpose**: Handles the details and operations related to cryptocurrency transactions.
- **Key Methods**:
    - `serialize()`: Serializes transaction data for storage or transmission.
    - `deserialize()`: Converts serialized data back into a transaction object.
    - `validate()`: Verifies the integrity and validity of a transaction.
    - `execute()`: Processes the transaction on the blockchain.

#### 6. **Network Class (`Network.h`)**
- **Purpose**: Manages peer-to-peer network communications for the blockchain.
- **Key Methods**:
    - `sendTransaction()`: Sends a transaction to peers in the network.
    - `broadcastBlock()`: Broadcasts a new block to the network.
    - `receiveData()`: Receives and processes data from peers.
    - `connect()`: Establishes connections with other nodes in the network.

#### 7. **Utility Class (`util.h`)**
- **Purpose**: Provides essential utility functions for the blockchain.
- **Key Methods**:
    - `TimeStamp()`: Generates a timestamp for blockchain entries.
    - `shaHash(const std::string& data)`: Performs SHA hashing.
    - `toString(const T& value)`: Converts various data types to strings.

### Blockchain Initialization

The project starts with the creation of a Genesis Block, which intentionally lacks a previous hash. This design choice enhances security and prevents potential leaks.

## Usage

To utilize Project DASH, follow these key steps:

1. **Installation:**
    - Ensure OpenSSL is installed, as it is a crucial library for cryptographic operations.

2. **Block and Blockchain Initialization:**
    - Create instances of the `Block`, `Blockchain`, and `Wallet` classes.

3. **Genesis Block Creation:**
    - Initialize the blockchain with a Genesis Block, setting the stage for subsequent blocks.

4. **Blockchain Operations:**
    - Generate new blocks using the `GenerateBlock` function.
    - Validate blocks for integrity using the `ValidateBlock` and `verifyBlock` functions.

5. **Timestamp and Hashing:**
    - Leverage the blockchain's timestamp and cryptographic hashing mechanisms.

6. **Version Control:**
    - Manage blockchain evolution through version control.

7. **Transactions:**
    - Record and manage transactions using the `Transaction` class and the `Wallet` class.

8. **Network Operations:**
    - Use the `Network` class to handle communications with other peers in the network.

9. **Display Blockchain:**
    - Use the `display` function to visualize the current state of the blockchain.

## What's Next
Project DASH is still in development. While currently functional, it has known memory leaks that need to be addressed. Once these are resolved, future improvements will focus on:

	- Expanding the Coin class to track more than just circulation.

	- Implementing Delegated Proof of Stake (DPoS).

	- Enhancing network functionality for better peer-to-peer interaction.