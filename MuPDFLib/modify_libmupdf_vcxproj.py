#!/usr/bin/env python

"""
Rewrite libmupdf.vcxproj:
  1. insert reference to mupdf_load_system_font.c,
  2. add preprocessor TOFU;TOFU_CJK_EXT

Run this script before compiling libmupdf.vcxproj
"""

import os, time
import pathlib
from datetime import datetime

def main():
	os.chdir(os.path.dirname(__file__))

	now = datetime.now()

	file = "..\mupdf\platform\win32\libmupdf.vcxproj"
	mt = os.path.getmtime(file)

	content = open(file).read()
	if content.find("mupdf_load_system_font.c") < 0:
		r = "<ClCompile Include=\"..\\..\\source\\pdf\\pdf-font.c\" />";
		content = content.replace(r, r + "<ClCompile Include=\"..\\..\\..\\MuPDFLib\\Document\\mupdf_load_system_font.c\" />").replace("<PreprocessorDefinitions>", "<PreprocessorDefinitions>TOFU;TOFU_CJK_EXT;")
		open(file, "wt").write(content)
		os.utime(file, (now, mt))
		print("rewrite " + file)
	else:
		print("skipped " + file)

if __name__ == "__main__":
	main()
