import socket


def start_udp_server(ip, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Bind the socket to the server address and port
    server_address = (ip, port)
    sock.bind(server_address)
    print(f"UDP Server listening on {ip}:{port}")

    try:
        while True:
            print("\nWaiting to receive message...")
            data, address = sock.recvfrom(4096)  # buffer size is 4096 bytes

            print(f"Received {len(data)} bytes from {address}")
            print(f"Data: {data.decode()}")

    except KeyboardInterrupt:
        pass
    finally:
        sock.close()


if __name__ == "__main__":
    IP_ADDRESS = "192.168.1.229"
    PORT = 3333
    start_udp_server(IP_ADDRESS, PORT)
