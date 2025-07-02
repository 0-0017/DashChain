#include "Trainer.h"

Trainer::Trainer() {
    MAX_DATA_LENGTH = 6;
}

void Trainer::load_data(const std::vector<double>& data) {
    // Safety check
    if (data.size() < MAX_DATA_LENGTH) {
        std::cerr << "Trainer received insufficient data: expected " << MAX_DATA_LENGTH << ", got " << data.size() << std::endl;
        return;
    }

    // Build JSON string with named fields to match FastAPI model
    std::ostringstream json_stream;
    json_stream << "{";
    json_stream << R"("total_supply": )" << data[0] << ", ";
    json_stream << R"("circ_supply": )" << data[1] << ", ";
    json_stream << R"("balance": )" << data[2] << ", ";
    json_stream << R"("votes": )" << data[3] << ", ";
    json_stream << R"("height": )" << data[4] << ", ";
    json_stream << R"("tx_volume": )" << data[5];
    json_stream << "}";

    json_body = json_stream.str();

    // Optional: log the payload for debugging
    std::cout << "[Trainer] Prepared JSON payload: " << json_body << std::endl;
}

std::vector<double> Trainer::train() const {
    std::vector<double> error;
    try {
        asio::io_context io_context;


        // Resolve the host
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("localhost", "8000");

        // Connect to the server
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        // Build HTTP POST request
        std::ostringstream request_stream;
        request_stream << "POST /predict/ HTTP/1.1\r\n";
        request_stream << "Host: localhost:8000\r\n";
        request_stream << "Content-Type: application/json\r\n";
        request_stream << "Content-Length: " << json_body.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << json_body;

        // Send the request
        asio::write(socket, asio::buffer(request_stream.str()));

        // Read the response
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Parse status line
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;
        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);
        std::cout << "Response: " << status_code << " " << status_message << "\n";

        // Read headers
        asio::read_until(socket, response, "\r\n\r\n");
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            std::cout << header << "\n";
        }

        // Read body
        std::ostringstream body;
        if (response.size() > 0)
            body << &response;

        // Convert to JSON and extract vector
        std::vector<double> predictions;
        try {
            json parsed = json::parse(body.str());
            if (parsed.contains("prediction") && parsed["prediction"].is_array()) {
                predictions = parsed["prediction"].get<std::vector<double>>();
            }
            return predictions;
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            return predictions;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return error;
}
