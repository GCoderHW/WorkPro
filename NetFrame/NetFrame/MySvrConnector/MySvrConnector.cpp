/************C++ Source File****************
#
#	Filename: MySvrConnector.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-07-31 10:47:39
#	Last Modified: 2018-07-31 10:47:39
*******************************************/



#include "MySvrConnector.h"

MySvrConnector::MySvrConnector(int nMaxConnect)
{
	m_listen_socket = 0;
	m_cur_maxfds = 0;
	m_max_connect = nMaxConnect;
	memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	memset(m_recv_buf, 0, sizeof(m_recv_buf));
	memset(m_send_buf, 0, sizeof(m_send_buf));
	client_socket_map.clear();	
	FD_ZERO(&m_rfds);
	FD_ZERO(&m_sfds);
	FD_ZERO(&m_efds);

#ifdef _WIN32

	static WSADATA wsa;
	if(0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("initialize windows socket failed!\n");
		exit(0);
	}
	
#endif
}

MySvrConnector::~MySvrConnector()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void MySvrConnector::run()
{
	if(0 >= Socket())
	{
		printf("create socket failed!\n");		
		return;
	}

	if(0 != Bind())
	{
		printf("bind socket failed!\n");
		return;
	}

	if(0 != Listen())
	{
		printf("listen socket failed!\n");
		return;
	}
	
	while(true)
	{
		fd_set rfds_cp = m_rfds;
		fd_set sfds_cp = m_sfds;
		fd_set efds_cp = m_efds;

		int max_nfds = 0;
#ifndef _WIN32
		max_nfds = client_socket_map.size() + 1 + 1;	//多一个Listen socket
#endif // _WIN32

		int n = select(max_nfds, &rfds_cp, NULL, &efds_cp, NULL);			//问题  客户端发送的数据在accept之前

		if (0 > n)
		{
			printf("select call error!\n");
			return;
			//printf("select result:%d\n", WSAGetLastError());
		}

		if (0 == n)
			continue;

		int cur_count = 0;
		
		if (FD_ISSET(m_listen_socket, &rfds_cp))
		{
			Accept();
			++cur_count;
			if (n <= cur_count)
				continue;
		}
		
		for (std::map<unsigned int, socket_info>::const_iterator it = client_socket_map.begin(); it != client_socket_map.end(); ++it)
		{
			
			if (FD_ISSET(it->first, &rfds_cp))
			{
				Recv(it->first);
				++cur_count;
				if (n <= cur_count)
					break;
			}
			
			/*
			if (FD_ISSET(it->first, &sfds_cp))
			{
				Send(it->first);
				++cur_count;
				if (n <= cur_count)
					break;
			}
			*/

	
			if (FD_ISSET(it->first, &efds_cp))
			{
				Except(it->first);
				++cur_count;
				if (n <= cur_count)
					break;
			}
		}
	}
	
	
}


int MySvrConnector::Socket()
{
	m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	return m_listen_socket;
}

int MySvrConnector::Bind()
{
	if(0 >= m_listen_socket)
		return -1;
	
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(6666);
	inet_pton(AF_INET, INADDR_ANY, (void*)&m_sockaddr.sin_addr);

	if(0 != bind(m_listen_socket, (sockaddr*)&m_sockaddr, sizeof(m_sockaddr)))
		return -1;

	return 0;
}


int MySvrConnector::Listen()
{
	if (0 != listen(m_listen_socket, 5))
		return -1;
	FD_SET(m_listen_socket, &m_rfds);
	
	printf("listen sucess!\nbegin listen port:%d, listen fd:%d\n", ntohs(m_sockaddr.sin_port), m_listen_socket);

	return 0;
}

int MySvrConnector::Accept()
{
	sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	memset(&clientAddr, 0, sizeof(clientAddr));
	int nfd = accept(m_listen_socket, (sockaddr*)&clientAddr, &len);
	if(0 >= nfd)
		return -1;

	if(client_socket_map.size() >= m_max_connect)
		return -1;

	socket_info new_socket_info;
	memset(&new_socket_info, 0, sizeof(new_socket_info));

	new_socket_info.socket = nfd;
	memcpy(&new_socket_info.socket_addr, &clientAddr, sizeof(new_socket_info.socket_addr));			//这里最好提供memcpy_s函数
	inet_ntop(AF_INET, &clientAddr.sin_addr, new_socket_info.ip, sizeof(new_socket_info.ip));
	new_socket_info.port = ntohs(clientAddr.sin_port);
	
	client_socket_map.insert(std::make_pair(new_socket_info.socket, new_socket_info));

	printf("Accept from ip:%s, port:%d, fd:%d\n", new_socket_info.ip, new_socket_info.port, new_socket_info.socket);

	FD_SET(nfd, &m_rfds);
	FD_SET(nfd, &m_sfds);
	FD_SET(nfd, &m_efds);

	return 0;
}

int MySvrConnector::Recv(unsigned int nfd)
{
	std::map<unsigned int, socket_info>::const_iterator it_fd = client_socket_map.find(nfd);
	if (it_fd == client_socket_map.end())
	{
		Close(nfd);
		return -1;
	}

	int n = recv(nfd, m_recv_buf, sizeof(m_recv_buf), 0);
	if (0 >= n)
	{
		printf("recv client socket[%d] close msg!\n", nfd);
		Close(nfd);
		return -1;
	}
	printf("recv data[%s],from ip:%s,port:%d, fd:%d\n", m_recv_buf, it_fd->second.ip, it_fd->second.port, it_fd->first);

	n = send(nfd, m_recv_buf, strlen(m_recv_buf), 0);
	printf("send data to socket[%d] len:%d\n", nfd, n);
	
	memset(m_recv_buf, 0, sizeof(m_recv_buf));
	return 0;
}

int MySvrConnector::Send(unsigned int nfd)
{
	std::map<unsigned int, socket_info>::const_iterator it_fd = client_socket_map.find(nfd);
	if (it_fd == client_socket_map.end())
	{
		Close(nfd);
		return -1;
	}
	printf("send data[%s],to ip:%s,port:%d, fd:%d\n", m_recv_buf, it_fd->second.ip, it_fd->second.port, it_fd->first);

	return 0;
}

int MySvrConnector::Except(unsigned int nfd)
{
	std::map<unsigned int, socket_info>::const_iterator it_fd = client_socket_map.find(nfd);
	if (it_fd == client_socket_map.end())
	{
		Close(nfd);
		return -1;
	}
	printf("send data[%s],to ip:%s,port:%d, fd:%d\n", m_recv_buf, it_fd->second.ip, it_fd->second.port, it_fd->first);

	return 0;
}
