# Vivi Script (.vivi)

Vivi is a hackable modern metaprogramming language with reflective preprocessing.

It is a gradually typed, embeddable scripting lang with optional type annotation and a lightweight core.

Vivi is interpreted, can interpret itself, and runs linearly.

Its `#proctime` phase works like `#comptime`, giving you full reflective access to the program before execution

## Vivi is NOT
- A language that has macros. Generate it with `#proctime`.
- An object oriented language. No objects, no this.
- A virtual machine, virtual environment, or bytecode enabled interpreter.
- A language with a fully featured core library. You extend it yourself, in Vivi.
- Rigid or locked down, you can hack any part of the language to suit your needs.
- A language that locks you into itself, you can interop.
- Free of opinions, but it is also a lang that doesn't get in your way.

## Operators

```
::              // Typically in modern languages, :: is immutable comptime declaration,
                // but since vivi is interpreted, :: is *proctime* sugar for immutable declaration
:=              // Mutable runtime declaration
=               // Reassignment
==              // Is equal to
in,             // x is in y
not, !=         // Not equal to
and, &&
or, ||
!               // Logical not (!var)
+, -, *, /,     // Math
+=, -=, *=,
/=, %
++, --          // Plus, Minus
x?y:z           // Ternary (condition ? expression_if_true : expression_if_false)
..              // Slice operator (0..5)
:               // Separator, for structs
|               // Union
>, >=,          // Greater than, less than
<, <=
.               // Accessor chaining
,               // General purpose string concatenation operator
                // print("this", "that")
                // a := "this", word, "that"
                // c := a, b
                // Leaving a trailing string ie a,\n is an error because it expects a termination
```

## Sugary inferred declaration, args always optional:

```
name := {}      // struct
name := []      // array
name := x       // variable assignment
u32 name := 0
```

## Arrays

```
// Works like lua's tables, 0 indexed and can nest
t := []                             // Empty array instantiation
t := ["apple", "banana", "orange"]  // Populate an array
t[0]                                // Returns apple

t := ["key": [1, 2, 3], 4, 5]       // Populate a nested array
t[0]                                // 4
t[1]                                // 5
t["key"]                            // [1,2,3]
t["key"][0]                         // Chains into the first index of "key" (1)

t["key"].add(0, val)                // Insert val at specified index
t["key"].take(0)                    // Removes the specified entry
t["key"].push(val)                  // Pushes val at the end of "key":[1,2,3] to become [1,2,3,val]
t["key"].pop()                      // Removes last entry
t["key"].sort()                     // Sorts sequentially
```

## Types are optional, but special scope cases for mass type declaration (like enum{...}):

```
u32 {
    a := 1
    b := 2
}
```

## Types:

```
null
rune            // Char
string
bool
i32             // other int types, 16, 64 etc, 32 is default
u32             // other unsigned types
f32             // other float types
struct
enum
ext             // External asset
```

## ext External Assets

External assets are just magic io/filesys types that autoresolve

```
ext icon :: "assets/icon.png"
```

Loaded into runtime with metadata, can be used in other Functions
can be passed through preproc/interop for reflection or passing off to raylib or something

## Type casting as functions.

```
i32, u32, f32, etc, cannot explicitly cast something that doesn't match, ie enum or function or null

i32(var)        // Explicit cast
```

## Statements

```
if () {...}        // Typical if statement

if () {
    ...
} else if () {
    ...
} else () {
    ...
}

while true {...} (or while var {...} etc)

for 0..10 {...}
for 0..var {...}
for i := 0; i < 10; i++ {...}
for i, name in array {...}
for key, value in struct { 
    print("{key} = {value}")
}
// For loops are versatile, they infer based on context
```

## If as Expression

```
msg :=  if hp > 50 { "good" } 
        else if player.health > 20 { "bad" } 
        else { "almost dead" }
```

## Functions

Functions operate on type inference (type is return type and is optional):
```
fn name :: () -> type {...}
// Functions are always constant, always need the fn keyword
```

Function declarations for empty placeholders (args always optional on declaration):
```
fn name :: (args){}
```

Anonymous functions are simply called with:
```
fn (){}
```

Default args:
```
fn name :: (a := "test", b := 1) {...}
```

Optional explicitness with type annotations:
```
fn name :: (f32 x, rune y) {...}
```

Can mix type annotations with default arguments:
```
fn name :: (f32 x := 1.0, rune y := "blep") {...}
```

Return types are optional for functions, but can still be useful
```
fn name :: () -> bool {...}
```

```
// Note: return is a mandatory keyword for returning a type
// You can also return a custom type, ie Entity if defined
```

## Constructors infer from context:

```
struct Vec2 {           // Typedef type constructor
    i32 x: 0,           // Type declaration is optional, interpreter will infer if you don't provide one,
    i32 y: 0            // struct type setting is different than elsewhere with : delimiter
}
v := Vec2(10, 20)

// Same as
struct Vec2 {x:0, y:0}
v.x = 10                // No need to re-declare, can just use =
v.y = 20
```

## Methods

```
My_Struct := {
    a: "blep",
    my_fn: fn () {
        print(self.a)
    }
}

My_Struct.my_fn() // returns "blep"
```

## Simple prints

```
print("Hello, Black Mage!") // automatic newline always
```

Library functions have extensive pattern matching to make them intuitive, ie string literals via interpolation and type coercion:

```
print("asdf {x + y * z / w} blep")

x := 5
y := 10
print("{x + y}") <-- prints 15, +, -, *, // is always arithmetic ONLY, no concatenation EVER with it

x := 5
print("10{x}") <-- 105

x := 5
y := 10
print("{x}{y}") <--510

"10" + 5 = 15 <-- type coercion
"10" + "10" = 20 <-- also type coercion
```

