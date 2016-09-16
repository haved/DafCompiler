from sys import argv
import re

print("Removing all <Unit> tags containing LLVM*.cmake")

if len(argv) < 2:
    print("Error: Expected a file to fix")
    exit()

with open(argv[1], 'r+') as f:
    text = f.read()
    text = re.sub(r'<Unit ((?!</Unit>).|\n)*?LLVM.*[.]cmake(.|\n)*</Unit>', '', text)
    f.seek(0)
    f.write(text)
    f.truncate()
