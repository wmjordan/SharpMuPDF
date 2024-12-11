#!/usr/bin/env python

"""
Rewrite version numbers in AssemblyInfo and resource
"""

import os, re, time
import pathlib
from datetime import datetime

MAJOR = "2"
MAJOR_REV = "1"
MINOR = "25"

def rewriteRevision(m):
	return m.group(1) + MAJOR + m.group(2) + MAJOR_REV + m.group(2) + MINOR + m.group(2) + str(int(m.group(3))+1) + m.group(4)

def main():
	os.chdir(os.path.dirname(__file__))

	now = datetime.now()

	file = "AssemblyInfo.cpp"
	mt = os.path.getmtime(file)

	if (now - datetime.fromtimestamp(mt)).days > 1:
		asmInfo = open(file).read()
		asmInfo = re.sub(r"(AssemblyFileVersion(?:Attribute?)\(\")\d+\.\d+\.\d+(\.)(\d+)(\"\))", rewriteRevision, asmInfo)
		open(file, "wt").write(asmInfo)
		os.utime(file, (now, mt))
		print("rewrite " + file)
	else:
		print("skipped " + file)

	file = "MuPDFLib.rc"
	mt = os.path.getmtime(file)
	if (now - datetime.fromtimestamp(mt)).days > 1:
		rc = open(file, encoding="utf-16").read()
		rc = re.sub(r"([A-Z]+VERSION\s+)\d+,\d+,\d+(,)(\d+)(\s*?)", rewriteRevision, rc)
		rc = re.sub(r"(VALUE\s+\"\w+Version\", \")\d+\.\d+\.\d+(\.)(\d+)(\")", rewriteRevision, rc)
		open(file, "wt", encoding="utf-16").write(rc)
		os.utime(file, (now, mt))
		print("rewrite " + file)
	else:
		print("skipped " + file)

if __name__ == "__main__":
	main()
