#!/usr/bin/env python

"""
Generates PDF names for name-table.h, replacing symbols invalid for .NET fields.
"""

import os, re, time
import pathlib

def replaceInvalidSymbol(m):
	return "PDF_MAKE_NAME(\""+m.group(1)+"\", _"+m.group(2)+")"

def main():
	os.chdir(os.path.join(os.path.dirname(__file__), ".."))

	src = "mupdf/include/mupdf/pdf/name-table.h"
	dest = "MuPDFLib/!Include/name-table.h"

	ms = os.path.getmtime(src)

	if os.path.isfile(dest):
		md = os.path.getmtime(dest)
		if ms == md:
			print("skipped up-to-date name-table.h")
			return;

	name_table = open(src).read()
	name_table = re.sub(r"PDF_MAKE_NAME\(\"(.+?)\", (\d.*)\)", replaceInvalidSymbol, name_table)
	name_table = re.sub(r"/[/\*].*?\r?\n", "", name_table)

	target = open(dest, "wt")
	target.write("// This file is automatically generated by sync_name_table.py from mupdf/include/mupdf/pdf/name-table.h\n")
	target.write("// Time: " + time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()))
	target.write(name_table)

	os.utime(dest, (time.time(), ms))

	print("synced name-table.h")

if __name__ == "__main__":
	main()
