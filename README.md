# e

This is a fork of [e](https://github.com/hellerve/e). The goal is to simplify
even more the editor. Only basic features should be supported by the core
editor. It should be possible to implemented the rest as a lua script.

The original `e` is a braindead editor by [Veit
Heller](http://blog.veitheller.de). It is inspired by
[kilo](https://github.com/antirez/kilo), and a large swath of the code is
similar.

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
  - Markdown (unfinished)
  - Python (experimental)
  - JavaScript (experimental)
  - Go (experimental)
  - Haskell (experimental)

## Installation

```sh
git clone https://github.com/hellerve/e
cd e
make install # install_lua for Lua support
```

## Troubleshooting

Linux and Lua trouble `e` sometimes. If you run into any problems with compiling or linking Lua, chances are this is due to missing libraries or problematic Lua configuration. Fret not, here are some answers to common problems!

On some Linux systems, you might have to first install `libreadline` — or  `libreadline-dev`, depending on your package manager. This is required by Lua.

If there are still problems, you can adjust the `LUA_OPT` flag to your operating system, and see whether that changes anything. Valid values are: `aix bsd c89 freebsd generic linux macosx mingw posix solaris`. `generic` is the default. Your make call might then look like this:

```
make install_lua LUA_OPT=linux
```

If the linker complains about missing functions, you can adjust the `LUA_FLAGS` flag to your needs. On Ubuntu, for instance, it is needed that we link `libdl` and l`ibmath` into `e` - look out for this if the linker complains about dlopen and various math functions such as `floor`. This leads us to the following call:

```
make install_lua LUA_OPT=linux LUA_FLAGS="-lm -ldl"
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

### Scripting through Lua

By default, `e` creates a directory called `.config/e` in the user's home
directory (the location is overridable by providing `CFGDIR` to `make
install`).  There, `e` will search for a startup scipt file called `rc.lua`.

It uses an embeded lua VM. It follows a description of how it was integrated.

The first step in building Lua integration into my editor was registering the l key to open a prompt where you can type Lua code and have it be evaluated when you press enter. I've also added support for a resource file—.erc in the user's home directory by default, tweakable at compile time—, and added a small library to interact with the editor and register custom commands.

There's just a handful of functions and variables to work with, but they are in fact enough to add useful and interesting features to the editor. I might add more in the future, if anyone has good arguments for adding to the list. So, without further ado, here's a complete listing of the API:

```
-- print something in the status line
message("string")

-- insert text at the current cursor position
insert("string")
-- insert text at the current position (appends a newline)
insertn("string")
-- delete a number of characters at the current position
delete(number)
-- move to a given cursor position
move(number, number)
-- open another file, closing the current file
open("string")
-- prompt the user for input
string = prompt("input: %s")

-- get the current cursor position
number, number = get_coords()
-- get the window size
number, number = get_bounding_rect()
-- get the editor text
string = get_text()
-- get tab width
number = get_tab()
-- set tab width
number = set_tab()
-- get filename
string = get_filename()

-- a table containing custom edit keys
keys = {}
-- a table containing custom meta commands
meta_commands = {}

-- add syntax highlighting
add_syntax("string")
```

The `keys` and `meta_commands` variables might not be immediately obvious, so let me give you an example for both of them. Suppose you want to register a custom command in meta mode—a mode accessible by typing : and behaving similar to that mode in Vim—, called hi, you just add a function containing the actions you want to execute to meta_commands, like so:

```
meta_commands["hi"] = function()
  insert("hi")
  message("hi inserted")
end
```

The next time you type `<Ctrl-p>hi<return>`, `hi` will be inserted at the current cursor position and your status bar will helpfully tell you what just happened. This system is simple, yet tremendously powerful.

The `keys` variable works similarly. If you want to regi$ster a custom function that will be run whenever a given key is pressed in initial mode—normal mode for Vim users—, you just register it in the `keys` dictionary.

I've certainly seen fancier editor integrations before, but so far this seems to do the job just fine. And, considering I'm the only user of this editor, I feel like I have the right to only implement what I need in order to be productive.

It follows an example configuration file in the repository that you can look at
for inspiration. It must be placed in `~/.config/e/rc.lua`.

```
et_tab(2) -- sets tab with

-- registers a meta command under `:hi` that inserts hi and notifies you
meta_commands["hi"] = function()
  insertn("hi")
  message("hi inserted")
end

-- registers a custom key under `%` that implements word count
keys["%"] = function()
  s = get_text()
  count = 0

  for word in s:gmatch("%w+") do count = count + 1 end

  message(count .. " words")
end

-- lists all meta commands
function get_meta_commands()
  res = ""

  for k, _ in pairs(meta_commands) do
    res = res .. k .. ", "
  end

  return res
end

-- lists all custom keys
function get_keys()
  res = ""

  for k, _ in pairs(keys) do
    res = res .. k .. ", "
  end

  return res
end

-- registers a meta command `:help` that lists all meta commands and custom keys
meta_commands["help"] = function()
  s = "meta commands: " .. get_meta_commands() .. "custom keys: " .. get_keys()

  message(s:sub(1, -3))
end
```

## Adding syntax

The core editor only supports `c` and `markdown` syntax. Other syntax
highlighting can be added using the lua function `add_syntax`. It accenpt a
string as parameter, in the following format:

```
add_syntax [[

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

]]
```

`displayname` is the string displayed at the bottom of `e`. `extensions`
is a list of regexes to match the filenames. Highlighting keys are `comment`,
`keyword`, `type`, `pragma`, `string`, `number`, and `predefined`. By appending
`|no_sep`, the user signals to `e` that no separator is needed, i.e. highlighting
works even if the matched string is part of a longer word. The values are regexes.

If you provide a second regex (must by divided by a newline), `e` assumes that everything
between the two matches should be colored (useful for e.g. multiline comments).

To specify two or more syntaxes a single `add_syntax` call, the different sections
must be separated by a line with the `split:` statement.

### Tabs vs. Spaces

I personally always use spaces for indentation, thus `e` does the same. It will
always convert all of the tabs in your file to spaces automatically for you, and
I understand if that bit of behavior is undesired by some. It is very much by
design, though, and probably won’t change.

----

Have fun!
