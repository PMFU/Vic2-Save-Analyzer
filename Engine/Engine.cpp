#define IMGUI_USER_CONFIG "my_imgui_config.h"
#include "Engine.h"

JobSystem jobSystem;

//@41 mod enjoyer 112 St NW, Edmonton, AB T6J 5B1

//Enet testing stuff
/*
#include <enet/enet.h>

void EnetTest()
{
	enet_initialize();
	
	ENetAddress addr;
	ENetHost*	client;
	ENetHost*	server;

	addr.host = ENET_HOST_ANY;
	addr.port = 12345;

	server = enet_host_create(&addr, 32, 2, 0, 0);

	ENetEvent event;

	// Recieve data
	while(enet_host_service(server, &event, 15) > 0)
	{
		switch(event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				// event.peer->data = "Client info";
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				event.peer->data = nullptr;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_NONE:
				//
				break;
		}
	}

		//Send packet
	ENetPacket* packet = enet_packet_create("packet", strlen("packet") + 1, ENET_PACKET_FLAG_RELIABLE);

	ENetPeer* peersend;
	peersend = &server->peers[0];
	
	// enet_packet_resize (packet, strlen ("packetfoo") + 1);
	// strcpy (& packet -> data [strlen ("packet")], "foo");

	enet_peer_send(peersend, 0, packet);



	//Client
	client = enet_host_create(NULL, 1, 2, 0, 0);
	ENetAddress serveraddress;
	ENetPeer* serverpeer;
	// enet_address_get_host(&serveraddress, )
	enet_address_set_host(&serveraddress, "domainname.com");
	// serveraddress.host = 192.xx.xx.xx //this is if manual IP, not dns
	serveraddress.port = 12345;

	serverpeer = enet_host_connect(client, &serveraddress, 2, 0);

	if(enet_host_service(client, &event, 5000) > 0)	//5 seconds
	{
		if(event.type == ENET_EVENT_TYPE_CONNECT)
		{
			//Connection success
		}
		else
		{
			//not successful
		}
	}

		//Client send packet
	//2nd arg is channel id, this can be seen on both server and client, so would be good to utilize them for different things
	// i.e channel 1 is checksum sending, 2 is action data, 3 is others, 4 is ...
	enet_peer_send(serverpeer, 0, packet);


	// For sending the queued packets:
	// enet_host_service(server, &event, 5000);

	//Clean up
	enet_host_destroy(server);

	enet_deinitialize();

}
*/