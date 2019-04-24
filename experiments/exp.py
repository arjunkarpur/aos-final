import os
import time
import shutil
import datetime
from collections import namedtuple

from test_case_gen import get_gen_func

# Globals
RUN_NAME = "test1"
EXP_SKIP_LIST = []
N_EXPERIMENTS = 5
CLEAN_SRC = True
VERBOSE = False

PROJ_DIR = os.path.join("..")
FS_DIR = os.path.join(PROJ_DIR, "experiments", "fs")
LOG_DIR = os.path.join(PROJ_DIR, "experiments", "logs")

TestCase = namedtuple("TestCase", "case_id gen_func")

##############################################
# Helpers

def log_print(s):
    print("[%s]\t %s" % (datetime.datetime.now(), s))

def create_test_cases():
    # Populate with test case objects
    test_cases = []
    for i in range(1, 2):
        if i in EXP_SKIP_LIST:
            continue
        test_cases.append(TestCase(i, get_gen_func(i)))
    return test_cases

def time_command(cmd):
    start = time.time()
    os.system(cmd)
    end = time.time()
    return end-start

def experiment_main(test_case):
    log_print("[TEST CASE ID: %i]" % test_case.case_id)

    # Create file system according to test case
    log_print("\tCreating src directory tree")
    src_dir = os.path.join(FS_DIR, "%s_src_%i" % (RUN_NAME, test_case.case_id))
    dest_dir = os.path.join(FS_DIR, "%s_dest_%i" % (RUN_NAME, test_case.case_id))
    os.makedirs(src_dir)
    test_case.gen_func(src_dir)

    base_times = []
    our_times = []
    for _ in range(N_EXPERIMENTS):
        # Cp commands for both
        base_bin = "cp"
        ours_bin = os.path.join(os.path.join(PROJ_DIR, "coreutils-8.31", "src", "cp"))
        cp_args = "-r %s %s" % (src_dir, dest_dir)
        base_cmd = "%s %s" % (base_bin, cp_args)
        our_cmd = "%s %s" % (ours_bin, cp_args)

        # Run cp commands
        t_base = time_command(base_cmd)
        shutil.rmtree(dest_dir)
        t_ours = time_command(our_cmd)
        shutil.rmtree(dest_dir)

        # Record results
        base_times.append(t_base)
        our_times.append(t_ours)
        if VERBOSE:
            log_print("\t    Baseline %i/%i - %f" % (_+1, N_EXPERIMENTS, t_base))
            log_print("\t        Ours %i/%i - %f" % (_+1, N_EXPERIMENTS, t_ours))
    log_print("\t    Baseline Avg - %f" % (sum(base_times)/float(N_EXPERIMENTS)))
    log_print("\t        Ours Avg - %f" % (sum(our_times)/float(N_EXPERIMENTS)))

    # Clean up generated directory tree
    log_print("\tCleaning up")
    if CLEAN_SRC:
        shutil.rmtree(src_dir)
    return { "case_id": test_case.case_id, "base": base_times, "ours": our_times }

def log_results(results):
    if not os.path.isdir(LOG_DIR):
        os.makedirs(LOG_DIR)
    out_fp = os.path.join(LOG_DIR, "%s.txt" % RUN_NAME)
    f = open(out_fp, 'w')
    for res in results:
        f.write("%s\n" % res)
    f.close()

##############################################
# Main

def main():
    # Create all test cases
    test_cases = create_test_cases();
    log_print("%i Test Cases" % len(test_cases))

    # Run experiment loop on each test case
    log_print("Starting experiments...")
    results = []
    for tcase in test_cases:
        tc_results = experiment_main(tcase)
        results.append(tc_results)

    # Report results
    log_print("Writing results to file...")
    log_results(results)
    log_print("DONE!")

if __name__ == "__main__":
    main()
