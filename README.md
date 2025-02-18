# Hob-lang
## Getting Started
Requires GNU `gcc`, `cmake` and `llvm`
``` bash
mkdir .build
cd .build
cmake ../
cmake --build ./
./hoblang compile <source> <output>
gcc <output> -o <binary-output>
./<binary-output>
```
## Syntax
Multithreaded HTTP example(`examples/http_nolibc.hob`)
```
import "std/process.hob" as process;
import "std/tcp.hob" as tcp;
import "std/io.hob" as io;
import "std/fs.hob" as fs;
import "std/salloc.hob" as salloc;

use tcp::Socket;
use fs::File;

const PORT: u16 = 8080;
const BACKLOG: i32 = 16;

const BUFFER_SIZE: i32 = 1000;

fun die(msg: []u8) -> void {
    io::println(msg);
    process::exit(1);
}

fun handle_conn(client: Socket, file: []u8) -> void {
	defer tcp::close(client);
	var buf: [BUFFER_SIZE]u8;
	var buf_slice = &buf as []u8;
	while io::read_from(client, &buf_slice) as i32 == BUFFER_SIZE {}
	io::write_to(client, "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\n\r\n");
	io::write_to(client, file);
	io::println("wow, closed the connection!");
}

fun read_file(fd: File) -> []u8 {
	var length = io::fd_remains(fd);
	var content: []u8;
	content.raw = salloc::allocate(length as u32) as *u8;
	content.length = length as i32;

	if io::read_from(fd, &content) != length {
		die("failed to read content from file");
	}

	return content;
}

fun main() -> i32 {
	var filename = "test.jpg\0";
	var file = fs::open_read(filename);
	if file < 0 {
		io::println("cannot open file `test.jpg`");
		return 1;
	}
	defer io::close(file);
	var content = read_file(file);
	io::close(file);

	var server = tcp::bind(PORT, BACKLOG);
	if server < 0 {
		io::println("cannot bind a socket :(");
		return 1;
	}
	# tcp::set_non_blocking(server);
	# unnecessary but.. why not?)
	var handle_func: fun (Socket, []u8) -> void = handle_conn;

	while true {
		var client = -1;
		while client < 0 {
			client = tcp::accept(server);
		}
		var pid = process::fork();
		if pid == 0 {
			handle_func(client, content);
			process::exit(0);
		} else {
			io::close(client);
		}
	}
	return 0;
}

fun _start() -> void {
	process::exit(main());
}

```
### To compile
```
cd examples
mkdir .build
hoblang compile examples/cstest.hob .build/cstest.o
gcc .build/cstest.o -o .build/cstest
.build/cstest
```
