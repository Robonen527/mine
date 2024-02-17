import socket
import sys

port = int(sys.argv[2])
# to check the port number.
if port < 1 or port > 65535:
    exit('Illegal port number')

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    input1 = input();
    # the command_number and the data.
    l = input1.split(' ', 1)
    num = l[0]

    s.sendto(input1.encode(), (sys.argv[1], port))
    data, addr = s.recvfrom(1024)
    data = data.decode()
    if data != '':
        print(data)
    # if the client wants to leave the group.
    if num == '4':
        break
s.close()
