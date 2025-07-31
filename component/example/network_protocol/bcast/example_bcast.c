#include "platform_stdlib.h"
#include "basic_types.h"

#include "wifi_api.h"
#include "lwip_netconf.h"


static void example_bcast_thread(void *param)
{
	/* To avoid gcc warnings */
	(void) param;

	// Delay to check successful WiFi connection and obtain of an IP address
	LwIP_Check_Connectivity();

	RTK_LOGS(NOTAG, RTK_LOG_INFO, "\nExample: bcast \n");

	int socket = -1;
	int broadcast = 1;
	struct sockaddr_in bindAddr;
	uint16_t port = 49152;
	unsigned char packet[1024];
	char tgtTcpSrvIpAddr[16] = {0};

	// Create socket
	if ((socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: socket failed\n");
		goto err;
	}

	// Set broadcast socket option
	if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: setsockopt failed\n");
		goto err;
	}

	// Set the bind address
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(socket, (struct sockaddr *) &bindAddr, sizeof(bindAddr)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: bind failed\n");
		goto err;
	}


	while (1) {
		// Receive broadcast
		int packetLen;
		struct sockaddr from;
		struct sockaddr_in *from_sin = (struct sockaddr_in *) &from;
		socklen_t fromLen = sizeof(from);
		memset(packet, 0x00, sizeof(packet));
		if ((packetLen = recvfrom(socket, &packet, sizeof(packet), 0, &from, &fromLen)) >= 0) {
			uint8_t *ip = (uint8_t *) &from_sin->sin_addr.s_addr;
			uint16_t from_port = ntohs(from_sin->sin_port);
			sprintf(tgtTcpSrvIpAddr, "%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
			RTK_LOGS(NOTAG, RTK_LOG_INFO, "recvfrom - %d bytes from %d.%d.%d.%d:%d\n", packetLen, ip[0], ip[1], ip[2], ip[3], from_port);
			RTK_LOGS(NOTAG, RTK_LOG_INFO, "Message: %s\n", packet);
		}

		// Send broadcast
		if (packetLen > 0) {
			int sendLen;
			struct sockaddr to;
			struct sockaddr_in *to_sin = (struct sockaddr_in *) &to;
			to_sin->sin_family = AF_INET;
			to_sin->sin_port = htons(11111);
			to_sin->sin_addr.s_addr = INADDR_BROADCAST;
			strcat((char *)packet, " - broadcast");
			packetLen = strlen((char *)packet);

			if ((sendLen = sendto(socket, packet, (packetLen <= 1024) ? packetLen : 1024, 0, &to, sizeof(struct sockaddr))) < 0) {
				RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: sendto broadcast\n");
			} else {
				RTK_LOGS(NOTAG, RTK_LOG_INFO, "sendto - %d bytes to broadcast:%d\n", sendLen, port);
				break;
			}
		}

		
	}

	struct sockaddr_in server_addr;
	int tcp_srv_socket = -1;
	int keepalive = 1, keepalive_idle = 3, keepalive_interval = 5, keepalive_count = 3;
	bool testFinish = false;
	RTK_LOGS(NOTAG, RTK_LOG_INFO, "Prepare connecting to TCP server...\n");
	if ((tcp_srv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: tcp server socket failed\n");
		goto err;
	}
	// enable socket keepalive with keepalive timeout = idle(3) + interval(5) * count(3) = 18 seconds
	if (setsockopt(tcp_srv_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: SO_KEEPALIVE\n");
	}
	if (setsockopt(tcp_srv_socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: TCP_KEEPIDLE\n");
	}
	if (setsockopt(tcp_srv_socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: TCP_KEEPINTVL\n");
	}
	if (setsockopt(tcp_srv_socket, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: TCP_KEEPCNT\n");
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(tgtTcpSrvIpAddr);
	server_addr.sin_port = htons(11112);
	
	if (connect(tcp_srv_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
		unsigned char response_buf[100];
		int read_size;
		RTK_LOGS(NOTAG, RTK_LOG_INFO, "connect OK\n");
		while (!testFinish) {
			if (write(tcp_srv_socket, "TCP Test Message\n", 16) > 0) {
				memset(response_buf, 0, sizeof(response_buf));
				if ((read_size = read(tcp_srv_socket, response_buf, sizeof(response_buf))) > 0) {
					RTK_LOGS(NOTAG, RTK_LOG_INFO, "read %d bytes\n", read_size);
					RTK_LOGS(NOTAG, RTK_LOG_INFO, "recv msg: %s\n", response_buf);
					if (strcmp((char *)response_buf, "end") == 0) {
						RTK_LOGS(NOTAG, RTK_LOG_ERROR, "End TCP client test");
						close(tcp_srv_socket);
						testFinish = true;
					}
				}
			}
			
			// RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: read %d\n", read_size);
			// close(tcp_srv_socket);
		}
	} else {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: connect\n");
		close(tcp_srv_socket);
	}

	RTK_LOGS(NOTAG, RTK_LOG_INFO, "Setting up TCP connection...\n");
	// Prepare TCP socket for accepting connection
	int tcp_socket = -1;
	int tcp_client_fd = -1;
	int ret = 0;
	int tcp_packet_len = 0;
	struct sockaddr_in client_addr;
	size_t client_addr_size;

	// Create socket
	if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: tcp socket failed\n");
		goto err;
	}

	// Set broadcast socket option
	// if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
	// 	RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: setsockopt failed\n");
	// 	goto err;
	// }

	// Set the bind address
	// memset(&bindAddr, 0, sizeof(bindAddr));
	// bindAddr.sin_family = AF_INET;
	// bindAddr.sin_port = htons(port);
	// bindAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(tcp_socket, (struct sockaddr *) &bindAddr, sizeof(bindAddr)) < 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: bind failed\n");
		goto err;
	}

	if (listen(tcp_socket, 2) != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: listen\n");
		goto err;
	}

	while (1) {

		client_addr_size = sizeof(client_addr);
		tcp_client_fd = accept(tcp_socket, (struct sockaddr *) &client_addr, &client_addr_size);

		if (tcp_client_fd >= 0) {
			uint8_t *ip = (uint8_t *) &client_addr.sin_addr.s_addr;
			uint16_t from_port = ntohs(client_addr.sin_port);
			RTK_LOGS(NOTAG, RTK_LOG_INFO, "Conn est from %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], from_port);
			while (1) {
				memset(packet, 0x00, sizeof(packet));
				ret = recv(tcp_client_fd, packet, sizeof(packet), MSG_DONTWAIT);
				if (ret > 0) {
					RTK_LOGS(NOTAG, RTK_LOG_INFO, "TCP Message recv: %s\n", packet);
					strcat((char *)packet, " - TCP Reply");
					tcp_packet_len = strlen((char *)packet);
					ret = send(tcp_client_fd, packet, tcp_packet_len, 0);
					RTK_LOGS(NOTAG, RTK_LOG_INFO, "TCP Message sent\n");
				} else if (ret < 0) {
					RTK_LOGS(NOTAG, RTK_LOG_INFO, "TCP Connection Lost\n");
					break;
				}
 			}

		}
	}


err:
	RTK_LOGS(NOTAG, RTK_LOG_ERROR, "ERROR: broadcast example failed\n");
	close(socket);
	rtos_task_delete(NULL);
	return;
}

void example_bcast(void)
{
	if (rtos_task_create(NULL, ((const char *)"example_bcast_thread"), example_bcast_thread, NULL, 2048 * 4, 1) != RTK_SUCCESS) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "\n\r%s rtos_task_create(init_thread) failed", __FUNCTION__);
	}
}
