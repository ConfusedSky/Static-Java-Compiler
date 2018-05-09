import sys

if(len(sys.argv) < 2):
    print("Please enter an input file.")
    sys.exit()

name = ""
values = []

with open(sys.argv[1], "r") as f:
    content = f.readlines()
content = [x.strip() for x in content]

if(len(content) < 2):
    print("File is in incorrect format.")
    print("Must be at least 2 lines:\nOne for the name,")
    print("And one for at least one enumeration.")
    sys.exit()

name = content[0]
values = content[1:]

with open(name + ".h", "w") as f:
    f.write("extern const char* {}_NAMES[];\n".format(name.upper()))
    f.write("typedef enum {\n")
    for value in values:
        f.write("    {}_{},\n".format(name.upper(), value))
    f.seek(-2, 1)
    f.truncate()
    f.write("\n{} {};".format("}", name.upper()))

with open(name + ".c", "w") as f:
    f.write("const char* {}_NAMES[] = {{\n".format(name.upper()))
    for value in values:
        f.write("    \"{}\",\n".format(value))
    f.seek(-2, 1)
    f.truncate()
    f.write("\n};")
