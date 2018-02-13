/*
 * StringUtils.cpp
 *
 * Copyright 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "AVSCommon/Utils/String/StringUtils.h"

#include <iomanip>
#include <limits>

#include <errno.h>
#include <stdlib.h>

#include "AVSCommon/Utils/Logger/Logger.h"

namespace alexaClientSDK {
namespace avsCommon {
namespace utils {
namespace string {

using namespace avsCommon::utils::logger;

/// String to identify log entries originating from this file.
static const std::string TAG("StringUtils");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

static const int BASE_TEN = 10;

bool stringToInt(const std::string& str, int* result) {
    return stringToInt(str.c_str(), result);
}

bool stringToInt(const char* str, int* result) {
    if (!str) {
        ACSDK_ERROR(LX("stringToIntFailed").m("str parameter is null."));
        return false;
    }
    if (!result) {
        ACSDK_ERROR(LX("stringToIntFailed").m("result parameter is null."));
        return false;
    }

    // ensure errno is set to zero before calling strtol.
    errno = 0;
    char* endPtr = nullptr;
    long tempResult = strtol(str, &endPtr, BASE_TEN);

    // If strtol fails, then endPtr will still point to the beginning of str - a simple way to detect error.
    if (str == endPtr) {
        ACSDK_ERROR(LX("stringToIntFailed").m("input string was not parseable as an integer."));
        return false;
    }

    if (ERANGE == errno || tempResult < std::numeric_limits<int>::min() ||
        tempResult > std::numeric_limits<int>::max()) {
        ACSDK_ERROR(LX("stringToIntFailed").m("converted number was out of range."));
        return false;
    }

    *result = static_cast<int>(tempResult);
    return true;
}

std::string byteVectorToString(const std::vector<uint8_t>& byteVector) {
    std::stringstream ss;
    bool firstTime = true;
    for (const auto& byte : byteVector) {
        ss << std::hex << (firstTime ? "" : " ") << "0x" << std::setfill('0') << std::setw(2) << int(byte) << std::dec;
        firstTime = false;
    }
    return ss.str();
}

}  // namespace string
}  // namespace utils
}  // namespace avsCommon
}  // namespace alexaClientSDK