## Intuitive type coercion, complaint on mismatch

```
i32 x := 10
print(x + "test {x}") <-- type mismatch, can't add an int to a non-int string
x := "10" * 2 <-- returns 20
```

## String concatenation

```
a := "1"
b := "2"
c := "3"
print(a, b, c) // Returns 123
```

## Switch statements:

```
switch x {
    default:
    "1":
        ...

    2:
        ...
}

// You can also use slices in switch statements cus idk why no lang has thought of that

switch x {
    0..3:
        ...
    
    4:
        ...
}

// You can also type check to use switches as match statements

switch x {
    i32:
        ...

    string:
        ...
}
```

## Can alternatively test with if statements and use them as asserts:

```
if (x == string) {...}
```

## Includes:

```
#import "core:math"             // Core library import
#import "library.vivi"          // Normal import and provide global function calls
#import "library.vivi" as vivi  // Import as vivi namespace (vivi.function)
```

## Import shadowing

There are no overloads. You can shadow imported functions locally.

```
#import "core:math"

eg: math.pow()

fn :: math.pow() {
    // whatever
}
```

## Comments:

```
#
;;
//
/* ... */

Don't care, use whatever comment style you want
```

```
// Try/catch
try {
    // do something
} catch (err) {
    print(err) // Prints error and proceeds normally, doesn't crash unless panic()
}
```

## Other stuff

```
defer fn()      // Runs before returning
return          // Drops out of the function, if -> type'd, mandatory return value
break           // Break's a loop prematurely
continue        // Keep processing
exit            // Exit's program
panic()         // Function to forcibly segfault the application and return an error
|               // Union, eg var := expression | fallback_statement
```

## Typecast Functions

```
i16()
i32()
i64()
u16()
u32()
u64()
f16()
f32()
f64()
```

## #proctime

Signals a scope to the preprocessor to fire during processing rather than runtime.
When using `:=` in proctime, the interpreter uses it as mutable proctime local variable declaration.
They only exist during preprocessing and don't leak into runtime and are destroyed before the program runs.
Note: `::` is *already* proctime by default, but `#proctime` is useful for preprocessor stuff.

```
#proctime {
    // These are mutable *proctime* locals (using :=)
    pi := 3.1415926535
    radius := 10.0
    circumference := 2 * pi * radius   // computed now, stored in a proctime local

    // To make it available at runtime, you must export it with ::
    CIRC :: circumference   // now a runtime constant
}
print("{CIRC}")   // prints 62.83185307
```

## #proctime reflection

Reflection functions only available in #proctime blocks

```
type_of(expr)           -> type         // Returns inferred type of expression, ident or literal
kind_of(type)           -> string       // Returns type
type_name(type)         -> string       // Returns declared name of type
fields(type)            -> []field      // Array of structs, {name: string, type: type, default: any}
methods(type)           -> []method     // Array of structs, {name: string, params: []params, return_type: type}
param(fn)               -> []param      // Array of structs, {name: string, type: type, default: any}
return_type(fn)         -> type         // Type struct or null if not annotated
has_field(type, name)   -> bool
has_method(type, name)  -> bool
has_function(name)      -> bool
has_type(name)          -> bool
has_symbol(name)        -> bool
is_type(expr, type)     -> bool
is_exported(symbol)     -> bool
symbols()               -> []symbol     // Array of structs, {name: string, kind: string}
functions()             -> []function   // Array of structs, {name: string, params: []params, return_type: type}
types()                 -> []type       // Array of type structs
constants()             -> []constant   // Array of structss, {name: string, value: any}
imports()               -> []str        // Array of module name strings
emit(code_str)          -> void         // Injects code (string)
eval(code_str)          -> any          // Parses and runs code, returns result

#proctime {
    for t in types() {
        if t.kind == "struct" {
            fs := fields(t)
            # fs is an array of { name, type, default }
            # No methods, just data
        }
    }
}
```

## Generating C Wrappers

highly tbd, eg:

```
#proctime {
    c_functions := [
        { name: "sqrt", return: "f64", params: [{"x", "f64"}] },
        { name: "cos",  return: "f64", params: [{"x", "f64"}] }
    ]
    for fn in c_functions {
        emit(`#interop c {
            extern fn ` + fn.name + ` (` + join(fn.params, ", ") + `) -> ` + fn.return + `;
        }`)
    }
}
```

## Libraries & language features

```
core:sys            // System functions
core:io             // File handling
core:time           // Delta time, etc
core:log            // Console functions
core:net            // Networking features
core:json           // JSON Parsing
core:bin            // Binary packing
core:math           // Math library
core:string         // String functions, split join format etc
core:regex          // I hate regex but it might be nice
core:db             // SQL database stuff
core:debug          // Runtime query interface

Interop             // Native foreign function interface, #interop c {...}
Embedding C Api     // Call Vivi from inside C/C++
Proctime Reflection // Inspect loaded symbols
```

## Library structure

```
core is not a reserved namespace
core is a folder inside /lib in Vivi's root directory.
vivi/lib/core/sys.vivi
Available as core:sys

You can shadow library functions locally,
or you can just hack it/patch it/delete it/replace it
I literally don't care, the core lib isn't sacred.

You can also create new namespaces in Vivi's lib folder
vivi/lib/tests/unit_test.vivi
and it would be available as tests:unit_test
```

## Command line

```
vivi <dir>              // looks for main() in <dir>, eg: . or /folder
vivi <script>           // runs the supplied script.vivi, no main needed

vivi <...> --reflect    // runs the pre processor and prints out types for the supplied thing
vivi <...> --debug      // runs the debugger
vivi <...> --lexer      // prints out the lex
vivi <...> --ast        // prints out the ast tree

vivi --version          // prints out the interpreters version
```
