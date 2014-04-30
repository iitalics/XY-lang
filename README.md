Syntax Overview (WIP)
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
    [ 1, 2, 3 ]             ; = [ 1, 2, 3 ]
    [ 5, 9, 4 ] . 0         ; = 5
    [ 5, 9, 4 ] . 1         ; = 9
    [ 5, 6, 2, 3, 5 ] .. 2  ; = [ 2, 3, 5 ]
    [ 1, 3, 2 ] + [ 7, 8 ]  ; = [ 1, 3, 2, 7, 8 ]
    [ 1, 2 ] == [ 1, 2 ]    ; = true
    [ 1, 2 ] == [ 2, 1 ]    ; = false

Functions
-------------------------------

Simple function declaration:

    let main () = 3
    
Overloading with constants:

    let foo (0) = 12
    let foo (n) = n + 1
    
    let main () = foo(3) + foo(0)  ; = 12 + 3 + 1 = 16

Overloading with conditions:

    let sign (n : n < 0) = -1
	let sign (n > 0) = 1  ; equiv. to 'let sign (n : n > 0) = ...'
	let sign (0) = 0
	
	let main () = sign(-32)   ; = -1

	