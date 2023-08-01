# smash - Small Shell

A simple Unix shell that provides a command-line user interface for Unix-like operating systems.

## Introduction

`smash` (small shell) is a basic implementation of a Unix shell, providing a subset of shell functionalities. It behaves like a real Linux shell but supports only a limited number of Linux shell commands. It has been designed to simulate the behaviour of a typical Bash shell, allowing user interaction through the typing of commands.

## Project structure

This project contains the following files:

- `commands.cpp`: Handles the built-in command functionality.
- `commands.h`: Contains the declarations for the built-in commands.
- `signals.cpp`: Handles signal functionality.
- `signals.h`: Contains the declarations for the signal handling routines.
- `smash.cpp`: Main shell program.
- `Makefile`: Used to build the project.

## Setup & Installation

In order to setup and run the program, follow these steps:

1. Clone the repository.

```bash
git clone <repository_link>
```

2. Change to the project directory.

```bash
cd <project_directory>
```

3. Build the project using the provided Makefile.

```bash
make
```

4. Run the `smash` program.

```bash
./smash
```

## Command Usage

`smash` supports a limited number of built-in commands, as well as the execution of external commands. 

- Built-in commands: chprompt, showpid, pwd, cd, jobs, fg, bg, quit, kill, setcore, getfileinfo, chmod.
- External commands: Any command that is not a built-in command or a special command.

To use a command, type the command into the shell followed by any necessary parameters, and press enter.

### Special Commands

The shell also includes functionality for a few special commands: 

1. IO Redirection: `>` and `>>`
2. Pipes: `|` and `|&`
3. Set core for a process: `setcore <job-id> <core_num>`
4. Get file type and size: `getfileinfo <path-to-file>`
5. Change file mode: `chmod <new-mod> <path-to-file>`

### Error Handling

If an invalid command or parameters are given, `smash` will display an error message and continue to await new commands.

## Contributing

This is an academic project and contributions will not be accepted. This repository is for reference and learning purposes only.
