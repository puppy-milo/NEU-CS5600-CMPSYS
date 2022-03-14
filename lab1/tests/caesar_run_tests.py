from interfaces.i_run_test import Runner
import sys
import test_vectors.caesar_test_vectors as tvs


class Project0Runner(Runner):
    def __init__(self, executable="./caesar"):
        self.executable = executable

    def __run_caesar(self, test_vector, decode=True, negative_test_case=False):
        errors = 0
        arg1 = "decode" if decode else "encode"
        for tv in test_vector:
            inp = tv["ct"] if decode else tv["pt"]
            out = tv["pt"] if decode else tv["ct"]
            output = self._get_output_for_args(([self.executable, arg1, str(tv["s"]), inp]))
            if (negative_test_case and output != "ERROR") or (not negative_test_case and output != out):
                out_ = out if not negative_test_case else "ERROR"
                print(f"ERROR: \n\tInput: <{inp.splitlines()}>\n\tKey: <{tv['s']}>\n\tExpected: <{out_}>\n\tGot: <{output}>")
                errors += 1
        return errors

    def run_valgrind(self):
        arg1 = "decode"
        for tv in tvs.alnum_pass_tv:
            inp = tv["ct"]
            output = self._get_valgrind_for_args(([self.executable, arg1, str(tv["s"]), inp]))
            print(output)

    def decode_alphabet_lower_upper(self):
        return self.__run_caesar(tvs.alphabet_lower_upper_pass_tv, decode=True)

    def decode_numbers(self):
        return self.__run_caesar(tvs.numbers_pass_tv, decode=True)

    def decode_alphanumeric(self):
        return self.__run_caesar(tvs.alnum_pass_tv, decode=True)

    def encode_alphabet_lower_upper(self):
        return self.__run_caesar(tvs.alphabet_lower_upper_pass_tv, decode=False)

    def encode_numbers(self):
        return self.__run_caesar(tvs.numbers_pass_tv, decode=False)

    def encode_alphanumeric(self):
        return self.__run_caesar(tvs.alnum_pass_tv, decode=False)


    def run_all_tests(self):
        testcases = [self.decode_alphabet_lower_upper, self.decode_alphanumeric,
                     self.decode_numbers, self.encode_alphabet_lower_upper,
                     self.encode_alphanumeric,
                     self.encode_numbers, ]
        self._run_all_tests(testcases)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        executable_path = sys.argv[1]
        p = Project0Runner(executable=executable_path)
    else:
        print("[i] Trying to use default path `./caesar`")
        p = Project0Runner()

    # Egg here: try this
    # p.run_valgrind()
    p.run_all_tests()
