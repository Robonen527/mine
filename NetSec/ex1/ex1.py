from Cryptodome.Cipher import DES
from Cryptodome.Util.Padding import pad, unpad
import sys


def printHex(bytes):
    bytesHex = bytes.hex()
    for i in range(0, len(bytesHex), 2):
        print("0x" + bytesHex[i:i + 2])


def xor(x, y, z):
    return x ^ y ^ z


def oracle(ciphertext, key, iv):
    try:
        cipher = DES.new(key, DES.MODE_CBC, iv)
        decrypted = cipher.decrypt(ciphertext)
        unpad(decrypted, 8)
        return True
    except:
        return False


def findBlock(previousBlock, currentBlock):
    c = b'\x00' * 8 + currentBlock
    answer = [0, 0, 0, 0, 0, 0, 0, 0]
    for i in range(8):
        while True:
            current = c[7 - i]
            newCurrent = (current + 1) % 256
            c = c[:7 - i] + bytes([newCurrent]) + c[8 - i:]
            result = oracle(c, key, IV)
            if result:
                break
        pi = xor(1 + i, previousBlock[7 - i], c[7 - i])
        for j in range(i + 1):
            c = c[:7 - i + j] + bytes([xor(c[7 - i + j], i + 2, i + 1)]) + c[8 - i + j:]
        answer[7 - i] = pi
    return bytes(answer)


encrypted = bytes.fromhex(sys.argv[1])
key = sys.argv[2].encode()
IV = bytes.fromhex(sys.argv[3])
blockSize = 8
BlocksLen = len(encrypted) // blockSize
blocks = [IV]
whole = b''
for i in range(BlocksLen):
    blocks.append(encrypted[i * 8:(i + 1) * 8])

for i in range(BlocksLen):
    whole = whole + findBlock(blocks[i], blocks[i + 1])
print(unpad(whole, DES.block_size).decode('utf-8'))
