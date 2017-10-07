/**
 * SocketHardwareController.h
 *
 * TODO: Add Intel copyright
 */

#ifndef ALEXA_CLIENT_SDK_KWD_SOCKET_HW_CTRL_H_
#define ALEXA_CLIENT_SDK_KWD_SOCKET_HW_CTRL_H_

#include <chrono>
#include <string>
#include <memory>

#include "HardwareController/AbstractHardwareController.h"

namespace alexaClientSDK {
namespace kwd {

class SocketHardwareController : public AbstractHardwareController {
public:
    /**
     * Creates a new pointer to a @c SocketHardwareController. 
     *
     * @param host Host of the socket server to connect to
     * @param port Port on the host to connect on
     * @return @c SocketHardwareController, nullptr if an error occurs
     */
    static std::shared_ptr<SocketHardwareController> create(std::string host, int port);

    /**
     * Read a @c KeywordDetection from the hardware controller.
     *
     * @param timeout Timeout for the read
     * @return @c KeywordDetection when a detection occurs, otherwise @c nullptr
     * if an error occurs, or a timeout
     */
    std::unique_ptr<KeywordDetection> read(std::chrono::milliseconds timout) override;
    
    /**
     * Destructor.
     */
    ~SocketHardwareController();
private:
    /**
     * Constructor.
     *
     * @param host Host of the socket server to connect to
     * @param port Port on the host to connect on
     */
    SocketHardwareController(std::string host, int port);

    /// Initialize the connection to the socket.
    bool init();

    /// Host to connect to
    std::string m_host;

    /// Port on the host to connect to
    int m_port;

    /// File descriptor for the socket the controller is connected to 
    int m_sockFd;
};

} // kwd
} // alexaClientSDK

#endif // ALEXA_CLIENT_SDK_KWD_SOCKET_HW_CTRL_H_
