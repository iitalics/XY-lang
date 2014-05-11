xy-lang
===============================

XY is a purely functional, stateless, dynamically typed programming language.

It is currently interpreted, although a bootstrapped compiler is currently being written.

Syntax Overview
===============================


Comments:
-------------------------------
    ; this is a comment

Numbers
-------------------------------
    1 + 2        ; = 3
    3 * 5        ; = 15
    6(8)         ; = 48
    (5 - 1) * 3  ; = 12
    5 - 1 * 3    ; = 2
    300 * 0.25   ; = 75

Booleans
-------------------------------
    true             ; = true
    false            ; = false
    3 < 4            ; = true
    8 == 9           ; = false
    8 != 9           ; = true
    1 > 2 and 3 < 4  ; = false
    5 > 6 or 1 < 2   ; = true
    !(true or false) ; = false

Lists
-------------------------------
    [1, 2, 3]               ; = [ 1, 2, 3 ]
    3 .. 9                  ; = [ 3, 4, 5, 6, 7, 8, 9 ]
    [5, 9, 4] . 0           ; = 5
    [5, 9, 4] . 1           ; = 9
    [5, 6, 2, 3, 5] .. 2    ; = [ 2, 3, 5 ]
    [1, 3, 2] + [7, 8]      ; = [ 1, 3, 2, 7, 8 ]
    [1, 2] == [1, 2]        ; = true
    [1, 2] == [2, 1]        ; = false
	
There are also unary operators 'hd' and 'tl', similar to Scheme's (car) and (cdr)

	hd [1, 2, 3, 4]         ; = 1
	tl [1, 2, 3, 4]         ; = [2, 3, 4]
	hd tl [1, 2, 3, 4]      ; = 2
	
Functions
-------------------------------

Simple function declaration and calling:

    let twice (x) = x * 2

    let main () = twice(4.5)

Overloading with constants:

    let foo (0) = 12
    let foo (n) = n + 1

    let main () = foo(3) + foo(0)  ; = 12 + 3 + 1 = 16

Overloading with conditions:

    let sign (n : n < 0) = -1
    let sign (n > 0) = 1       ; equiv. to 'let sign (n : n > 0) = 1'
    let .. (0) = 0             ; '..' refers to the name of the last defined function

    let main () = sign(-32)    ; = -1

Other parameters may be used in the conditions of a parameter

    let filter (f, []) = []
    let .. (f, a : f(hd a)) =
        [hd a] +
        filter(f, tl a)
    let .. (f, a) =
        filter (f, tl a)
    ; see 'list comprehension' for built-in list filtering

Alternative syntax:

    x -> y        ; equiv. to 'y(x)'
    x -> f -> g   ; equiv. to 'g(f(x))'

Order of operations precedence for '->' is less than numeral operators but greater than comparison operators

    ; these are all equivalent
    c == a + b -> f
    c == ((a + b) -> f)
    c == f(a + b)

'With' Aliasing
-------------------------------

Syntax:

    with (var1 = value1, var2 = value2... )
        expression
	
	with ([var1, var2, var3..] = list, ...) 
		expression
	
Example:

    let area (diameter) =
        with (radius = diameter / 2, pi = 3.1415926535)
            pi * radius ^ 2
	
	let quadratic_formula (terms) =
		with ([a, b, c] = terms)
			[ (-b + (b ^ 2 - 4(a)(c)) ^ 0.5) / 2(a),
			  (-b - (b ^ 2 - 4(a)(c)) ^ 0.5) / 2(a) ]
	
	let reverse (a) =
		with ([head, tail..] = a)
			reverse(tail) + [head]
Lambdas
-------------------------------

Syntax:

    @(args) = body

    @{ let (args1) = body1
       let (args2) = body2
       let (args3) = ... }

Example:

    let main () =
        filter(@(x) = x < 3,
            [ -1, -2, 3, 4, 0, 2, -3, 1, 5 ])  ; = [ -1, -2, 0, 2, -3, 1 ]

Alternative Syntax:

    (&+)      ; equiv. to '(@(x, y) = x + y)'
    (`* 2)   ; equiv. to '(@(x) = x * 2)'

Planned, unimplented syntax:

    #(3 + 2)  ; equiv. to '(@() = 3 + 2)'

List Comprehension
-------------------------------

Syntax:

    list $ iterator : filter_expression = map_expression
    ; filter is applied first, followed by map.
    ; either are optional, but you must supply atleast one

Examples:

    let threshold (a, min) = ; list of each value in 'a' not less than 'min'
        a $ t : t >= min

    let double_all (a) =     ; list of the values of a, doubled
        a $ t = t * 2

Maps
-------------------------------

Syntax:

    { key1 = value1, key2 = value2 }
    map_object::key1

Examples:

    with (foo = { x = 2, y = 3 })
        foo::x + foo::y             ; 5
	
	with (a = { x = 1, y = 2},
			b = { x = 4, z = 3 })
		(a + b)                     ; { x = 4, y = 2, z = 3 }


Globals
-------------------------------

Unimplemented.  Planned syntax:

    let PI = 3.1415926535

    let area (r) =
        PI * r ^ 2