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

def TestCase(id):
    def decorator(f):
        _gen_funcs[id] = f
    return decorator

def create_rand_file(fp, size):
    with open(fp, "wb") as f:
        f.write(os.urandom(size))

def log_print(s):
    print("[%s]\t\t\t%s" % (datetime.datetime.now(), s))

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

@TestCase(1)
def small_files_2p0(root_dir):
    log_print("2^0 10KB file")
    for i in range(int(2**0)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(2)
def small_files_2p1(root_dir):
    log_print("2^1 10KB file")
    for i in range(int(2**1)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(3)
def small_files_2p2(root_dir):
    log_print("2^2 10KB file")
    for i in range(int(2**2)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(4)
def small_files_2p3(root_dir):
    log_print("2^3 10KB file")
    for i in range(int(2**3)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(5)
def small_files_2p4(root_dir):
    log_print("2^4 10KB file")
    for i in range(int(2**4)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(6)
def small_files_2p5(root_dir):
    log_print("2^5 10KB file")
    for i in range(int(2**5)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(7)
def small_files_2p6(root_dir):
    log_print("2^6 10KB file")
    for i in range(int(2**6)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(8)
def small_files_2p7(root_dir):
    log_print("2^7 10KB file")
    for i in range(int(2**7)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(9)
def small_files_2p8(root_dir):
    log_print("2^8 10KB file")
    for i in range(int(2**8)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(10)
def small_files_2p9(root_dir):
    log_print("2^9 10KB file")
    for i in range(int(2**9)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(11)
def small_files_2p10(root_dir):
    log_print("2^10 10KB file")
    for i in range(int(2**10)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(12)
def small_files_2p11(root_dir):
    log_print("2^11 10KB file")
    for i in range(int(2**11)):
        create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

# @TestCase(13)
# def small_files_2p12(root_dir):
#     log_print("2^12 10KB file")
#     for i in range(int(2**12)):
#         create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

# @TestCase(14)
# def small_files_2p13(root_dir):
#     log_print("2^13 10KB file")
#     for i in range(int(2**13)):
#         create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

# @TestCase(15)
# def small_files_2p14(root_dir):
#     log_print("2^14 10KB file")
#     for i in range(int(2**14)):
#         create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

# @TestCase(16)
# def small_files_2p15(root_dir):
#     log_print("2^15 10KB file")
#     for i in range(int(2**15)):
#         create_rand_file(os.path.join(root_dir, "{}.txt".format(i)), 10*_KB_)

@TestCase(17)
def short_balanced_tree(root_dir):
    # Balanced tree of height 3
    # Each non-leaf has 3 files and 1 dir
    log_print("Balanced tree of height 3")
    create_mary_tree(root_dir, 3, 3, 3)

@TestCase(18)
def tall_balanced_tree(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 3 files and 2 dir
    log_print("Balanced tree of height 7, 3 files")
    create_mary_tree(root_dir, 7, 3, 2)

@TestCase(19)
def tall_balanced_tree_more_files(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 10 files and 2 dir
    log_print("Balanced tree of height 7, 10 files")
    create_mary_tree(root_dir, 7, 10, 2)

# @TestCase(20)
# def tall_balanced_tree_even_more_files(root_dir):
#     # Balanced tree of height 7
#     # Each non-leaf has 10 files and 2 dir
#     # Warning: this is a phat test case, will take a couple minutes to run
#     log_print("Balanced tree of height 7, 20 files")
#     create_mary_tree(root_dir, 7, 20, 2)

@TestCase(21)
def tall_imbalanced_tree_more_files(root_dir):
    # Balanced tree of height 7
    # Each non-leaf has 10 files and 2 dir
    log_print("Imbalanced tree of height 5 and 2, 10 files")
    create_mary_tree(root_dir, 5, 10, 2)
    fp = os.path.join(root_dir, "poop")
    os.mkdir(fp)
    create_mary_tree(fp, 2, 10, 2)
