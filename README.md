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
Multithreaded HTTP example(`examples/cstest.hob`)
```
import "tcp/lib.hob" as tcp;
import "libc/lib.hob" as c;
import "pthread/lib.hob" as pthread;

use c::mem;
use c::io;
use c::process;
use pthread::thread;

fun handle_conn(handle: *tcp::handle::TcpClientHandle): void {
	defer mem::free(handle as *void);
	defer tcp::handle::close(handle);

	io::puts("new connection!\0");
	var buf = [0 as u8, 0, 0, 0, 0];
	while tcp::handle::read(handle, buf, 5) == 5 { }

	var fd = io::open("test.html\0", 0);
	if fd < 0 {
		io::puts("failed to open a file test.html\0");
		return;
	}
	defer io::close(fd);

	var stat: io::FileStatus;
	if io::file_status(fd, &stat) != 0 {
		io::puts("failed to get file status!\0");
		return;
	}

	tcp::handle::send(handle, "HTTP/1.1 200 OK\r\n", 17);
	tcp::handle::send(handle, "Content-Type: text/html\r\n", 25);
	tcp::handle::send(handle, "\r\n", 2);
	if io::send_file(handle.*.socket, fd, 0, stat.size) < 0 {
		io::puts("failed to send file!\0");
		return;
	}
	
	io::puts("connection closed!\0");
}

fun main(): i32 {
    var server: tcp::server::TcpServer;
    if !tcp::server::init(&server, 9000) {
        return 1;
    }
    defer tcp::server::stop(&server);

	var func = handle_conn;

	while true {
		var client = mem::malloc(20) as *tcp::handle::TcpClientHandle;
		if !tcp::server::accept(&server, client) {
			return 1;
		}
		var t: thread::Thread;
		if thread::create(&t, 0 as *pthread::attr::Attr, func as fun (*void) -> *void, client as *void) != 0 {
			io::puts("failed to create the thread!\0");
		}
		if thread::detach(t) != 0 {
			io::puts("failed to detach thread!\0");
		}
	}
    return 0;
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
