#include <uv.h>
#include <string>
#include <vector>
#include "util.h"
#include <stdio.h>
#include <stdint.h>
#include <uv.h>
#include <assert.h>
#define UV_THREADPOOL_SIZE 64


#include <iostream>
#include <uv.h>
#define CONTAINER_OF(ptr, type, field)                                        \
  ((type *) ((char *) (ptr) - ((char *) &((type *) 0)->field)))


typedef struct ConCtx
{
	uv_tcp_t sock;
	uv_connect_t conReq;
	uv_write_t writeReq;
	uv_buf_t buf;
	uint64 inBytes;
	uint64 outBytes;
	int index;
}ConCtx;

class Client
{
public:
	Client(int conCount, uint32_t statisTime) : m_connectionCount(conCount), loop(uv_default_loop()), data(1024, '1'), statisTime(statisTime)
	{
		timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
		uv_timer_init(loop, timer);
	}

public:
	void run() {
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	}
	void startConn() {
		ConCtx* ctx = (ConCtx*)calloc(1, sizeof(ConCtx));
		ctx->inBytes = 0;
		ctx->outBytes = 0;
		//uv_tcp_t *pSock = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
		uv_tcp_t *pSock = (uv_tcp_t*)(&ctx->sock);
		uv_tcp_init(loop, pSock);
		uv_tcp_keepalive(pSock, 1, 60);
		struct sockaddr_in dest;
		uv_ip4_addr(ip.c_str(), port, &dest);
		pSock->data = this;
		//uv_connect_t *req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
		uv_connect_t* req = (uv_connect_t*)(&ctx->conReq);
		req->data = pSock;
		auto onConnect = [](uv_connect_t* req, int status) {
			uv_tcp_t* psock = (uv_tcp_t*)(req->data);
			Client* pclient = (Client*)(psock->data);
			ConCtx* ctx = (ConCtx*)CONTAINER_OF((uv_tcp_t*)psock, ConCtx, sock);
			if (status == 0) {
				//printf("onConect\n");
				ctx->index = pclient->m_connections.size();
				pclient->m_connections.push_back((uv_tcp_t*)psock);
				//printf("connect %d\n", ctx->index);
				pclient->Read((uv_stream_t*)psock);
				//pclient->Read((uv_stream_t*)psock);
				//pclient->write((uv_stream_t*)psock);
				if (pclient->m_connections.size() == pclient->m_connectionCount) {
					//printf("start send data\n");
					auto onTimer = [](uv_timer_t* handle) {
						uv_timer_stop(handle);
						Client* pclient = (Client*)handle->data;
						for (int i = 0; i < pclient->m_connections.size(); ++i) {
							uv_tcp_t* tcp = pclient->m_connections[i];
							//printf(".");
							//If timeout is zero, the callback fires on the next event loop iteration. 
							//If repeat is non-zero, the callback fires first
							//after timeout milliseconds and then repeatedly after repeat milliseconds.
							if (uv_is_writable((uv_stream_t*)tcp) == 1) {
								pclient->write((uv_stream_t*)tcp);
							}

						}

					};
					pclient->timer->data = pclient;
					if (uv_is_closing((uv_handle_t*)pclient->timer) == 0) {
						uv_timer_start(pclient->timer, onTimer, 2000, 2000);
					}
				}
			}
			else {
				//printf("create conection error %s", uv_err_name(status));
				pclient->startConn();
			}
		};
		assert(0 == uv_tcp_connect(req, pSock, (sockaddr*)&dest, onConnect));
	}

