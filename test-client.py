import socket
import sys
import random
import threading

RELAY_HOST = f"127.0.0.2"
RELAY_PORT_START = 18720
SRC_PORT = None



def reverse_endianness(n : int) -> int:
    return int.from_bytes(n.to_bytes(2), "little")





def recv_loop(s : socket.socket):
    while(True):
        data = s.recvfrom(65536)
        print("Recieved: " + data[0].decode())




def test_port(port : int) -> socket.socket:
    global RELAY_HOST
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.bind((f"0.0.0.0", reverse_endianness(port)))
        sock.sendto(b"\x4d\x61\x6e\x61\x13\x37\x42\x69", (RELAY_HOST, port))
        data = sock.recvfrom(8)
        print(data)
        if data[0] != b"\x55":
            sock.close()
            return None
        return sock
    except:
        return None
def main():
    cur_port = RELAY_PORT_START
    while True:
        sock = test_port(cur_port)
        if sock == None:
            print(f"{cur_port} Is invalid")
            cur_port += 1
            continue
        break

    #now send/recv fr
    threading.Thread(target=recv_loop, args=(sock,),daemon=True).start()
    while True:
        sock.sendto(input("payload: ").encode(), (RELAY_HOST, int(input("port: ")))) 



if __name__ == "__main__":
    main()
