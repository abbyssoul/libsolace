# Command Line Interface utilities

libsolace provides a framework to help to build CLI tools.
Command line interface is a way to interact with a program via command line arguments. For example:
```shell
    ./my_program --verbose file.yaml
    ./my_program --help
```

Thus command line is understood to be an array of string tokens:
```
    ["a", "bbb", "asdf, "d", "das"]
```

libsolace supports two styles of command line arguments: flags and multi-command style CLIs.

## Flags
Flags or optional argument are extra value that a user may pass to the program to alter its behaviour. For example if
program `my_tool` reads its configuration from a file it might be beneficial to give a user an option to specify a non-default
configuration, for example to use for testing and production. This can be achieved by making `my_tool` to take an extra option 'config'.
Note in this case it is assumed that a program can perform its actions using default configuration file and thus can be run without any command line options.

    ./my_tool --config test.cfg -l

By convention flags can have two distinct forms:
* A single latter flag, also known as short option. For example: `-l`
* A long name of the option. For example: `--config prod.yaml`

Usually flags are followed by a value such as in the example above:
`--config test.cfg`
in this case option config is set to have a value 'test.cfg'

Note that some flags are used to set boolean values, for example: `--force true`.

In this cases it is sufficient to specify the flag itself with out following value.

    ./my_tool --force --verbose file.txt

## Commands
Commands offer a different style of applications where a program is used to dispatch(select) and action.
Most prominent example of such stile can be git:

    git status
    git commit -am "Good commit message"
    git remote add mirror git://git@github.com

In this case `git` is a dispatcher program/cli that enables selection of the action to be taken.
Commands not always have values but instead allow for a more expressive CLIs.

## Mix and match
It is often useful to combine flags and commands to have the most flexible CLI:

    ./my_tool --config local.yaml --verbose restart-node "node-3212"

In this example we use `my_tool` with non-default config `local.yaml`, ask for verbose output and using command `restart-node` that takes node name as a mandatory argument.

# Positional arguments
Unlike other CLI frameworks libsolace does support positional arguments for commands.
The only restriction - positional arguments can not be followed by a command or flags:

    # Supported form
    ./my_tool -c some.cfg action1 ../file1 ../file2

    # Not supported format
    ./my_tool action1 ../dir/file.txt --fast


So the full example of supported syntax is:
For flags / options:
```
['program_name', 'positional-argument1', 'positional-argument2', ...]
['program_name', '--flag', '--option', 'value', 'positional-argument1', 'positional-argument2', ...]
```

With commands:
```
['program_name', '-option', 'value', '-flag', '--option2', 'value', 'command', '--comand-specific-flag', 'value', 'positional-argument1', 'positional-argument2', ...]
```