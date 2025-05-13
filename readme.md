# Sharp MuPDF

Sharp MuPDF demonstrates how to compile MuPDF source code into a dynamic link library and consume its functionality in .NET.

## Compile

To compile the source code.

1. Install _Visual Studio 2019_ or newer versions.

2. Install _python 3_ (`Python.exe` must be accessible via the PATH environment variable).

3. Open the `MuPDF.sln` in the solution folder with Visual Studio.

4. Compile the solution.

5. During compilation, Python will be called to generate the definition file for the target dll file.

6. Results:

   `MuPDFLib` project will produce two DLL files for mupdf, one for x86 and the other for x64.

   `Demo` project contains some code to demonstrate how to use functions in MuPDFLib.dll

## System font loading

In order to support loading system fonts for documents with unembedded fonts and avoid the performance lost across DLL files, a code file named `mupdf_load_system_font.c` shall be compiled with the `libmupdf` project.

We have to modify that project and reference the code file from `MuPDFLib\Document\mupdf_load_system_font.c`.

That code file is copied from project *SumatraPDF* and all credits goes to them.

## Shrinking MuPDFLib.dll

The default compilation of MuPDFLib contains a large Unicode font TOFU.

Usually we don't need it. We can exclude it by the following procedure.

Open the property page for the `libmupdf` project.

Add `;TOFU;TOFU_CJK_EXT` to _C/C++_/_Preprocessor_/_Preprocessor Definitions_ for _All Configurations_ and _All Platforms_ in configuration manager.

For more information, see `config.h` file within the `!include/fitz` folder in `libmupdf` project.

## Compilation helper scripts

There are several Python scripts in the folder of MuPDFLib. You can open and read the comment on top of the scripts to learn their usage.

Those scripts are run before compiling the `MuPDFLib` project automatically.

Especially, the `modify_vcxprojs.py` script should be run before compiling the `libmupdf` project, to make modifications about _System font loading_ and _Shrinking MuPDFLib.dll_.

## .NET assembly reference of MuPDFLib.dll

From version 2.* on, it is possible to reference MuPDFLib.dll as a .NET assembly, since it is compiled with C++/CLI.

If you redistribute the MuPDFLib.dll which is referenced as a .NET assembly in your application, your users may encounter a problem that *the MuPDFLib.dll could not be loaded*.

To fix the problem, enclose the Visual C++ Runtime library files with your redistribution. At least, `vcruntime140.dll` and `msvcp140.dll` are the minimum set of required DLL files.

## License

This project follows the license terms of *MuPDF*.

## Update source code

1. Use `git pull` command to update the repository.

2. To update source code, tags and submodules, use:

   ``` bash
   cd mupdf
   git pull origin master --recurse-submodules
   ```

   It is possible that local modifications have been made. To discard local modifications when updating submodules, use the following command before `pull`ing from master:

   ``` bash
   git reset --hard --recurse-submodules origin/master
   git reset --hard --recurse-submodules <TAG>
   ```

   To fetch remote tags, use:

   ``` bash
   git fetch origin --tags
   ```

   Afterwards, it is possible to check out the newly added tags:
   ``` bash
   git checkout <TAG>
   ```

   Alternatively, we can also execute the following commands against each changed files:

   ``` bash
   git reset HEAD <file name>
   git checkout -- <file name>
   ```

   We may see the following warning when we `pull` from master:

   ```
   error: You have not concluded your merge (MERGE_HEAD exists).
   hint: Please, commit your changes before merging.
   fatal: Exiting because of unfinished merge.
   ```

   To fix this, run the following commands:

   ``` bash
   git fetch --all
   git reset --hard origin/master
   ```

   Afterwards, we can `pull` from master then.

   Sometimes, new submodules could be added to the origin repository. Use the following command to update them into your repository.

   ``` bash
   git submodule update --init --recursive
   ```

3. The first a few lines in the `name-table.h` file have slight modifications from the original one in the `libmupdf` project, for the sake of making field names valid in .NET. Check whether it is changed and make the corresponding synchronization. To facilitate this operation, run the `sync_name_table.py` script within the `MuPDFLib` project folder.

## Git Proxy
If accessing the Internet requires HTTPS proxy, use the following command:

``` bash
git config --global http.proxy <PROXY:PORT>
git config --global https.proxy <PROXY:PORT>
```

When you are done, use the following command to reset the proxy to default:

``` bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```
