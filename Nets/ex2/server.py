import socket

# Creating socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', 12345))
sock.listen(55)
buffer = ""

# This func gets a path of file and returns the data of the file.
def GetFile(path):
    try:
        if ".jpg" in path or ".ico" in path:
            f = open(path, "rb")
        else:
            f = open(path, "r")
    except:
        return None
    t = f.read()
    return t

# This func gets string of massage and processes it.
def process_data(string):
    try:
        l = string.split(' ', 2)
    except:
        return 'error', False

    # initialization of techniques members.
    conn = False
    string_conn = 'close\n'
    typo = ' 200 OK\n'
    redirect = False
    is_file = True

    # if the first word isn't GET there is an error.
    if l[0] != 'GET':
        return 'error', False
    # other = continue of the massage.
    other = (l[2].split('\n', 1))[1]
    x = other.find('Connection: ')
    if x != -1:
        x += 12
        # to find the type of the connection.
        other = other[x:].split('\r\n', 1)
        other = other[0]
        if other == 'keep-alive':
            conn = True
    if l[1] == '/':
        l[1] = '/index.html'
    if l[1] == '/redirect':
        redirect = True
        conn = False
        is_file = False
        typo = ' 301 Moved Permanently\n'
    path = "files" + l[1]
    # if redirect we don't need it to be a file.
    if not redirect:
        # to test the file.
        try:
            f = open(path, 'r')
            f.close()
        except:
            typo = ' 404 Not Found\n'
            is_file = False
            conn = False
    if conn:
        string_conn = 'keep-alive\r\n'
    to_return = 'HTTP/1.1' + typo + 'Connection: ' + string_conn
    if is_file:
        dataf = GetFile(path)
        to_return += 'Content-Length: ' + str(len(dataf)) + '\n\n'
        to_return = bytes(to_return, 'utf-8')
        f.close()
        try:
            dataf = bytes(dataf, 'utf-8')
        except:
            pass
        return to_return + dataf, conn
    if redirect:
        to_return += 'Location: /result.html\n'
    return (to_return).encode(), conn

# the process for each client.
def process():
    client_sock, client_addr = sock.accept()
    connection = True
    global buffer
    while connection:
        client_sock.settimeout(1)
        while "\r\n\r\n" not in buffer:
            try:
                data = client_sock.recv(1024).decode("utf-8")
                client_sock.settimeout(None)
            except:
                connection = False
                break
            print(data)
            if data == '':
                connection = False
                break
            buffer += data

        if connection == False:
            break
        m = buffer.split("\r\n\r\n", 1)[0]
        buffer = ""
        (data, connection) = process_data(m)
        client_sock.send(data)
    client_sock.close()

while True:
    process()