	void connect(std::string ip, int port) {
		this->ip = ip;
		this->port = port;
		for (int i = 0; i < m_connectionCount; ++i) {
			startConn();
		}
	}
	void Read(uv_stream_t* tcp) {
		//printf("client read socket\n");
		auto onAlloc = [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
		{
			buf->base = (char*)malloc(suggested_size);
			buf->len = suggested_size;
		};

		auto onRead = [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
		{
			//printf("onRead\n");
			if (nread < 0) {
				if (nread == UV_EOF) {
					//printf("server (%d)eof  initiative close\n", (int)nread);
				}
				else if (nread == UV_ECONNRESET) {
					//printf("server (%d)conn reset  passive close\n", (int)nread);
				}
				else {
					//printf("server nread = %d read error\n", (int)nread);
				}
				uv_close((uv_handle_t*)stream, [](uv_handle_t* handle) {

				});
				return;
			}

			//std::string data(buf->base, nread);
			ConCtx* ctx = (ConCtx*)CONTAINER_OF((uv_tcp_t*)stream, ConCtx, sock);
			ctx->inBytes += nread;
			//printf("connection %d recvBytes = %d\n", ctx->index, ctx->inBytes);
			//Client* pclient = (Client*)(stream->data);
			//pclient->write(stream);
			free(buf->base);

		};

		assert(0 == uv_read_start((uv_stream_t*)tcp, onAlloc, onRead));

	}

	void write(uv_stream_t* tcp) {
		ConCtx* ctx = (ConCtx*)CONTAINER_OF((uv_tcp_t*)tcp, ConCtx, sock);
		//printf("client write\n");
		int size = data.size();
		//printf("Write dataSize =  %d\n", size);
		//char* buf = (char*)malloc(size);
		ctx->buf = uv_buf_init(&data[0], size);

		//memcpy(buf, data.c_str(), size);

		//uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
		uv_write_t* req = (uv_write_t*)(&ctx->writeReq);
		req->data = &ctx->buf;

		assert(0 == uv_write(req, (uv_stream_t*)tcp, &ctx->buf, 1, [](uv_write_t* req, int status) {

			if (status == 0) {
				//auto buf = (uint8_t*)(req->data);
			//delete buf;
				uv_buf_t* buf = (uv_buf_t*)req->data;
				//printf("write cb buf->len = %d\n", buf->len);
				ConCtx* ctx = (ConCtx*)CONTAINER_OF((uv_write_t*)req, ConCtx, writeReq);
				ctx->outBytes += buf->len;
				//printf("connection %d writeByte =%d\n", ctx->index, ctx->outBytes);
				//printf("Write completed\n");
				//free(req);
			}

		}));
	}


	void shutDown() {
		stopAsync.data = this;
		uv_async_init(loop, &stopAsync, [](uv_async_t *handle) {
			auto self = (Client *)(handle->data);
			auto timer = self->timer;
			uv_timer_stop(timer);
			timer->data = self;
			uv_close((uv_handle_t *)timer, [](uv_handle_t *handle) {
				auto self = (Client *)(handle->data);
				int size = self->m_connections.size();
				//printf("connection count =%d\n", size);
				for (int i = 0; i < size; ++i) {
					auto tcp = self->m_connections[i];
					auto loop = self->loop;
					ConCtx* ctx = CONTAINER_OF(tcp, ConCtx, sock);
					Client* pc = (Client*)tcp->data;
					double inBytes = ctx->inBytes / 1024;
					double outBytes = ctx->outBytes / 1024;
					double inMps = inBytes / pc->statisTime;
					double outMps = outBytes / pc->statisTime;
					printf("sock index %d inBytes = %fk,  inMps = %fk/s, outBytes = %fk outMps = %fk/s\n", ctx->index, inBytes, inMps, outBytes, outMps);
				}
				uv_walk(self->loop,
					[](uv_handle_t *handle, void *arg) {
					if (uv_is_closing(handle) == 0) {
						//printf("close loop handles\n");
						uv_close(handle, [](uv_handle_t *h) {});
					}
				},
					nullptr);
				uv_stop(self->loop);
			});
		});
		uv_async_send(&stopAsync);
	}
public:
	uv_loop_t* loop;
	uv_async_t  stopAsync;
	uv_timer_t* timer;
	std::vector<uv_tcp_t*> m_connections;
	int m_connectionCount;
	int statisTime;
	std::vector<char> data;
	std::string ip;
	int port;
};

int main(int argc, const char * argv[])
{
	int runTimeSecond = 120;
	int connectCount = 10;
	std::string ip = "53.28.100.150";
	int port = 8888;
	Client client(connectCount, runTimeSecond);
	client.connect(ip, port);
	uv_timer_t stopTimer;
	stopTimer.data = &client;
	uv_timer_init(uv_default_loop(), &stopTimer);
	uv_timer_start(&stopTimer, [](uv_timer_t* handle) {
		Client* client = (Client*)handle->data;
		client->shutDown();
	}, runTimeSecond * 1000, 1);

	client.run();
	printf("\n");
	getchar();
}


///* Libuv multiple loops + thread communication example. Written by Kristian
// * Evensen <kristian.evensen@gmail.com>  */
//
// //rpath is needed because it is an argument to the linker (not compiler) about
// //where to look
// //gcc -Iinclude -g libuv-test.c -o libuv_test -L "./" -l uv -Xlinker -rpath -Xlinker "./" -lrt
//
//
//void timer_callback(uv_timer_t *handle) {
//	uv_async_t *other_thread_notifier = (uv_async_t *)handle->data;
//
//	fprintf(stderr, "main thread Timer expired, notifying other thread\n");
//
//	//Notify the other thread
//	uv_async_send(other_thread_notifier);
//}
//
//void child_thread(void *data) {
//	uv_loop_t *thread_loop = (uv_loop_t *)data;
//	fprintf(stderr, "child thread will start event loop\n");
//
//	//Start this loop
//	uv_run(thread_loop, UV_RUN_DEFAULT);
//}
//
//void consumer_notify(uv_async_t *handle) {
//	fprintf(stderr, "child thread recv notify from main thread\n");
//}
//
//int main(int argc, char *argv[]) {
//	uv_thread_t thread;
//	uv_async_t async;
//
//	/* Create and set up the consumer thread */
//	uv_loop_t *thread_loop = uv_loop_new();
//	uv_async_init(thread_loop, &async, consumer_notify);
//	uv_thread_create(&thread, child_thread, thread_loop);
//
//	/* Main thread will run default loop */
//	uv_loop_t *main_loop = uv_default_loop();
//	uv_timer_t timer_req;
//	uv_timer_init(main_loop, &timer_req);
//
//	/* Timer callback needs async so it knows where to send messages */
//	timer_req.data = &async;
//	uv_timer_start(&timer_req, timer_callback, 0, 2000);
//
//	fprintf(stderr, "Starting main loop\n");
//	uv_run(main_loop, UV_RUN_DEFAULT);
//
//	return 0;
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