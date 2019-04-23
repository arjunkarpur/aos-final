
import os

file_size = 1024*1024;
with open("one.txt", "wb") as f:
    f.write(os.urandom(file_size));
