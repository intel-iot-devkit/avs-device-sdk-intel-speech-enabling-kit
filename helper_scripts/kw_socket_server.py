"""Small Python socket server for triggering the socket based hardware keyword
detector.
"""
import argparse
import socket
import sys


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('keyword', type=str, help='Keyword to send')
    parser.add_argument('--port', type=int, default=3000, 
            help='Socket server port')
    return parser.parse_args()


def init_server(port):
    """Create the socket server and return the sock.
    """
    s = None
    for res in socket.getaddrinfo(None, port, socket.AF_UNSPEC,
                                  socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
        af, socktype, proto, canonname, sa = res
        try:
            s = socket.socket(af, socktype, proto)
        except socket.error as msg:
            s = None
            continue
        try:
            s.bind(sa)
            s.listen(1)
        except socket.error as msg:
            s.close()
            s = None
            continue
        break
    return s


def accept(s, port):
    """Simple wrapper around socket.accept(). Returns the connection and address.
    """
    print("Listening on 127.0.0.1:{}".format(port))
    conn, addr = s.accept()
    print("Received connection from {0}:{1}".format(*addr))
    return conn, addr


def main():
    """Main method
    """
    args = parse_args()
    
    s = init_server(args.port)
    conn = None
    payload = bytearray(args.keyword, 'utf-8')

    if s is None:
        print('-- ERROR: Could not open the socket')
        return -1

    try:
        print('Press CTRL-C to stop.')

        while True:
            conn, addr = accept(s, args.port)

            try: 
                while True:
                    print('Press ENTER to send the keyword')
                    input()
                    conn.send(payload)
                    print('-- INFO: Sent')
            except socket.error:
                print('-- INFO: Lost connection with {0}:{1}'.format(*addr))
                conn.close()
                conn = None
    except KeyboardInterrupt:
        print('-- INFO: Quitting')
        if conn is not None:
            conn.close()
        s.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())


