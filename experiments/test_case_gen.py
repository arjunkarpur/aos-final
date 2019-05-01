import os

_KB_ = 1024
_MB_ = 1024*1024
_GB_ = 1024*1024*1024

####################################
# Helpers

def get_gen_funcs():
    gen_funcs = \
    {
        1: _tc_one_,
        2: _tc_two_,
        3: _tc_three_,
        4: _tc_four_,
    }
    return gen_funcs

def create_rand_file(fp, size):
    with open(fp, "wb") as f:
        f.write(os.urandom(size))

####################################
# Test Cases

def _tc_one_(root_dir):
    # Single small file in root dir
    create_rand_file(os.path.join(root_dir, "one.txt"), 10*_KB_)

def _tc_two_(root_dir):
    # 10 small files in root dir
    for _ in range(10):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 10*_KB_)

def _tc_three_(root_dir):
    # 10 large files in root dir
    for _ in range(10):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 25*_MB_)

def _tc_four_(root_dir):
    # 500 small files in root dir
    for _ in range(500):
        create_rand_file(os.path.join(root_dir, "%i.txt" % _), 10*_KB_)
