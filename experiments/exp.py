from collections import namedtuple

# Globals
N_EXPERIMENTS = 5
TestCase = namedtuple("TestCase", "name")

##############################################
# Helpers

def create_test_cases():
    test_cases = []

    # Populate with test cases
    #TODO

    return test_cases

def experiment_main(test_case):
    # Create file system according to test case
    #TODO

    # Run tests for base cp -r
    #TODO

    # Run tests for our cp -r
    #TODO

    # Clean up generated directory tree
    #TODO
    return {}

def log_results(results):
    #TODO
    pass

##############################################
# Main

def main():
    # Create all test cases
    test_cases = create_test_cases();

    # Run experiment loop on each test case
    results = []
    for tcase in test_cases:
        tc_results = experiment_main(tcase)
        results.append(tc_results)

    # Report results
    log_results(results)

if __name__ == "__main__":
    main()
