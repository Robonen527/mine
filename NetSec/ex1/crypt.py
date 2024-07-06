from Cryptodome.Cipher import DES
from Cryptodome.Util.Padding import pad, unpad

def printHex(bytes):
    bytesHex = bytes.hex()
    for i in range(0, len(bytesHex), 2):
        print("0x" + bytesHex[i:i + 2])

message = b'Hey bro whatsapp?'
padded = pad(message, 8)
key = b'mydeskew'
IV = b'\x00' * 8
cipher = DES.new(key, DES.MODE_CBC, IV)
encryptedBlocks = []
numOfBlocks = len(padded) // 8  # 2
for i in range(numOfBlocks):
    start = i * 8
    end = (i + 1) * 8
    block = padded[start:end]
    encryptedBlock = cipher.encrypt(block)
    encryptedBlocks.append(encryptedBlock)

encrypted = b''.join(encryptedBlocks)
print(encrypted.hex())
