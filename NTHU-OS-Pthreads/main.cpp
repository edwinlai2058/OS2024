#include <assert.h>
#include <stdlib.h>
#include "ts_queue.hpp"
#include "item.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "producer.hpp"
#include "consumer_controller.hpp"

#define READER_QUEUE_SIZE 200
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000

int main(int argc, char** argv) {
	assert(argc == 4);

	int n = atoi(argv[1]);
	std::string input_file_name(argv[2]);
	std::string output_file_name(argv[3]);

	// TODO: implements main function
	// 1. Create queues
	TSQueue<Item*> reader_queue(READER_QUEUE_SIZE);
	TSQueue<Item*> worker_queue(WORKER_QUEUE_SIZE);
	TSQueue<Item*> writer_queue(WRITER_QUEUE_SIZE);

	// 2. Create transformer
	Transformer transformer;

	// 3. Create threads
	Reader reader(n, input_file_name, &reader_queue);
	Writer writer(n, output_file_name, &writer_queue);

	std::vector<Producer*> producers;
	for(int i = 0; i < 4; i++) {
		producers.push_back(new Producer(&reader_queue, &worker_queue, &transformer));
	}

	int lt = WORKER_QUEUE_SIZE * CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE / 100;
	int ht = WORKER_QUEUE_SIZE * CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE / 100;
	ConsumerController consumer_controller(&worker_queue, &writer_queue, &transformer, CONSUMER_CONTROLLER_CHECK_PERIOD, lt, ht);

	// 4. Start threads
	reader.start();
	writer.start();
	for(auto producer : producers) {
		producer->start();
	}
	consumer_controller.start();

	// 5. Join threads
	reader.join();
	writer.join();

	for(auto producer : producers) {
		delete producer;
	}

	return 0;
}
