import subprocess
import tempfile


class Runner:
    @staticmethod
    def __pipes_out_to_string(output):
        out_str = ""
        for pipe in output:
            if pipe is not None:
                out_str += pipe.decode().replace('\n', '').replace('\t', '')
        return out_str

    @staticmethod
    def _get_valgrind_for_args(args):
        valgrind_args = "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes".split()
        nargs = valgrind_args + args
        return Runner._get_output_for_args(nargs)

    @staticmethod
    def _get_output_for_args(args):
        output = subprocess.Popen(args, stdout=subprocess.PIPE).communicate()
        return Runner.__pipes_out_to_string(output)

    @staticmethod
    def _get_tmpfile_handle():
        # Caller has to cleanup by calling os.remove(file_name)
        fd, fn = tempfile.mkstemp()
        return fd, fn

    @staticmethod
    def _get_test_case_name(testcase):
        return testcase.split(' ')[2]

    @staticmethod
    def _run_all_tests(testcases):
        failures = []
        passes = []
        for testcase in testcases:
            print(f"[RUN] {Runner._get_test_case_name(str(testcase))}")
            error = testcase()
            if error:
                failures.append(Runner._get_test_case_name(str(testcase)))
                print(f"[FAIL] {Runner._get_test_case_name(str(testcase))}")
            else:
                passes.append(Runner._get_test_case_name(str(testcase)))
                print(f"[OK] {Runner._get_test_case_name(str(testcase))}")

        if len(failures) == 0:
            print("All test cases passed!!!")
        else:
            print(f"[FAIL] {len(failures)}/{len(testcases)}")
            for f in failures:
                print(f"[FAIL] {f}")
            for f in passes:
                print(f"[PASS] {f}")
