import os
import sys

filename = input('enter filename to encrypt')

if not os.path.exists(filename):
    print("invalid path name")
    exit()
else:
    with open ('blah.txt', 'rb') as _file:
        data = _file.read()
        for c in data:
            print(chr(c), ':', hex(c))
