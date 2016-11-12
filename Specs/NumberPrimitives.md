# Number primitives

There are the following built in number types:
```i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, char```
They are exactly what you expect, with char being an u8 with a different type.
A float must have a deciaml point somewhere.
A literal can take on the following forms:
* string of digits: 462865 (of type: i32 if it fits, u32 otherwise (optional warning))
* string of digits with type: 840624u64
* **the whole "with type" thing continues**
* hexadecimal: 0xdeadbeef (i32, u32 if too large)
* hexalong: 0xdeadbeefleefu64 (u64)
* binary: 0b101011
* digits with decimal point 3.5 (f32)
* digits with order of magnitue 3e4 (i32)
* digits with point and order of magnitude 2.5e3 = 2.5*10^3 = 250 (f32)
* hexadecimals with point and power of two: 0x2.5p4 = 0x25/16*2^4 = 37=0x25 (f32)
* Maybe have: hexadecimal without decimal point but with exponent (i32) 
* Maybe also let exponents be negative (f32)

As written in the list, the type infered when none is written, is i32 for numbers less than 0x8000000, and
u32 for numbers larger than 0x80000000. This very number is equal to 0 minus itself, meaning you need to
specify weather or not it's an i32 or an u32. If you don't, a warning is given, and u32 is used,
resulting in a positive number. If i32 is explicitly used, the lowest possible value for a signed 32 bit integer is used.

### Cases
When two values are combined in some way, the largest type is used.
If an i8 is turned into an i16, the sign is filled on the right, making the signed value equal.
With unsigned numbers, the new bits are set to 0.
When doing arithmetic with unsigned and signed integers, the following rules apply:
* If one integer is of a bigger type than the other, its sign-status is kept in the result
 * i32 + u16 = i32
 * i16 - u32 = u32
* If the integers are of the same size, the result is an unsigned integer
 * i32 + u32 = u32

###### When comparing:
Warnings are given when comparing unsigned and signed values, but the operation is still done correctly, just slower.
When checking for equality, if only one number is signed, they are not equal if one of them have their most significant bit. (Only check one, as the equality is checked later anyways).
When comparing, if one is signed and has its MVB, it's the lowest, while if the unsigned has its MVB it's the highest.
