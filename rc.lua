
add_syntax[[

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

split:

displayname: go
extensions: .*\.go
keyword: (break|case|const|continue|default|defer|else|fallthrough|for|func|go|goto|if|import|interface|iota|make|new|package|range|return|select|struct|switch|type|var|append|cap|close|complex|copy|delete|imag|len|make|new|panic|print|println|real|recover)
type: (uint8|uint16|uint32|uint64|int8|int16|int32|int64|float32|float64|complex64|complex128|byte|rune|uint|int|uintptr|string|map|chan)
predefined: (true|false|nil)
comment|no_sep: //.*$
comment|no_sep: /\*.*\*/
comment|no_sep: /\*.*
                 .*\*/
string|no_sep: '([^\\\']|\\.)'
string|no_sep: "([^\\\"]|\\.)*"
number: [+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)

split:

displayname: haskell
extensions: .*\.hs$
comment|no_sep: --.*$
keyword: (!|-<|-<|->|::|<-|=>|#|@|[\||\|]|_|as|case|of|class|data(\s+family)?|default|deriving|do|forall|foreign|hiding|if|then|else|import|infix(l|r)?|instance|let|in|mdo|module|newtype|proc|qualified|rec|type(\s+family)?|where)
type: [A-Z][a-zA-Z]*
comment|no_sep: \{-.*-\}
comment|no_sep: \{-.*
                 .*-\}
pragma: `.*`
string|no_sep: '([^\\\']|\\.)'
string|no_sep: "([^\\\"]|\\.)*"
number: [+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)

split:

displayname: javascript
extensions: .*\.js
            .*\.jsx
comment|no_sep: //.*$
comment|no_sep: /\*.*\*/
comment|no_sep: /\*.*
                 .*\*/
predefined: (null|true|false)
keyword: (break|case|catch|class|const|continue|debugger|default|delete|do|else|export|extends|finally|for|from|function|if|import|in|instanceof|new|return|super|switch|this|throw|try|typeof|var|void|while|with|yield)
keyword: (enum|implements|interface|let|package|private|protected|public|static)
keyword: await
type: (Object|Number|Array|Boolean|String|Error|Date|JSON|Math|Regexp)
number: /[^/]/\w
string|no_sep: '([^\\\']|\\.)*'
string|no_sep: "([^\\\"]|\\.)*"
number: [+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)

split:

displayname: md
extensions: .*\.md
            .*\.markdown
pragma|no_sep: ```[^ ]*
pragma|no_sep: #.*$
number: \*\*[^(\*\*)]\*\*
number: \*[^\*]\*
keyword: -\s+

split:

displayname: python
extensions: .*\.py
pragma: (from|import)
keyword: (and|as|assert|break|class|continue|def|del|elif|else|except|exec|finally|for|global|if|in|is|lambda|not|or|pass|raise|return|try|while|with|yield)
predefined: (abs|all|any|basestring|bin|bool|bytearray|callable|chr|classmethod|cmp|compile|complex|delattr|dict|dir|divmod|enumerate|eval|execfile|file|filter|float|format|frozenset|getattr|globals|hasattr|hash|help|hex|id|input|int|isianstance|issubclass|iter|len|list|locals|long|map|max|memoryview|min|next|object|oct|open|ord|pow|print|property|range|raw_input|reduce|reload|repr|reversed|round|set|setattr|slice|sorted|staticmethod|str|sum|super|type|tuple|unichr|unicode|vars|xrange|zip|__import__|True|False|None)
comment: #.*$
string|no_sep: """[^(""")]*"""
string|no_sep: """.*
               .*"""
string|no_sep: '([^\\\']|\\.)*'
string|no_sep: "([^\\\"]|\\.)*"
type: @[^\(]*
number: [+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)[l]?

split:

displayname: stx
extensions: .*\.stx
keyword|no_sep: (displayname|extensions)
type|no_sep: (type|keyword|pragma|predefined|comment|match|number|string)
comment|no_sep: no_sep


]]

