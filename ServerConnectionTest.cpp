#include <uv.h>
#include <string>
#include <vector>
#include "util.h"
#include <stdio.h>
#include <stdint.h>
#include <uv.h>

/* Libuv multiple loops + thread communication example. Written by Kristian
 * Evensen <kristian.evensen@gmail.com>  */

 //rpath is needed because it is an argument to the linker (not compiler) about
 //where to look
 //gcc -Iinclude -g libuv-test.c -o libuv_test -L "./" -l uv -Xlinker -rpath -Xlinker "./" -lrt


void timer_callback(uv_timer_t *handle) {
	uv_async_t *other_thread_notifier = (uv_async_t *)handle->data;

	fprintf(stderr, "main thread Timer expired, notifying other thread\n");

	//Notify the other thread
	uv_async_send(other_thread_notifier);
}

void child_thread(void *data) {
	uv_loop_t *thread_loop = (uv_loop_t *)data;
	fprintf(stderr, "child thread will start event loop\n");

	//Start this loop
	uv_run(thread_loop, UV_RUN_DEFAULT);
}

void consumer_notify(uv_async_t *handle) {
	fprintf(stderr, "child thread recv notify from main thread\n");
}

int main(int argc, char *argv[]) {
	uv_thread_t thread;
	uv_async_t async;

	/* Create and set up the consumer thread */
	uv_loop_t *thread_loop = uv_loop_new();
	uv_async_init(thread_loop, &async, consumer_notify);
	uv_thread_create(&thread, child_thread, thread_loop);

	/* Main thread will run default loop */
	uv_loop_t *main_loop = uv_default_loop();
	uv_timer_t timer_req;
	uv_timer_init(main_loop, &timer_req);

	/* Timer callback needs async so it knows where to send messages */
	timer_req.data = &async;
	uv_timer_start(&timer_req, timer_callback, 0, 2000);

	fprintf(stderr, "Starting main loop\n");
	uv_run(main_loop, UV_RUN_DEFAULT);

	return 0;
}

//#include <iostream>
//#include <uv.h>
//
//
//class Client
//{
//public:
//	Client(int conCount) : m_connectionCount(conCount), loop(uv_default_loop()), data(1024000, '1')
//	{
//	}
//
//public:
//	void run() {
//		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
//	}
//
//	void startConn(std::string ip, int port, int index) {
//		uv_tcp_t *pSock = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
//		uv_tcp_init(loop, pSock);
//		uv_tcp_keepalive(pSock, 1, 60);
//		struct sockaddr_in dest;
//		uv_ip4_addr(ip.c_str(), port, &dest);
//		pSock->data = this;
//		uv_connect_t *req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
//		req->data = pSock;
//		auto onConnect = [](uv_connect_t* req, int status) {
//			//printf("onConect\n");
//			uv_tcp_t* psock = (uv_tcp_t*)(req->data);
//			Client* pclient = (Client*)(psock->data);
//
//			pclient->m_connections.push_back((uv_tcp_t*)psock);
//			pclient->Read((uv_stream_t*)psock);
//			pclient->write((uv_stream_t*)psock);
//
//		};
//		uv_tcp_connect(req, pSock, (sockaddr*)&dest, onConnect);
//	}
//
//	void connect(std::string ip, int port) {
//		for (int i = 0; i < m_connectionCount; ++i) {
//			startConn(ip, port, i);
//		}
//	}
//	void Read(uv_stream_t* tcp) {
//		//printf("client read socket\n");
//		auto onAlloc = [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
//		{
//			buf->base = (char*)malloc(suggested_size);
//			buf->len = suggested_size;
//		};
//
//		auto onRead = [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
//		{
//			//printf("onRead\n");
//			if (nread < 0) {
//				if (nread == UV_EOF) {
//					printf("server %s(%d)eof  initiative close\n", (int)nread);
//				}
//				else if (nread == UV_ECONNRESET) {
//					printf("server %s(%d)conn reset  passive close\n", (int)nread);
//				}
//				else {
//					printf("server %s nread = %d read error\n", (int)nread);
//				}
//				uv_close((uv_handle_t*)stream, [](uv_handle_t* handle) {
//
//				});
//				return;
//			}
//
//			std::string data(buf->base, nread);
//			free(buf->base);
//			printf("recv >>%s\n", data.c_str());
//
//		};
//
//		int error = uv_read_start((uv_stream_t*)tcp, onAlloc, onRead);
//		if (error < 0) {
//			printf("onRead Error\n");
//		}
//
//	}
//
//	void write(uv_stream_t* tcp) {
//		//printf("client write\n");
//		int size = data.size();
//		//char* buf = (char*)malloc(size);
//		uv_buf_t a[] = { uv_buf_init(&data[0], size) };
//
//		//memcpy(buf, data.c_str(), size);
//
//		uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
//		//req->data = buf;
//
//		int ret = uv_write(req, (uv_stream_t*)tcp, a, 1, [](uv_write_t* req, int status) {
//			//auto buf = (uint8_t*)(req->data);
//			//delete buf;
//			delete req;
//			//printf("Write completed\n");
//		});
//		if (ret < 0) {
//			printf("Write eorr %s %d\n", uv_err_name(ret), ret);
//		}
//
//	}
//public:
//	uv_loop_t* loop;
//	std::vector<uv_tcp_t*> m_connections;
//	uint32_t m_connectionCount;
//	std::vector<char> data;
//};
//
//int main(int argc, const char * argv[])
//{
//	Client client(5000);
//	client.connect("53.28.100.150", 8888);
//	client.run();
//	getchar();
//}

