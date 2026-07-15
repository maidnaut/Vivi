# Vivi (.vivi)

Vivi is a hackable modern metaprogramming language with reflective preprocessing.

It is a gradually typed, embeddable scripting lang with optional type annotation and a lightweight core.

Vivi is interpreted, can interpret itself, and runs linearly. 

Its #proctime phase works like #comptime, giving you full reflective access to the program before execution

Vivi is NOT
- A language that has macros. Generate it with #proctime.
- An object oriented language. No objects, no this.
- A virtual machine, virtual environment, or bytecode enabled interpreter.
- A language with a fully featured core library. You extend it yourself, in Vivi.
- Rigid or locked down, you can hack any part of the language to suit your needs.
- A language that locks you into itself, you can interop.
- Free of opinions, but it is also a lang that doesn't get in your way.

Currently only the lexer, ast and parser are implemented. Run vivi with ``./build/vivi . --ast`` after compiling.

Please see the [spec](https://github.com/maidnaut/Vivi/blob/main/docs/spec.txt) for a full breakdown.

---

Syntax Highlighting is available for [vscode](https://marketplace.visualstudio.com/items?itemName=JulianKylander.vivi) and [tree-sitter](https://github.com/jkylander/tree-sitter-vivi), courtesy of [jkylander](https://github.com/jkylander).
