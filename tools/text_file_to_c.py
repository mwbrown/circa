#!/usr/bin/env python

def escape_line(line):
    out = []
    for c in line:
        if c == '"': out.append('\\"')
        elif c == '\\': out.append('\\\\')
        else: out.append(c)
    return "".join(out)

def generate(sourceFile, variableName):
    source = open(sourceFile)
    out = []

    out.append("// This file was autogenerated from "+sourceFile)
    out.append("")
    out.append("namespace circa {")
    out.append("")
    out.append("extern const char* "+variableName+" = ")

    while source:
        line = source.readline()
        if line == "": break
        line = line[:-1]
        line = escape_line(line)
        out.append('    "' + line + '\\n"')

    out[-1] += ";"
    out.append("")
    out.append("} // namespace circa")
    out.append("")

    return "\n".join(out)

if __name__ == '__main__':
    import sys
    print generate(sys.argv[1], sys.argv[2])