import socket
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
port = int(sys.argv[1])
# to check the port number/
if port < 1 or port > 65535:
    exit('Illegal port number')
s.bind(('', port))

# here we are processing the data
def process_data(init_data, address, group):
    l = init_data.split(' ', 1)
    command_number = l[0]
    # if the command_number is different of 4,5 we need to get data too so:
    if not ((l[0] == '4') or (l[0] == '5')):
        try:
            data = l[1]
        except:
            return 'Illegal request'
    # if the command number is illegal:
    if command_number > '5' or command_number < '1':
        return 'Illegal request'
    # if the client isn't in the group yet:
    if (address not in group.members) and command_number > '1':
        return 'Illegal request, you are not in group yet'
    # if the client wants to join after he has already joined.
    if command_number == '1' and address in group.members:
        return 'Illegal request, you are already in the group.\n' \
               'you can try to change your name with \'3\''

    if command_number == '1':
        return group.add_member(Member(data, address))

    if command_number == '2':
        member = group.members[address]
        group.message_to_all(address, f"{member.name}: {data}")
        str = ''
        if bool(member.messages):
            str = '\n'.join(member.messages)
        member.messages = []
        return str

    if command_number == '3':
        member = group.members[address]
        old_name = member.name
        member.name = data
        group.message_to_all(address, f"{old_name} changed his name to {data}")
        str = ''
        if bool(member.messages):
            str = '\n'.join(member.messages)
        member.messages = []
        return str

    if command_number == '4':
        member = group.members[address]
        group.message_to_all(address, f"{member.name} has left the group")
        group.members.pop(address)
        return ''

    if command_number == '5':
        member = group.members[address]
        str = ''
        if bool(member.messages):
            str = '\n'.join(member.messages)
        member.messages = []
        return str

class Member:
    def __init__(self, name, address, group = None):
        self.name = name
        self.address = address
        self.messages = []
        self.group = group

    def set_name(self, name):
        self.name = name

    def add_message(self, str):
        self.messages.append(str)

    def set_group(self, group):
        self.group = group

class Group:
    def __init__(self):
        self.members = {}

    def add_member(self, member):
        if member.address not in self.members:
            x = ''
            if bool(self.members):
                # we need to return the names from the last to the first so:
                l = [v.name for v in self.members.values()]
                l.reverse()
                x = str(", ".join(l))
            self.members[member.address] = member
            member.set_group(self)
            self.message_to_all(member.address, f"{member.name} has joined")
            return x

    def message_to_all(self, address, message):
        for key in self.members:
            # we need to un send the message to the member who sent it:
            if key != address:
                self.members[key].add_message(message)

group = Group()

while True:
    data, addr = s.recvfrom(1024)
    string = str(process_data(data.decode(), addr, group))
    s.sendto(string.encode(), addr)