//typedef struct Conctx {
//	Conctx() { con = new uv_tcp_t; }
//	uv_tcp_t* con;
//	std::vector<char> readBuf{ decltype(readBuf)(4096, '0') };
//	std::string data{ std::string(1024 * 1000, '1') };
//} Conctx;
//
//std::vector<Conctx*> connects;
//uv_loop_t* loop;
//int main() {
//	loop = uv_default_loop();
//	struct sockaddr serverAddr = str2Addr("53.28.100.150:8888");
//	print(format("serverAddr = %s\n", addr2Str(serverAddr).c_str()));
//
//	if (uv_loop_init(loop) < 0) {
//		print(format("uv_loop_init   failed\n"));
//	}
//
//	Conctx* ctx = (Conctx*)malloc(sizeof(Conctx));
//	uv_tcp_t* con = (uv_tcp_t*)ctx;
//	if (con == nullptr) {
//		print(format("uv_tcp_t  molloc failed\n"));
//	}
//	con->data = (Conctx*)con;
//	connects.push_back(ctx);
//	if (uv_tcp_init(loop, con) < 0) {
//		print(format("uv_connect_t  molloc failed\n"), "red");
//	}
//
//	uv_connect_t* conReq = (uv_connect_t*)malloc(sizeof(uv_connect_t));
//	if (conReq == nullptr) {
//		print(format("uv_connect_t  molloc failed\n"));
//	}
//	conReq->data = con;
//	uv_tcp_connect(conReq, con, &serverAddr, [](uv_connect_t* req, int status) {
//		uv_tcp_t* tcp = (uv_tcp_t*)(req->data);
//		if (status < 0) {
//			uv_close((uv_handle_t*)tcp, [](uv_handle_t* handle) { handle = nullptr; });
//		}
//
//		uv_read_start((uv_stream_t*)tcp,
//			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
//			Conctx* ctx = (Conctx*)handle;
//			buf->base = &(ctx->readBuf[0]);
//			buf->len = 4096;
//		},
//			[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
//			if (nread < 0) {
//				if (nread == UV_EOF) {
//					fprintf(stdout, "server eof  initiative close %d\n", (int)nread);
//				}
//				else if (nread == UV_ECONNRESET) {
//					fprintf(stdout, "server reset  passive close %d\n", (int)nread);
//				}
//				else {
//					fprintf(stderr, "server read error %d\n", (int)nread);
//				}
//				uv_close((uv_handle_t*)stream, [](uv_handle_t* handle) { handle = nullptr; });
//				return;
//			}
//			std::string data(buf->base, nread);
//			print(format("recv from server >>%s\n", data.c_str()));
//		});
//		Conctx* ctx = (Conctx*)(tcp);
//		uv_write_t* wreq = (uv_write_t*)malloc(sizeof(uv_write_t));
//		uv_buf_t buf = uv_buf_init((char*)ctx->data.c_str(), ctx->data.length());
//		uv_write(wreq, (uv_stream_t*)tcp, &buf, 1, [](uv_write_t* req, int status) {
//
//		});
//	});
//	connects.push_back(ctx);
//	uv_run(loop, UV_RUN_DEFAULT);
//	getchar();
//}