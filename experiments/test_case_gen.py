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

def create_mary_tree(root_dir, height, num_files, num_dirs, file_size=10*_KB_):
    for i in range(num_files):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), file_size)
    if height == 1:
        return
    for i in range(num_dirs):
        fp = os.path.join(root_dir, "dir_{}".format(i))
        os.mkdir(fp)
        create_mary_tree(fp, height - 1, num_files, num_dirs, file_size)

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

@TestCase
def short_balanced_tree(root_dir):
    # Balanced tree of height 3
    # Each non-leaf has 3 files and 1 dir
    log_print("Balanced tree of height 3")
    create_mary_tree(root_dir, 3, 3, 3)

@TestCase
def tall_balanced_tree(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 3 files and 2 dir
    log_print("Balanced tree of height 7, 3 files")
    create_mary_tree(root_dir, 7, 3, 2)

@TestCase
def tall_balanced_tree_more_files(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 10 files and 2 dir
    log_print("Balanced tree of height 7, 10 files")
    create_mary_tree(root_dir, 7, 10, 2)

@TestCase
def tall_balanced_tree_even_more_files(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 10 files and 2 dir
    # Warning: this is a phat test case, will take a couple minutes to run
    log_print("Balanced tree of height 7, 20 files")
    create_mary_tree(root_dir, 7, 20, 2)

@TestCase
def tall_imbalanced_tree_more_files(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 10 files and 2 dir
    log_print("Imbalanced tree of height 5 and 2, 10 files")
    create_mary_tree(root_dir, 5, 10, 2)
    fp = os.path.join(root_dir, "poop")
    os.mkdir(fp)
    create_mary_tree(fp, 2, 10, 2)
