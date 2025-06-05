#pragma once

#include <nlohmann/json.hpp>

namespace qase {
    void validate_json_payload(const nlohmann::json& payload);
}
