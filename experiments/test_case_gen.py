import datetime
import os

_KB_ = 1024
_MB_ = 1024*1024
_GB_ = 1024*1024*1024

####################################
# Helpers

_gen_funcs = {}
def get_gen_funcs():
    return _gen_funcs

def TestCase(f):
    _gen_funcs[len(_gen_funcs) + 1] = f

def create_rand_file(fp, size):
    with open(fp, "wb") as f:
        f.write(os.urandom(size))

def log_print(s):
    print("[%s]\t\t%s" % (datetime.datetime.now(), s))

####################################
# Test Cases

@TestCase
def _tc_one_(root_dir):
    # Single small file in root dir
    log_print("Single 10KB file")
    create_rand_file(os.path.join(root_dir, "one.txt"), 10*_KB_)

@TestCase
def _tc_two_(root_dir):
    # 10 small files in root dir
    log_print("10 10KB files")
    for _ in range(10):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 10*_KB_)

@TestCase
def _tc_three_(root_dir):
    # 10 large files in root dir
    log_print("10 25MB files")
    for _ in range(10):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 25*_MB_)

@TestCase
def _tc_four_(root_dir):
    # 500 small files in root dir
    log_print("500 10KB files")
    for _ in range(500):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 10*_KB_)
