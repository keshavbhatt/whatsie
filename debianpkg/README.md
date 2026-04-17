### Steps to Compile the Debian Package (Supported for Debian 12-based distributions - Deepin 23)

1. **Compile the Program**: First, compile the program using the following instructions:  
   Clone the source code:  

   ```bash
   1. Clone source code

   git clone https://github.com/keshavbhatt/whatsie.git

   2. Enter into the source directory  

   cd whatsie/src

   3. Run qmake

   qmake

   4. Run make (with the optional -j option to specify jobs or commands to run simultaneously while building)

   make -j4

   5. Run the created whatsie executable (if the compilation finishes without errors, close the program if it works correctly to proceed to the next steps)

   ./whatsie
   ```

2. **Organize Files**: Move the "src" folder, where the compiled program resides, to the `debianpkg` directory.

3. **Move the Executable**: Move the "whatsie" file located in the "src" folder to the "data" folder.  
   **WARNING**: Do not copy the file; it must be moved. The file should no longer exist in the "src" folder, or it will cause issues when compiling the Debian package.

4. **Open the Directory**: Open a terminal inside the `debianpkg` folder to execute the following steps. The files inside the folder should look like this (subject to changes):  

   ```bash
   $ ls
   data  debian  gitattributes.txt  gitignore.txt  src
   ```

5. **New Release**: Run the following command to set the necessary parameters for the package and its version:  

   ```bash
   dch --create -D stable --package "whatsie" --newversion=4.16.0 "New release."
   ```

   **Note**: Update the package version as needed by modifying `--newversion=`.

6. **Install Build Dependencies**: Run the following command to install the necessary dependencies for compiling the Debian package:  

   ```bash
   sudo apt build-dep .
   ```

7. **Compile the Debian Package**: Execute the following command to start compiling the Debian package. If you encounter errors, please report the issue and verify that all steps were completed correctly:  

   ```bash
   dpkg-buildpackage -Zxz -rfakeroot -b
   ```

   After completing these steps, you will find the `.deb` package in the folder where you cloned the repository. By default, the folder should be named `whatsie`.
