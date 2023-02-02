#include "../lumberjack.h"

int main(void)
{
	lumberjack_config_t config = {
		.hosts = "192.168.110.8",
		.port = 7070,
		.compress_level = 0,
		.client_port_range = "60000,60100",
		.bandwidth = 0,
		.protocol = LUMBERJACK_PROTO_VERSION_V2,
		.timeout = 10,
		.batch = 100,
		.with_ssl = false,
		.metric_interval = 5,
		.metric_enable = true,
	};
	// lumberjack_config_t config = {0};
	lumberjack_client_t *client = lumberjack_new_client(NULL, &config);
	printf("client created %s:%d \n", client->conn.host, client->conn.port);
	client->start(client);
	printf("client started\n");

	client->bootstrap(client);
	char *fmt = "{\"@message\":\"hello world, batch[%d], seq[%d]\",\"@@id\":\"8f440041835117afc302a47965be727a\",\"@filehashkey\":\"c0419d1e720f6256fd44da6dbdacf5f7\",\"@collectiontime\":\"2023-01-31T09:59:13.827+08:00\",\"@hostname\":\"ck08\",\"@path\":\"/root/chenyc/test/dc/mave/probes/itoa-flow/data/utf-8.log\",\"@rownumber\":1,\"@seq\":1,\"@topic\":\"dc_test\",\"@ip\":\"192.168.110.8\",\"@taskid\":\"5961086096158208\"}";
	int batch = 0, seq = 0;
	while (1)
	{
		char *msg = "{\"@message\":\"hello world\",\"@@id\":\"8f440041835117afc302a47965be727a\",\"@filehashkey\":\"c0419d1e720f6256fd44da6dbdacf5f7\",\"@collectiontime\":\"2023-01-31T09:59:13.827+08:00\",\"@hostname\":\"ck08\",\"@path\":\"/root/chenyc/test/dc/mave/probes/itoa-flow/data/utf-8.log\",\"@rownumber\":1,\"@seq\":1,\"@topic\":\"dc_test\",\"@ip\":\"192.168.110.8\",\"@taskid\":\"5961086096158208\"}";
		batch++;
		while (client->push(client, msg))
			;
		// printf("batch = %d\n", batch);
		if (client->get_status(client) == LJ_STATUS_ACK_MISMATCH)
		{
			// ack mismatched, should resend last batch
			batch--;
		}
	}

	if (client)
	{
		client->stop(client);
		client = NULL;
	}
	return 0;
}
