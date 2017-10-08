/**
 * SocketHardwareController.cpp
 *
 * TODO: Add Intel copyright
 */

#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

#include <AVSCommon/Utils/Logger/Logger.h>

#include "Socket/SocketHardwareController.h"

namespace alexaClientSDK {
namespace kwd {

// Logging tag
static const std::string TAG("SocketHardwareController");

#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
#define MAX_MSG_LEN 512

std::shared_ptr<SocketHardwareController> SocketHardwareController::create(
        std::string host, int port)
{
    std::shared_ptr<SocketHardwareController> ctrl = std::make_shared<SocketHardwareController>(
            SocketHardwareController(host, port));

    if(!ctrl->init()) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initHardwareControllerFailed"));
        return nullptr;
    }

    return ctrl;
}

std::unique_ptr<KeywordDetection> SocketHardwareController::read(
        std::chrono::milliseconds timeout)
{
    struct timeval tv;
    fd_set rfds;
    int retval;

    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;
        
    FD_ZERO(&rfds);
    FD_SET(m_sockFd, &rfds);

    if((retval = select(m_sockFd + 1, &rfds, NULL, NULL, &tv)) == -1) {
        ACSDK_ERROR(LX("readFailed").d("reason", "selectReadFailed"));
        return nullptr;
    }
    
    // Timeout happended
    if(!retval) {
        return nullptr;
    }

    char buff[MAX_MSG_LEN];
    memset(buff, '\0', sizeof(buff));
    int len = recv(m_sockFd, buff, sizeof(buff) - 1, 0);

    if(len < 0) {
        ACSDK_ERROR(LX("readFailed").d("reason", "recvFailed"));
        return nullptr;
    } else if(len > 0) {
        std::string kw(buff);
        return KeywordDetection::create(-1, -1, kw);
    } else {
        // For some reason no data was actually read
        return nullptr;
    }
}

SocketHardwareController::~SocketHardwareController() {
    close(m_sockFd);
}

SocketHardwareController::SocketHardwareController(std::string host, int port) :
    m_host(host), m_port(port), m_sockFd(-1)
{}

bool SocketHardwareController::init() {
    struct sockaddr_in servAddr;
    struct hostent* server;

    server = gethostbyname(m_host.c_str());
    if(server == NULL) {
        ACSDK_ERROR(LX("initFailed").d("reason", "getHostFailed"));
        return false;
    }

    if((m_sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ACSDK_ERROR(LX("initFailed").d("reason", "createSocketFailed"));
        return false;
    }

    bzero((char*) &servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &servAddr.sin_addr.s_addr, server->h_length);
    servAddr.sin_port = htons(m_port);

    if(connect(m_sockFd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
        ACSDK_ERROR(LX("initFailed").d("reason", "socketConnectFailed"));
        return false;
    }

    // Freeing server
    free(server); 

    return true;    
}

} // kwd
} // alexaClientSDK
