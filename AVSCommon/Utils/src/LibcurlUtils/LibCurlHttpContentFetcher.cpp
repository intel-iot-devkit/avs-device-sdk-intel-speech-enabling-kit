/*
 * LibCurlHttpContentFetcher.cpp
 *
 * Copyright 2016-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <algorithm>

#include <AVSCommon/Utils/LibcurlUtils/CurlEasyHandleWrapper.h>
#include <AVSCommon/Utils/LibcurlUtils/LibCurlHttpContentFetcher.h>
#include <AVSCommon/Utils/Memory/Memory.h>
#include <AVSCommon/Utils/SDS/InProcessSDS.h>

namespace alexaClientSDK {
namespace avsCommon {
namespace utils {
namespace libcurlUtils {

/// String to identify log entries originating from this file.
static const std::string TAG("LibCurlHttpContentFetcher");

/**
 * The timeout for a blocking write call to an @c AttachmentWriter. This value may be increased to decrease wakeups but
 * may also increase latency.
 */
static const std::chrono::milliseconds TIMEOUT_FOR_BLOCKING_WRITE = std::chrono::milliseconds(100);

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

size_t LibCurlHttpContentFetcher::headerCallback(char* data, size_t size, size_t nmemb, void* userData) {
    if (!userData) {
        ACSDK_ERROR(LX("headerCallback").d("reason", "nullUserDataPointer"));
        return 0;
    }
    std::string line(static_cast<const char*>(data), size * nmemb);
    std::transform(line.begin(), line.end(), line.begin(), ::tolower);
    if (line.find("http") == 0) {
        // To find lines like: "HTTP/1.1 200 OK"
        std::istringstream iss(line);
        std::string httpVersion;
        long statusCode;
        iss >> httpVersion >> statusCode;
        LibCurlHttpContentFetcher* thisObject = static_cast<LibCurlHttpContentFetcher*>(userData);
        thisObject->m_lastStatusCode = statusCode;
    } else if (line.find("content-type") == 0) {
        // To find lines like: "Content-Type: audio/x-mpegurl; charset=utf-8"
        std::istringstream iss(line);
        std::string contentTypeBeginning;
        std::string contentType;
        iss >> contentTypeBeginning >> contentType;

        size_t separator = contentType.find(";");
        if (separator != std::string::npos) {
            // Remove characters after the separator ;
            contentType.erase(separator);
        }
        LibCurlHttpContentFetcher* thisObject = static_cast<LibCurlHttpContentFetcher*>(userData);
        thisObject->m_lastContentType = contentType;
    }
    return size * nmemb;
}

size_t LibCurlHttpContentFetcher::bodyCallback(char* data, size_t size, size_t nmemb, void* userData) {
    if (!userData) {
        ACSDK_ERROR(LX("bodyCallback").d("reason", "nullUserDataPointer"));
        return 0;
    }
    LibCurlHttpContentFetcher* thisObject = static_cast<LibCurlHttpContentFetcher*>(userData);
    if (thisObject->m_shuttingDown) {
        // In order to properly quit when downloading live content, which block forever when performing a GET request
        return 0;
    }
    if (!thisObject->m_bodyCallbackBegan) {
        thisObject->m_bodyCallbackBegan = true;
        thisObject->m_statusCodePromise.set_value(thisObject->m_lastStatusCode);
        thisObject->m_contentTypePromise.set_value(thisObject->m_lastContentType);
    }
    auto streamWriter = thisObject->m_streamWriter;
    size_t totalBytesWritten = 0;

    if (streamWriter) {
        size_t targetNumBytes = size * nmemb;

        while (totalBytesWritten < targetNumBytes && !thisObject->m_shuttingDown) {
            avsCommon::avs::attachment::AttachmentWriter::WriteStatus writeStatus =
                avsCommon::avs::attachment::AttachmentWriter::WriteStatus::OK;

            size_t numBytesWritten = streamWriter->write(
                data,
                targetNumBytes - totalBytesWritten,
                &writeStatus,
                std::chrono::milliseconds(TIMEOUT_FOR_BLOCKING_WRITE));
            totalBytesWritten += numBytesWritten;
            data += numBytesWritten;

            switch (writeStatus) {
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::CLOSED:
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE:
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::ERROR_INTERNAL:
                    return totalBytesWritten;
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::TIMEDOUT:
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::OK:
                    // might still have bytes to write
                    continue;
                case avsCommon::avs::attachment::AttachmentWriter::WriteStatus::OK_BUFFER_FULL:
                    ACSDK_ERROR(LX(__func__).d("unexpected return code", "OK_BUFFER_FULL"));
                    return 0;
            }
            ACSDK_ERROR(LX(__func__).m("unexpected writeStatus"));
            return 0;
        }
    }
    return totalBytesWritten;
}

size_t LibCurlHttpContentFetcher::noopCallback(char* data, size_t size, size_t nmemb, void* userData) {
    return 0;
}

LibCurlHttpContentFetcher::LibCurlHttpContentFetcher(const std::string& url) :
        m_url{url},
        m_bodyCallbackBegan{false},
        m_lastStatusCode{0},
        m_shuttingDown{false} {
    m_hasObjectBeenUsed.clear();
}

