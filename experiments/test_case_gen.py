import os

_KB_ = 1024
_MB_ = 1024*1024
_GB_ = 1024*1024*1024

def get_gen_func(i):
    if i == 1:
        return _tc_one_
    else:
        pass
    return

def _tc_one_(root_dir):
    # Single small file in directory
    with open(os.path.join(root_dir, "one.txt"), "wb") as f:
        f.write(os.urandom(1*_MB_));
