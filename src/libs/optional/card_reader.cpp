#include "card_reader.h"

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace card_reader {

CardReader::CardReader() = default;

CardReader::~CardReader() {
    stop_polling();
    serial_disconnect();
}

bool CardReader::initialize(const std::string& port, int baudrate) {
    if (!serial_connect(port, baudrate)) {
        return false;
    }

    // Reset the reader
    if (!send_command(CMD_RESET)) {
        spdlog::error("Failed to reset card reader");
        return false;
    }

    // Give device time to reset
    usleep(500000);

    // Reset LEDs
    led_reset();

    // Give device time to process LED reset before RADIO_ON
    usleep(200000);

    spdlog::info("Card reader initialized on {}", port);
    return true;
}

bool CardReader::serial_connect(const std::string& port, int baudrate) {
    serial_fd_ = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd_ < 0) {
        spdlog::error("Failed to open serial port {}: {}", port, strerror(errno));
        return false;
    }

    struct termios tty{};
    if (tcgetattr(serial_fd_, &tty) != 0) {
        spdlog::error("Failed to get terminal attributes: {}", strerror(errno));
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    speed_t speed;
    switch (baudrate) {
        case 38400: speed = B38400; break;
        case 115200: speed = B115200; break;
        default:
            spdlog::error("Unsupported baudrate: {}", baudrate);
            close(serial_fd_);
            serial_fd_ = -1;
            return false;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 20;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0) {
        spdlog::error("Failed to set terminal attributes: {}", strerror(errno));
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    int flags = TIOCM_DTR | TIOCM_RTS;
    ioctl(serial_fd_, TIOCMBIC, &flags);

    usleep(500000);
    tcflush(serial_fd_, TCIOFLUSH);

    return true;
}

void CardReader::serial_disconnect() {
    if (serial_fd_ >= 0) {
        close(serial_fd_);
        serial_fd_ = -1;
    }
}

bool CardReader::serial_write(const std::vector<uint8_t>& data) {
    if (serial_fd_ < 0) return false;

    ssize_t written = write(serial_fd_, data.data(), data.size());
    if (written != static_cast<ssize_t>(data.size())) {
        spdlog::error("Serial write failed: wrote {} of {} bytes", written, data.size());
        return false;
    }

    tcdrain(serial_fd_);
    return true;
}

std::optional<std::vector<uint8_t>> CardReader::serial_read_response() {
    if (serial_fd_ < 0) return std::nullopt;

    uint8_t sync_byte;
    ssize_t n = read(serial_fd_, &sync_byte, 1);
    if (n != 1) {
        return std::nullopt;
    }

    if (sync_byte != SYNC_BYTE) {
        while (read(serial_fd_, &sync_byte, 1) == 1) {
            if (sync_byte == SYNC_BYTE) break;
        }
        if (sync_byte != SYNC_BYTE) {
            return std::nullopt;
        }
    }

    uint8_t len_byte;
    n = read(serial_fd_, &len_byte, 1);
    if (n != 1) {
        return std::nullopt;
    }

    uint8_t packet_len = len_byte;
    size_t expected_total = 2 + packet_len;

    std::vector<uint8_t> rest(expected_total - 2);
    size_t total_read = 0;
    while (total_read < rest.size()) {
        n = read(serial_fd_, rest.data() + total_read, rest.size() - total_read);
        if (n <= 0) {
            return std::nullopt;
        }
        total_read += n;
    }

    std::vector<uint8_t> full_packet;
    full_packet.push_back(SYNC_BYTE);
    full_packet.push_back(packet_len);
    full_packet.insert(full_packet.end(), rest.begin(), rest.end());

    return full_packet;
}

std::vector<uint8_t> CardReader::encode_packet(const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> out;
    out.reserve(payload.size() + 4);
    out.push_back(SYNC_BYTE);

    uint8_t checksum = 0;
    for (uint8_t byte : payload) {
        if (byte == SYNC_BYTE || byte == ESCAPE_BYTE) {
            out.push_back(ESCAPE_BYTE);
            out.push_back(byte - 1);
        } else {
            out.push_back(byte);
        }
        checksum += byte;
    }
    out.push_back(checksum);
    return out;
}

std::optional<std::vector<uint8_t>> CardReader::decode_packet(const std::vector<uint8_t>& data) {
    if (data.empty() || data[0] != SYNC_BYTE) {
        return std::nullopt;
    }

    std::vector<uint8_t> out;
    bool escape = false;

    for (size_t i = 1; i + 1 < data.size(); i++) {
        uint8_t byte = data[i];
        if (escape) {
            byte += 1;
            escape = false;
        } else if (byte == ESCAPE_BYTE) {
            escape = true;
            continue;
        } else if (byte == SYNC_BYTE) {
            return std::nullopt;
        }
        out.push_back(byte);
    }

    if (out.empty()) return std::nullopt;

    uint8_t received_checksum = data.back();

    uint8_t calculated_checksum = 0;
    for (uint8_t byte : out) {
        calculated_checksum += byte;
    }

    if (calculated_checksum != received_checksum) {
        return std::nullopt;
    }

    return out;
}

bool CardReader::send_command(uint8_t cmd, const std::vector<uint8_t>& payload, bool expect_response) {
    if (serial_fd_ < 0) return false;

    uint8_t payload_len = static_cast<uint8_t>(payload.size());
    uint8_t packet_len = 5 + payload_len;

    uint8_t expected_sequence = sequence_;

    std::vector<uint8_t> frame;
    frame.reserve(packet_len);
    frame.push_back(packet_len);
    frame.push_back(0);
    frame.push_back(expected_sequence);
    frame.push_back(cmd);
    frame.push_back(payload_len);
    frame.insert(frame.end(), payload.begin(), payload.end());

    sequence_ = (sequence_ + 1) & 0xFF;

    std::vector<uint8_t> encoded = encode_packet(frame);

    if (!serial_write(encoded)) {
        return false;
    }

    if (!expect_response) {
        return true;
    }

    usleep(50000);

    for (int attempt = 0; attempt < 5; attempt++) {
        auto raw_response = serial_read_response();
        if (!raw_response) {
            usleep(10000);
            continue;
        }

        auto decoded = decode_packet(*raw_response);
        if (!decoded) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if (decoded->size() < 6) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if ((*decoded)[2] != expected_sequence) {
        }

        uint8_t status = (*decoded)[4];
        if (status != 0) {
            spdlog::warn("Command 0x{:02x} returned status 0x{:02x}", cmd, status);
        }

        return true;
    }

    spdlog::error("Command 0x{:02x} failed after retries", cmd);
    return false;
}

bool CardReader::send_command_with_payload(uint8_t cmd, const std::vector<uint8_t>& payload, std::vector<uint8_t>* out_payload, bool expect_response) {
    if (serial_fd_ < 0) return false;

    uint8_t payload_len = static_cast<uint8_t>(payload.size());
    uint8_t packet_len = 5 + payload_len;

    uint8_t expected_sequence = sequence_;

    std::vector<uint8_t> frame;
    frame.reserve(packet_len);
    frame.push_back(packet_len);
    frame.push_back(0);
    frame.push_back(expected_sequence);
    frame.push_back(cmd);
    frame.push_back(payload_len);
    frame.insert(frame.end(), payload.begin(), payload.end());

    sequence_ = (sequence_ + 1) & 0xFF;

    std::vector<uint8_t> encoded = encode_packet(frame);

    if (!serial_write(encoded)) {
        return false;
    }

    if (!expect_response) {
        return true;
    }

    usleep(50000);

    for (int attempt = 0; attempt < 5; attempt++) {
        auto raw_response = serial_read_response();
        if (!raw_response) {
            usleep(10000);
            continue;
        }

        auto decoded = decode_packet(*raw_response);
        if (!decoded) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if (decoded->size() < 6) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if ((*decoded)[2] != expected_sequence) {
        }

        uint8_t status = (*decoded)[4];
        if (status != 0) {
            spdlog::warn("Command 0x{:02x} returned status 0x{:02x}", cmd, status);
        }

        if (out_payload) {
            uint8_t resp_payload_len = (*decoded)[5];
            if (resp_payload_len > 0 && decoded->size() >= 6 + resp_payload_len) {
                *out_payload = std::vector<uint8_t>(decoded->data() + 6, decoded->data() + 6 + resp_payload_len);
            }
        }

        return true;
    }

    spdlog::error("Command 0x{:02x} failed after retries", cmd);
    return false;
}

bool CardReader::start_polling() {
    if (is_polling_) return true;

    if (!send_command(CMD_RADIO_ON, {POLL_MODE_BOTH})) {
        spdlog::error("Failed to turn on NFC radio");
        return false;
    }

    is_polling_ = true;
    card_info_ = CardInfo{};
    return true;
}

void CardReader::stop_polling() {
    if (!is_polling_) return;

    send_command(CMD_RADIO_OFF);
    is_polling_ = false;
}

bool CardReader::poll_once() {
    if (!is_polling_ || serial_fd_ < 0) return false;

    uint8_t expected_sequence = sequence_;

    uint8_t payload_len = 0;
    uint8_t packet_len = 5 + payload_len;

    std::vector<uint8_t> frame;
    frame.reserve(packet_len);
    frame.push_back(packet_len);
    frame.push_back(0);
    frame.push_back(expected_sequence);
    frame.push_back(CMD_POLL);
    frame.push_back(payload_len);

    sequence_ = (sequence_ + 1) & 0xFF;

    std::vector<uint8_t> encoded = encode_packet(frame);

    if (!serial_write(encoded)) {
        return false;
    }

    usleep(50000);

    for (int attempt = 0; attempt < 3; attempt++) {
        auto raw_response = serial_read_response();
        if (!raw_response) {
            usleep(10000);
            continue;
        }

        auto decoded = decode_packet(*raw_response);
        if (!decoded) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if (decoded->size() < 6) {
            tcflush(serial_fd_, TCIFLUSH);
            usleep(10000);
            continue;
        }

        if ((*decoded)[2] != expected_sequence) {
        }

        uint8_t status = (*decoded)[4];
        if (status != 0) {
            if (card_info_.valid) {
                card_info_ = CardInfo{};
            }
            return false;
        }

        uint8_t resp_payload_len = (*decoded)[5];
        if (resp_payload_len == 0 || decoded->size() < 6 + resp_payload_len) {
            if (card_info_.valid) {
                card_info_ = CardInfo{};
            }
            return false;
        }

        const uint8_t* resp_payload = decoded->data() + 6;

        uint8_t count = resp_payload[0];
        size_t offset = 1;

        for (uint8_t i = 0; i < count; i++) {
            if (offset + 2 > resp_payload_len) break;
            uint8_t card_type = resp_payload[offset];
            uint8_t card_size = resp_payload[offset + 1];
            offset += 2;

            if (offset + card_size > resp_payload_len) break;

            std::vector<uint8_t> card_data(resp_payload + offset, resp_payload + offset + card_size);
            offset += card_size;

            if (card_type == CARD_TYPE_MIFARE && card_size == 4) {
                if (handle_mifare_card(card_data)) {
                    return true;
                }
            } else if (card_type == CARD_TYPE_FELICA && card_size == 16) {
                if (handle_felica_card(card_data)) {
                    return true;
                }
            }
        }

        if (card_info_.valid) {
            card_info_ = CardInfo{};
        }
        return false;
    }

    return false;
}

bool CardReader::handle_mifare_card(const std::vector<uint8_t>& uid_bytes) {
    if (uid_bytes.size() != 4) return false;

    uint32_t uid = static_cast<uint32_t>(uid_bytes[0]) |
                   (static_cast<uint32_t>(uid_bytes[1]) << 8) |
                   (static_cast<uint32_t>(uid_bytes[2]) << 16) |
                   (static_cast<uint32_t>(uid_bytes[3]) << 24);

    std::vector<uint8_t> select_payload(4);
    select_payload[0] = uid_bytes[0];
    select_payload[1] = uid_bytes[1];
    select_payload[2] = uid_bytes[2];
    select_payload[3] = uid_bytes[3];

    if (!send_command(CMD_MIFARE_SELECT, select_payload)) {
        return false;
    }

    std::vector<uint8_t> auth_payload(5);
    auth_payload[0] = uid_bytes[0];
    auth_payload[1] = uid_bytes[1];
    auth_payload[2] = uid_bytes[2];
    auth_payload[3] = uid_bytes[3];
    auth_payload[4] = 0x03;

    if (!send_command(CMD_MIFARE_AUTHENTICATE, auth_payload)) {
        return false;
    }

    std::vector<uint8_t> read_payload(5);
    read_payload[0] = uid_bytes[0];
    read_payload[1] = uid_bytes[1];
    read_payload[2] = uid_bytes[2];
    read_payload[3] = uid_bytes[3];
    read_payload[4] = 0x02;

    std::vector<uint8_t> block_data;
    if (!send_command_with_payload(CMD_MIFARE_READ_BLOCK, read_payload, &block_data)) {
        return false;
    }

    if (block_data.size() < 16) {
        return false;
    }

    std::vector<uint8_t> aime_id(block_data.begin() + 6, block_data.begin() + 16);

    std::string hex;
    hex.reserve(20);
    for (uint8_t b : aime_id) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", b);
        hex += buf;
    }

    card_info_ = CardInfo{
        .type = CardType::MIFARE,
        .card_id_hex = hex,
        .mifare_uid = uid,
        .valid = true
    };

    return true;
}

bool CardReader::handle_felica_card(const std::vector<uint8_t>& idm_bytes) {
    if (idm_bytes.size() != 16) return false;

    std::string hex;
    hex.reserve(16);
    for (int i = 0; i < 8; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", idm_bytes[i]);
        hex += buf;
    }

    card_info_ = CardInfo{
        .type = CardType::FELICA,
        .card_id_hex = hex,
        .mifare_uid = 0,
        .valid = true
    };

    return true;
}

void CardReader::set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    send_command(CMD_LED_SET_COLOR, {r, g, b}, false);
}

void CardReader::led_reset() {
    send_command(CMD_LED_RESET, {}, false);
}

}  // namespace card_reader