std::unique_ptr<avsCommon::utils::HTTPContent> LibCurlHttpContentFetcher::getContent(FetchOptions fetchOption) {
    if (m_hasObjectBeenUsed.test_and_set()) {
        return nullptr;
    }
    if (!m_curlWrapper.setURL(m_url)) {
        ACSDK_ERROR(LX("getContentFailed").d("reason", "failedToSetUrl"));
        return nullptr;
    }
    auto curlReturnValue = curl_easy_setopt(m_curlWrapper.getCurlHandle(), CURLOPT_FOLLOWLOCATION, 1L);
    if (curlReturnValue != CURLE_OK) {
        ACSDK_ERROR(LX("getContentFailed").d("reason", "enableFollowRedirectsFailed"));
        return nullptr;
    }
    curlReturnValue = curl_easy_setopt(m_curlWrapper.getCurlHandle(), CURLOPT_AUTOREFERER, 1L);
    if (curlReturnValue != CURLE_OK) {
        ACSDK_ERROR(LX("getContentFailed").d("reason", "enableAutoReferralSettingToRedirectsFailed"));
        return nullptr;
    }
    // This enables the libcurl cookie engine, allowing it to send cookies
    curlReturnValue = curl_easy_setopt(m_curlWrapper.getCurlHandle(), CURLOPT_COOKIEFILE, "");
    if (curlReturnValue != CURLE_OK) {
        ACSDK_ERROR(LX("getContentFailed").d("reason", "enableLibCurlCookieEngineFailed"));
        return nullptr;
    }
    auto httpStatusCodeFuture = m_statusCodePromise.get_future();
    auto contentTypeFuture = m_contentTypePromise.get_future();
    std::shared_ptr<avsCommon::avs::attachment::InProcessAttachment> stream = nullptr;
    switch (fetchOption) {
        case FetchOptions::CONTENT_TYPE:
            /*
             * Since this option only wants the content-type, I set a noop callback for parsing the body of the HTTP
             * response. For some webpages, it is required to set a body callback in order for the full webpage data
             * to render.
             */
            curlReturnValue = curl_easy_setopt(m_curlWrapper.getCurlHandle(), CURLOPT_WRITEFUNCTION, noopCallback);
            if (curlReturnValue != CURLE_OK) {
                ACSDK_ERROR(LX("getContentFailed").d("reason", "failedToSetCurlCallback"));
                return nullptr;
            }
            m_thread = std::thread([this]() {
                long finalResponseCode = 0;
                char* contentType = nullptr;
                auto curlReturnValue = curl_easy_perform(m_curlWrapper.getCurlHandle());
                if (curlReturnValue != CURLE_OK && curlReturnValue != CURLE_WRITE_ERROR) {
                    ACSDK_ERROR(LX("curlEasyPerformFailed").d("error", curl_easy_strerror(curlReturnValue)));
                }
                curlReturnValue =
                    curl_easy_getinfo(m_curlWrapper.getCurlHandle(), CURLINFO_RESPONSE_CODE, &finalResponseCode);
                if (curlReturnValue != CURLE_OK) {
                    ACSDK_ERROR(LX("curlEasyGetInfoFailed").d("error", curl_easy_strerror(curlReturnValue)));
                }
                ACSDK_DEBUG9(LX("getContent").d("responseCode", finalResponseCode).sensitive("url", m_url));
                m_statusCodePromise.set_value(finalResponseCode);
                curlReturnValue = curl_easy_getinfo(m_curlWrapper.getCurlHandle(), CURLINFO_CONTENT_TYPE, &contentType);
                if (curlReturnValue == CURLE_OK && contentType) {
                    ACSDK_DEBUG9(LX("getContent").d("contentType", contentType).sensitive("url", m_url));
                    m_contentTypePromise.set_value(std::string(contentType));
                } else {
                    ACSDK_ERROR(LX("curlEasyGetInfoFailed").d("error", curl_easy_strerror(curlReturnValue)));
                    ACSDK_ERROR(LX("getContent").d("contentType", "failedToGetContentType").sensitive("url", m_url));
                    m_contentTypePromise.set_value("");
                }
            });
            break;
        case FetchOptions::ENTIRE_BODY:
            // Using the url as the identifier for the attachment
            stream = std::make_shared<avsCommon::avs::attachment::InProcessAttachment>(m_url);
            m_streamWriter = stream->createWriter(sds::WriterPolicy::BLOCKING);
            if (!m_streamWriter) {
                ACSDK_ERROR(LX("getContentFailed").d("reason", "failedToCreateWriter"));
                return nullptr;
            }
            if (!m_curlWrapper.setWriteCallback(bodyCallback, this)) {
                ACSDK_ERROR(LX("getContentFailed").d("reason", "failedToSetCurlBodyCallback"));
                return nullptr;
            }
            if (!m_curlWrapper.setHeaderCallback(headerCallback, this)) {
                ACSDK_ERROR(LX("getContentFailed").d("reason", "failedToSetCurlHeaderCallback"));
                return nullptr;
            }
            m_thread = std::thread([this]() {
                auto curlReturnValue = curl_easy_perform(m_curlWrapper.getCurlHandle());
                if (curlReturnValue != CURLE_OK) {
                    ACSDK_ERROR(LX("curlEasyPerformFailed").d("error", curl_easy_strerror(curlReturnValue)));
                }
                if (!m_bodyCallbackBegan) {
                    m_statusCodePromise.set_value(m_lastStatusCode);
                    m_contentTypePromise.set_value(m_lastContentType);
                }
                /*
                 * Curl easy perform has finished and all data has been written. Closing writer so that readers know
                 * when they have caught up and read everything.
                 */
                m_streamWriter->close();
            });
            break;
        default:
            return nullptr;
    }
    return avsCommon::utils::memory::make_unique<avsCommon::utils::HTTPContent>(
        avsCommon::utils::HTTPContent{std::move(httpStatusCodeFuture), std::move(contentTypeFuture), stream});
}

LibCurlHttpContentFetcher::~LibCurlHttpContentFetcher() {
    m_shuttingDown = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

}  // namespace libcurlUtils
}  // namespace utils
}  // namespace avsCommon
}  // namespace alexaClientSDK
