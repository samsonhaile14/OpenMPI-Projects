# PA0: "Hello World"

# Dependencies, Building, and Running

## Dependency Instructions
These projects depend on the MPI Library and SLIURM Running environment preinstalled in [h1.cse.unr.edu](h1.cse.unr.edu).  There are no instructions included on how to install these suites on your personal machines.
If you are using a Windows machine, you may SSH into your respective UNR accounts through [PuTTY] (http://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html).

You may enter h1 off campus by first logging into your account through ubuntu.cse.unr.edu.  (Please note that keystrokes will be a tiny bit slower, but it's still manageable).
```bash
ssh username@ubuntu.cse.unr.edu
ssh username@h1.cse.unr.edu
```

## Building and Running
There are two options to build this project, CMake or Makefile.  CMake makes including new libraries easier, and handles new files added automatically to the src and include directory.  CMake, however, requires a small new learning curve, but it will make things easier in the long run.  Also, it is recommended to learn CMake for industry C/C++.
The second option is to use the provided Makefile, which is easier to look at and compile from.

# Quick running
You may simple run the build script with a few commands.
```bash
chmod +x buildscript.sh
./buildscript
```

Running the make in a separate directory will allow easy cleanup of the build data, and an easy way to prevent unnecessary data to be added to the git repository.  

### CMake Instructions
TBD

### Makefile Instructions
The makefile works as expected and must be updated with new files added in.

```bash
mkdir build
cd build
cp ../makefile .
make
srun -n16 ../bin/mpi_hello
```
To clean the files you've created, there is an extra target, `clean`.
Also, there is a running target that may be changed as per assignment requirements, `run`.
These are for convienience only, you may add/remove them as necessary.
```bash
make clean
make run
```
