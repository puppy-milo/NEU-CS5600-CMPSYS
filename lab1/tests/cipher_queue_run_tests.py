import sys

from queue_run_tests import Project2Runner
from test_vectors import cipher_queue_test_vectors as tvs


class Project3Runner(Project2Runner):
    def __init__(self, executable="./cipher_queue"):
        super().__init__(executable)

    def empty_enqueue_test(self, run_valgrind=False):
        return self._run_queue(tvs.empty_enqueue_test_vector, valgrind=run_valgrind)

    def large_enqueue_op_test(self, run_valgrind=False):
        return self._run_queue(tvs.large_enqueue_op, valgrind=run_valgrind)

    def regular_pass_test(self, run_valgrind=False):
        return self._run_queue(tvs.regular_pass_test_vector, valgrind=run_valgrind)

    def run_all_tests(self):
        testcases = [self.empty_enqueue_test, self.large_enqueue_op_test, self.regular_pass_test]
        self._run_all_tests(testcases)

    def run_valgrind(self):
        testcases = [self.empty_enqueue_test, self.large_enqueue_op_test, self.regular_pass_test]
        for testcase in testcases:
            testcase(run_valgrind=True)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        executable_path = sys.argv[1]
        p = Project3Runner(executable=executable_path)
    else:
        print("[i] Trying to use default path `./cipher_queue`")
        p = Project3Runner()

    # p.run_valgrind()
    p.run_all_tests()
