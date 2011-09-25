#!/usr/bin/env python

import re, sys
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

def get_included_file(line):
    m = IncludedFilePattern.match(line)
    if m: return m.group(1)
    m = IncludedFilePattern2.match(line)
    if m: return m.group(1)

def map_includes(file):
    if file == 'bytecode.h': return '../bytecode.h'
    if file == 'importing_macros.h': return '../importing_macros.h'
    if file == 'circa.h': return None
    return file

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

            if not file and line:
                self.lines.insert(n, "")

            if not file:
                break

            self.includedFiles.append(file)
            n += 1

        self.includedFiles = map(map_includes, self.includedFiles)
        self.includedFiles = filter(lambda l: l is not None, self.includedFiles)

        self.includedFiles.append("../importing.h")
        self.includedFiles.append("../importing_macros.h")

        self.includedFiles = list(set(self.includedFiles))
        self.includedFiles.sort()

        includeLines = ['#include "' + s + '"' for s in self.includedFiles]
        includeLines.insert(0, '#include "../common_headers.h"')
        includeLines.insert(1, "")

        self.lines = self.lines[0:2] + includeLines + self.lines[n:]

        #print "["+self.filename+"] includes: " + str(self.includedFiles)

    def write(self):
        f = open(self.filename, 'w')

        for l in self.lines:
            f.write(l + "\n")


files = map(File, sys.argv[1:])

for f in files:
    f.write()

# for x in <generator>
# maybe any expression can be treated as a generator?
