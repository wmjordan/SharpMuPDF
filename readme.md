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

   `Demo` project contains some code to demonstrate how to use functions in mupdflib.dll

## Shrinking MuPDF.dll

Open the property page for the `libmupdf` project.

Add `;TOFU;TOFU_CJK_EXT` to _C/C++_/_Preprocessor_/_Preprocessor Definitions_ for _All Configurations_ and _All Platforms_ in configuration manager.

So you can exclude several huge fonts from the DLL.

For more information, see `config.h` file within the `!include/fitz` folder in `libmupdf` project.

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

## License

This project follows the license terms of *MuPDF*.