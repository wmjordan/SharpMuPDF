#!/usr/bin/env python

"""
Rewrite libmupdf.vcxproj:
  1. insert reference to mupdf_load_system_font.c,
  2. add preprocessor TOFU;TOFU_CJK_EXT

Rewrite other vcxproj files to place output files into the proper directory.

Run this script before compiling libmupdf.vcxproj
"""

import os
import pathlib
import xml.etree.ElementTree as ET
import shutil

def main():
	os.chdir(os.path.dirname(__file__))
	os.chdir("..\\mupdf\\platform\\win32\\")

	modified_files = []
	default_namespace = "http://schemas.microsoft.com/developer/msbuild/2003"

	for filename in ["bin2coff.vcxproj", "libextract.vcxproj", "libharfbuzz.vcxproj", "libleptonica.vcxproj", "libluratech.vcxproj", "libmupdf.vcxproj", "libpkcs7.vcxproj", "libresources.vcxproj", "libtesseract.vcxproj", "libthirdparty.vcxproj", "libmubarcode.vcxproj", "libzxing.vcxproj", "sodochandler.vcxproj"]:
		print(f'Processing {filename}...')
		doc = ET.parse(filename)
		ET.register_namespace("", default_namespace)
		ns = {'ms': default_namespace}
		root = doc.getroot()
		any_modification = False
			
		for pg in root.findall("ms:PropertyGroup", ns):
			condition = pg.get('Condition')
			if condition is not None:
				if condition.endswith("|Win32'"):
					any_modification |= update_property_group(pg, ns, 'Win32')
				elif condition.endswith("|x64'"):
					any_modification |= update_property_group(pg, ns, 'x64')

		if filename.endswith("libmupdf.vcxproj"):
			any_modification |= modify_libmupdf(root, ns)

		if any_modification:
			# backup original file
			backup_filename = filename + '.bak'
			shutil.copy(filename, backup_filename)
			print(f'Backup created for {filename}.')
			# save modified file
			doc.write(filename, encoding='utf-8', xml_declaration=True)
			print(f'Processed {filename}.')
			modified_files.append(filename)
		else:
			print(f'No modifications needed for {filename}.')

	print(f'Files modified: {modified_files}')

def modify_preprocessor(preprocessors, preprocessor, value = None):
	pe = preprocessor + "=";
	for p in preprocessors:
		if preprocessor == p or p.startswith(pe): # skip if preprocessor already defined
			return False
	preprocessors.insert(0, preprocessor + "=" + value if value is not None else preprocessor)
	return True

def modify_libmupdf(root, ns):
	# find all ItemGroup/ClCompile
	modified = False

	# modify ItemDefinitionGroup/ClCompile/PreprocessorDefinitions elements
	preprocessor_elements = root.findall("ms:ItemDefinitionGroup/ms:ClCompile/ms:PreprocessorDefinitions", ns)

	if preprocessor_elements is not None:
		for elem in preprocessor_elements:
			preprocessors = elem.text.split(';')
			# check if contains "TOFU;TOFU_CJK_EXT;"
			modified |= modify_preprocessor(preprocessors, "TOFU")
			modified |= modify_preprocessor(preprocessors, "TOFU_CJK_EXT")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_CBZ", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_EPUB", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_FB2", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_HTML", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_JS", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_MOBI", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_OFFICE", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_OCR_OUTPUT", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_ODT_OUTPUT", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_DOCX_OUTPUT", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_TXT", "0")
			modified |= modify_preprocessor(preprocessors, "FZ_ENABLE_XPS", "0")
			if modified:
				elem.text = ';'.join(preprocessors)

	cl_compile_elements = root.findall("ms:ItemGroup/ms:ClCompile", ns)

	# check if included Include='..\..\..\MuPDFLib\Document\mupdf_load_system_font.c'
	target_include = "..\\..\\..\\MuPDFLib\\Document\\mupdf_load_system_font.c"
	if not any(elem.get('Include') == target_include for elem in cl_compile_elements):
		# get first ItemGroup element
		first_item_group = root.find("ms:ItemGroup[ms:ClCompile]", ns)
		if first_item_group is not None:
			new_cl_compile = ET.SubElement(first_item_group, "ClCompile")
			new_cl_compile.set('Include', target_include)
			modified = True
	return modified

def update_property_group(pg, ns, platform):
	"""Update PropertyGroup / OutDir or IntDir when needed."""
	modified = False
	if platform == 'Win32':
		out_dir = pg.find('ms:OutDir', ns)
		if out_dir is not None and out_dir.text != '$(Configuration)\\':
			out_dir.text = '$(Configuration)\\'
			modified = True
		elif out_dir is None:
			ET.SubElement(pg, "OutDir").text = '$(Configuration)\\'
			modified = True
		
		int_dir = pg.find('ms:IntDir', ns)
		if int_dir is not None and int_dir.text != '$(Configuration)\\$(ProjectName)\\':
			int_dir.text = '$(Configuration)\\$(ProjectName)\\'
			modified = True
	elif platform == 'x64':
		out_dir = pg.find('ms:OutDir', ns)
		if out_dir is not None and out_dir.text != '$(Platform)\\$(Configuration)\\':
			out_dir.text = '$(Platform)\\$(Configuration)\\'
			modified = True
		elif out_dir is None:
			ET.SubElement(pg, "OutDir").text = '$(Platform)\\$(Configuration)\\'
			modified = True
		
		int_dir = pg.find('ms:IntDir', ns)
		if int_dir is not None and int_dir.text != '$(Platform)\\$(Configuration)\\$(ProjectName)\\':
			int_dir.text = '$(Platform)\\$(Configuration)\\$(ProjectName)\\'
			modified = True
	return modified

if __name__ == "__main__":
	main()
