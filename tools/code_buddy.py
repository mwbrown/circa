#!/usr/bin/env python

import os, re, sys, subprocess
from glob import glob

def read_file(filename):
    f = open(filename, 'r')

    while True:
        line = f.readline()
        if not line:
            break

        yield line[:-1]


FirstLine = "// Copyright (c) Paul Hodge. See LICENSE file for license terms."
IncludedFilePattern = re.compile(r"#include \"(.*?)\"")
IncludedFilePattern2 = re.compile(r"#include <(.*?)>")
GccLocationPattern = re.compile(r"([a-z_\.]*):([0-9]+): (.*)")

def get_included_file(line):
    m = IncludedFilePattern.match(line)
    if m: return m.group(1)
    m = IncludedFilePattern2.match(line)
    if m: return m.group(1)

def map_includes(file):
    #if file == 'bytecode.h': return '../bytecode.h'
    #if file == 'importing_macros.h': return '../importing_macros.h'
    #if file == 'circa.h': return None
    return file

def monkeyFix(file, err, line):
    #print "monkey fix: "+line+", err = "+err

    if "request for member" in err and "length" in err:
        print "converted .length to ->length in "+file
        line = line.replace(".length", "->length")

    if "request for member" in err and "getFromEnd" in err:
        line = line.replace(".getFromEnd", "->getFromEnd")

    if "invalid types" in err and "array subscript" in err:
        line = line.replace("[","->get(")
        line = line.replace("]",")")

    if "Branch**" in err:
        line = line.replace("&","")

    if "no matching function" in err and "evaluate_save_locals" in err:
        line = line.replace("branch","&branch")

    if "base operand" in err:
        line = line.replace("[","->get(")
        line = line.replace("]",")")

    if "cannot convert" in err:
        line = line.replace("branch","&branch")

    return line

class File(object):
    def __init__(self, filename):
        self.filename = filename
        self.lines = list(read_file(filename))
        self.includedFiles = []

        if self.lines[0] != FirstLine:
            print "File doesn't start with the license comment: "+filename
            self.lines[0] = FirstLine

        n = 2
        while True:
            line = self.lines[n]
            file = get_included_file(line)

            if not file:
                break

            self.includedFiles.append(file)
            n += 1

        self.includedFiles = map(map_includes, self.includedFiles)
        self.includedFiles = filter(lambda l: l is not None, self.includedFiles)

        #self.includedFiles.append("../importing.h")
        #self.includedFiles.append("../importing_macros.h")

        self.includedFiles = list(set(self.includedFiles))
        self.includedFiles.sort()

        includeLines = ['#include "' + s + '"' for s in self.includedFiles]
        #includeLines.insert(0, '#include "../common_headers.h"')
        #includeLines.insert(1, "")

        # self.lines = self.lines[0:2] + includeLines + self.lines[n:]

        #print "["+self.filename+"] includes: " + str(self.includedFiles)

    def write(self):
        f = open(self.filename, 'w')

        for l in self.lines:
            f.write(l + "\n")

    def doGcc(self):
        print "doing GCC-based fixes on "+self.filename

        p = subprocess.Popen(["g++",'-c',self.filename,'-I..','-fsyntax-only'],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (output,err) = p.communicate()

        if not err:
            return

        errLines = err.split("\n")

        for line in errLines:
            #print line
            match = GccLocationPattern.match(line)
            if match:
                file = match.group(1)
                line_num = int(match.group(2)) - 1
                msg = match.group(3)
                if file == self.filename:
                    self.lines[line_num] = monkeyFix(file, msg, self.lines[line_num])
                #print "in file "+file+" on line "+line+" there was "+msg

        #print output

filenames = sys.argv[1:]

filenames = filter(os.path.isfile, filenames)

files = map(File, filenames)

for f in files:
    f.doGcc()
for f in files:
    f.write()

# for x in <generator>
# maybe any expression can be treated as a generator?
