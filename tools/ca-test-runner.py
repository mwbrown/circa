#!/usr/bin/env python

import os,subprocess,sys
from glob import glob

class OutputDifference(object):
    def __init__(self, fromCommand, fromFile, lineNumber):
        self.fromCommand = fromCommand
        self.fromFile = fromFile
        self.lineNumber = lineNumber

class TestFailure(object):
    def __init__(self, description):
        self.description = description

def read_text_file(filename):
    f = open(filename)
    contents = f.read()
    return contents

def diff_command_against_file(command, filename):
    """
    Run the command 'command' as a separate process, and read from stdin. Also,
    open the file 'file'. Compare the results of 'command' against the contents
    of 'file', and assert that they are the same.

    If the results are the same, we print None. If there is a difference, we return
    an instance of OutputDifference.
    """
    expectedOutput = read_text_file(filename)

    proc = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, close_fds=True)
    (stdin, stdout) = (proc.stdin, proc.stdout)

    numLines = 0
    for line in expectedOutput.split('\n'):
        # Read a line from stdout and check it against expected
        stdout_line = stdout.readline()
        stdout_line = stdout_line[:-1] # remove trailing newline

        if line != stdout_line:
            return OutputDifference(stdout_line, line, numLines)

        numLines += 1

    return None

def test_file(filename):
    failures = []

    # Diff test
    diff = diff_command_against_file("circa_t "+filename, filename + ".output")
    if diff:
        desc = ['Script output differed on line '+str(diff.lineNumber)]
        desc.append('  Expected: '+diff.fromFile)
        desc.append('  Observed: '+diff.fromCommand)
        failures.append(TestFailure(desc))

    # Source repro test
    diff = diff_command_against_file("circa_t -s "+filename, filename)
    if diff:
        desc = ['Source repro failed on line '+str(diff.lineNumber)]
        desc.append(' Expected: '+diff.fromFile)
        desc.append(' Observed: '+diff.fromCommand)
        failures.append(TestFailure(desc))

    return failures

EnabledTests = ['if.ca', 'for.ca']

def run_all_tests():
    if 'CIRCA_HOME' in os.environ:
        os.chdir(os.environ['CIRCA_HOME'])

    for file in ['src/ca/tests/'+f for f in EnabledTests]:
        print "Testing "+file
        failures = test_file(file)
        if failures:
            print str(len(failures)) + " failure(s) in "+file+":"
        for failure in failures:
            for line in failure.description:
                print " "+line

if __name__ == '__main__':
    run_all_tests()
