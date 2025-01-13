# Hob-lang
## Why?
Main idea hidden in the name of this lang. It is HOBBY language. It means that it creates only in recreaional purposes. I know what code is so bad, i want to refactor it in near future
## Getting Started
Requires GNU `gcc`, `cmake` and `llvm`
``` bash
mkdir .build
cd .build
cmake ../
cmake --build ./
./hoblang compile <source> <output>
```
## Syntax
```
module main;

extern(putchar) fun put_char(c: u8): void
extern(getchar) fun get_char(): u8
extern(puts) fun put_str(str: u8*): void

fun get_int(): i32 {
	var res: i32 = 0;
	var c: u8 = get_char();
	while c != '\n' {
		res = res * 10 + (c - '0') as i32;
		c = get_char();
	}
	return res;
}

fun int_len(val: i32): i32 {
	if val == 0 {
		return 1;
	}
	var len: i32 = 0;
	while val > 0 {
		len = len + 1;
		val = val / 10;
	}
	return len;
}

fun int_at(val: i32, at: i32): u8 {
	while at > 0 {
		at = at - 1;
		val = val / 10;
	}
	return (val - (val / (10 as i32) * 10 as i32)) as u8;
}

fun put_int(val: i32): void {
	if (val < 0) {
		val = 0 - val; # TODO: unary
		put_char('-');
	}
	
	var len: i32 = int_len(val) - 1;
	while len >= 0 {
		put_char('0' + int_at(val, len));
		len = len - 1;
	}
}

fun test_arr1(): void {
	var test: i32* = [23, 34, 45, 56, 12];
	put_char('[');
	var i = 0;
	while i < 5 {
		if i != 0 {
			put_char(',');
			put_char(' ');
		}
		put_int(test[i]);
		i = i + 1;
	}
	put_char(']');
	put_char('\n');
}

fun main(): void {
	put_str(['H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0']);
	put_str("Hello, world using strs!\0");
	test_arr1();
	var sum: i32 = 0;
	while true {
		put_char('>');
		put_char(' ');
		var a = get_int();
		sum = sum + a;
		put_int(sum);
		put_char('\n');
	}
}
```
