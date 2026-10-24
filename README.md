# minishell
A simple Unix-style Shell implemented in C++.

## Build and run

```
make
./myshell
```


## Features

| Feature | Example |
|---|---|
| Run programs with arguments | `ls -al` |
| Pipelines | `ls -l \| grep cc \| wc -l` |
| Redirect stdout (overwrite) | `ls > out.txt` |
| Redirect stdout (append) | `ls >> out.txt` |
| Redirect stdin | `cat < in.txt` |
| Redirect stderr | `ls /nonexistent 2> err.txt` |
| Redirect stdout and stderr, appended | `ls /nonexistent . >>& all.txt` |
| Background jobs | `sleep 5 &` |
| Built-in `cd` (no argument goes to `$HOME`) | `cd /tmp` |
| Built-in `exit` | `exit` |

In a pipeline, `<` applies to the first command, and `>`, `>>` and `>>&` apply to the last. `2>` applies to every command.

The shell ignores Ctrl+C. Child process terminations are logged to `logs.txt` (timestamp, pid, exit status), and finished background jobs are reaped through a `SIGCHLD` handler.

Before each command runs, the shell prints the parsed command table.

## How it works

```
input line -> tokenize() -> parse() -> command table -> execute()
```

| File | Role |
|---|---|
| `tokenizer.cc` / `tokenizer.h` | Splits a line into typed tokens |
| `command.h` | `SimpleCommand` and `Command` data structures |
| `command.cc` | `parse()` fills the command table; `execute()` builds the pipes and redirects and runs it; built-ins, logging and `main` |
