#!/usr/bin/env python

"""
Rewrite version numbers in AssemblyInfo and resource
"""

import os, re
import pathlib

MAJOR = "2"
MAJOR_REV = "1"
MINOR = "25"

def rewriteRevision(m):
	return m.group(1) + MAJOR + m.group(2) + MAJOR_REV + m.group(2) + MINOR + m.group(2) + str(int(m.group(3))+1) + m.group(4)

def main():
	os.chdir(os.path.dirname(__file__))

	asmInfo = open("AssemblyInfo.cpp").read()
	asmInfo = re.sub(r"(AssemblyFileVersion(?:Attribute?)\(\")\d+\.\d+\.\d+(\.)(\d+)(\"\))", rewriteRevision, asmInfo)
	open("AssemblyInfo.cpp", "wt").write(asmInfo)
	print("rewrite AssemblyInfo.cpp")

	rc = open("MuPDFLib.rc", encoding="utf-16").read()
	rc = re.sub(r"([A-Z]+VERSION\s+)\d+,\d+,\d+(,)(\d+)(\s*?)", rewriteRevision, rc)
	rc = re.sub(r"(VALUE\s+\"\w+Version\", \")\d+\.\d+\.\d+(\.)(\d+)(\")", rewriteRevision, rc)
	open("MuPDFLib.rc", "wt", encoding="utf-16").write(rc)
	print("rewrite MuPDFLib.rc")

if __name__ == "__main__":
	main()
