# Number primitives

There are the following built in number types:
```i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, char```
They are exactly what you expect, with char being an u8 with a different type.
A literal can take on the following forms:
* string of digits: 462865 (of type: i32)
* string of digits with type: 840624u64
* **the whole "with type" thing continues**
* hexadecimal: 0xdeadbeef (i32)
* hexalong: 0xdeadbeefleefu64
* digits with decimal point 3.5 (f32)
* digits with point and order of magnitude 2.5e3 = 2.5*10^3 = 250 (f32)
* hexadecimals with point and power of two: 0x2.5p4 = 0x25/16*2^4 = 37=0x25 (f32)

### Cases
When two values are combined in some way, the largest type is used.
If an i8 is turned into an i16, the sign is filled on the right, making the signed value equal.
With unsigned numbers, the new bits are set to 0.
unsigned - signed = unsigned
unsigned + signed = unsigned (both ways)
unsigned > signed = what you expect? Should just work?
