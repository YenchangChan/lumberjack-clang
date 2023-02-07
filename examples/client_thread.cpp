#include "../lumberjack.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

int main() {
	lumberjack_config_t config  = {
        hosts 					: (char *)"192.168.110.8",
        port  					: 7443,
		batch 					: 200,
		with_ssl 				: true,
		protocol 				: LUMBERJACK_PROTO_VERSION_V2,
        compress_level 			: 0,
		bandwidth 				: 0,
        client_port_range 		: (char *)"60000,60100",
        timeout 				: 10,
        metric_interval 		: 5,
		metric_enable 			: true,
    };

	lumberjack_client_t *client = lumberjack_new_client(NULL, &config);

	std::cout <<"client created " << client->conn.host << ":" << client->conn.port << std::endl;
	client->start(client);
	std::cout << "client started" << std::endl;
	std::thread thd = std::thread([&]{
		while(1) {
			int ack = client->wait_and_ack(client);
			//printf("got ack: %d\n", ack);
		}
	});
	while (1)
	{
		client->metrics_report(client);
		if (client->is_connected(client))
		{
			std::string msg = "{\"@message\":\"hello world\",\"@@id\":\"8f440041835117afc302a47965be727a\",\"@filehashkey\":\"c0419d1e720f6256fd44da6dbdacf5f7\",\"@collectiontime\":\"2023-01-31T09:59:13.827+08:00\",\"@hostname\":\"ck08\",\"@path\":\"/root/chenyc/test/dc/mave/probes/itoa-flow/data/utf-8.log\",\"@rownumber\":1,\"@seq\":1,\"@topic\":\"dc_test\",\"@ip\":\"192.168.110.8\",\"@taskid\":\"5961086096158208\"}";
			int i = 0;
			for (i = 0; i < client->config->batch; i++)
			{
				if (client->push(client, (char*)msg.c_str()) == false)
				{
					std::cout << "push message failed\n" << std::endl;
					break;
				}
			}
			int n = client->send(client);
			if (n < 0)
			{
				client->stop(client);
				client = NULL;
				break;
			}
		}
	}

	if (client)
	{
		client->stop(client);
		client = NULL;
	}
	if (thd.joinable()) {
		thd.join();
	}
	return 0;
}