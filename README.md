# e

This is a fork of [e](https://github.com/hellerve/e). The goal is to simplify
even more the editor. Only basic features should be supported by the core
editor. It should be possible to implemented the rest as a lua script.

The origina e is a braindead editor. Inspired by
[kilo](https://github.com/antirez/kilo), of course, and a large swath of the
code is similar.

It can be scripted through Lua.

[![asciicast](https://asciinema.org/a/e164s5tnu3okht44go6uhyju4.png)](https://asciinema.org/a/e164s5tnu3okht44go6uhyju4)

## Features

- Scripting through intuitive Lua interface
- Incremental search (and replace)
- Limitless Undo (until memory runs out)
- Extensible syntax highlighting
- No global state in the library part (just in `main.c`)
- Can be used as a library
- Ships with syntax highlighting for
  - C/C++ (stable)
  - Python (experimental)
  - JavaScript (experimental)
  - Go (experimental)
  - Haskell (experimental)
  - Carp (experimental)
  - Markdown (unfinished)

## Installation

```sh
git clone https://github.com/hellerve/e
cd e
make install # install_lua for Lua support
```

## Usage

Without any customization, `e` let you to move around with arrow keys and
insert text with the letter keys. Backspace and delete keys remove charactes
from the text.

Hitting `Ctrl-p` a command prompt will be shown. The following command are
avaiable:

- `c` or `copy`: *c*opy a line to the system clipboard (only on Windows and OS X)
- `v` or `paste`: *v*iew (i.e. paste) the contents of the system clipboard (only on Windows and OS X)
- `/` or `search`: incremental highlighted search
- `r` or `replace`: search and replace first occurrence
- `R` or `replaceall`: search and replace all occurrences
- `h` or `cutline`: *h*ide a line (delete it) and add it to the system clipboard (clipboard only on Windows and OS X)
- `u` or `undo`: undo last change
- `l` or `lua`: eval lua code
- `s` or `save`: save and quit (might be prompted for a file name)
- `q` or `quit`: exit (will abort if the file has unsaved content)
- `!`: force exit
- Number `n`: jump to line `n`

### Writing syntax files

By default, `e` creates a directory called `.estx` in the user's home
directory (the location is overridable by providing `STXDIR` to `make install`).
There, `e` will search for syntax files on startup. Their grammar is very
minimal, see the C file below:

```
displayname: c
extensions: .*\.cpp$
            .*\.hpp$
            .*\.c$
            .*\.h$
comment|no_sep: //.*$
keyword: (restrict|switch|if|while|for|break|continue|return|else|try|catch|else|struct|union|class|typedef|static|enum|case|asm|default|delete|do|explicit|export|extern|inline|namespace|new|public|private|protected|sizeof|template|this|typedef|typeid|typename|using|virtual|friend|goto)
type: (auto|bool|char|const|double|float|inline|int|mutable|register|short|unsigned|volatile|void|int8_t|int16_t|int32_t|int64_t|uint8_t|uint16_t|uint32_t|uint64_t|size_t|ssize_t|time_t)
comment|no_sep: /\*.*\*/
comment|no_sep: /\*.*
                 .*\*/
pragma: \s*#(include|pragma|define|undef) .*$
predefined: (NULL|stdout|stderr)
pragma: \s*#(ifdef|ifndef|if) .*$
pragma: \s*#(endif)
string|no_sep: "([^\\\"]|\\.)*"
string|no_sep: '([^\\\']|\\.)'
number: [+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)[fl]?
```

`displayname` is the string displayed at the bottom of `e`. `extensions`
is a list of regexes to match the filenames. Highlighting keys are `comment`,
`keyword`, `type`, `pragma`, `string`, `number`, and `predefined`. By appending
`|no_sep`, the user signals to `e` that no separator is needed, i.e. highlighting
works even if the matched string is part of a longer word. The values are regexes.

If you provide a second regex (must by divided by a newline), `e` assumes that everything
between the two matches should be colored (useful for e.g. multiline comments).

### Scripting through Lua

The editor has scripting capabilities in Lua. Thus far I've only documented them
in [a blog post](http://blog.veitheller.de/Editing_Revisited.html), but this
post should give you a good overview of how to write Lua scripts for `e`. There
is also an example [`.erc`](https://github.com/hellerve/e/blob/master/.erc)
file in the repository that you can look at for inspiration.

That's it!

### Tabs vs. Spaces

I personally always use spaces for indentation, thus `e` does the same. It will
always convert all of the tabs in your file to spaces automatically for you, and
I understand if that bit of behavior is undesired by some. It is very much by
design, though, and probably won’t change.

----

Have fun!
