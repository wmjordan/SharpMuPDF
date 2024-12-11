#!/usr/bin/env python

"""
Rewrite version numbers in AssemblyInfo and resource
"""

import os, re
import pathlib

MAJOR = "2"
MAJOR_REV = "1"
MINOR = "25"
MINOR_REV = "2"

def rewriteRevision(m):
	return m.group(1) + MAJOR + m.group(2) + MAJOR_REV + m.group(2) + MINOR + m.group(2) + MINOR_REV + m.group(4)

def main():
	os.chdir(os.path.dirname(__file__))

	file = "AssemblyInfo.cpp"
	content = open(file).read()
	newContent = re.sub(r"(AssemblyFileVersion(?:Attribute?)\(\")\d+\.\d+\.\d+(\.)(\d+)(\"\))", rewriteRevision, content)
	if newContent != content:
		open(file, "wt").write(newContent)
		print("rewrite " + file)
	else:
		print("skipped " + file)

	file = "MuPDFLib.rc"
	content = open(file, encoding="utf-16").read()
	newContent = re.sub(r"([A-Z]+VERSION\s+)\d+,\d+,\d+(,)(\d+)(\s*?)", rewriteRevision, content)
	if newContent != content:
		newContent = re.sub(r"(VALUE\s+\"\w+Version\", \")\d+\.\d+\.\d+(\.)(\d+)(\")", rewriteRevision, newContent)
		open(file, "wt", encoding="utf-16").write(newContent)
		print("rewrite " + file)
	else:
		print("skipped " + file)

if __name__ == "__main__":
	main()
