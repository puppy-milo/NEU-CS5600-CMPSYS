from test_vectors import queue_test_vectors as tvs
from interfaces.i_run_test import Runner
import os
import sys


class Project2Runner(Runner):
    def __init__(self, executable="./queue"):
        self.executable = executable

    def _run_queue(self, test_vectors, valgrind=False):
        fd, fname = self._get_tmpfile_handle()
        errors = 0
        for tv in test_vectors:
            inp = tv['in']
            out = tv['out']
            with open(fname, 'w') as fp:
                fp.write(inp)

            if valgrind:
                output = self._get_valgrind_for_args([self.executable, fname])
                print(output)
                continue

            output = self._get_output_for_args([self.executable, fname])
            if output != out:
                errors += 1
                print(f"ERROR:\n\tInput: <{inp.splitlines()}>\n\tExpected: <{out}>\n\tGot: <{output}>")
        os.unlink(fname)
        return errors

    def empty_enqueue_test(self, run_valgrind=False):
        return self._run_queue(tvs.empty_enqueue_test_vector, valgrind=run_valgrind)

    def large_enqueue_op_test(self, run_valgrind=False):
        return self._run_queue(tvs.large_enqueue_op, valgrind=run_valgrind)

    def regular_pass_test(self, run_valgrind=False):
        return self._run_queue(tvs.regular_pass_test_vector, valgrind=run_valgrind)

    def regular_print_test(self, run_valgrind=False):
        return self._run_queue(tvs.print_list_pass_test_vector, valgrind=run_valgrind)

    def run_all_tests(self):
        testcases = [self.empty_enqueue_test, self.large_enqueue_op_test, self.regular_pass_test,
                     self.regular_print_test]
        self._run_all_tests(testcases)

    def run_valgrind(self):
        testcases = [self.empty_enqueue_test, self.large_enqueue_op_test, self.regular_pass_test,
                     self.regular_print_test]
        for testcase in testcases:
            testcase(run_valgrind=True)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        executable_path = sys.argv[1]
        p = Project2Runner(executable=executable_path)
    else:
        print("[i] Trying to use default path `./queue`")
        p = Project2Runner()

    # p.run_valgrind()
    p.run_all_tests()
