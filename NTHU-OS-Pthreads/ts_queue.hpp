#include <pthread.h>

#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#define DEFAULT_BUFFER_SIZE 200

template <class T>
class TSQueue {
public:
	// constructor
	TSQueue();

	explicit TSQueue(int max_buffer_size);

	// destructor
	~TSQueue();

	// add an element to the end of the queue
	void enqueue(T item);

	// remove and return the first element of the queue
	T dequeue();

	// return the number of elements in the queue
	int get_size();
private:
	// the maximum buffer size
	int buffer_size;
	// the buffer containing values of the queue
	T* buffer;
	// the current size of the buffer
	int size;
	// the index of first item in the queue
	int head;
	// the index of last item in the queue
	int tail;

	// pthread mutex lock
	pthread_mutex_t mutex;
	// pthread conditional variable: enqueue when not full, dequeue when not empty
	pthread_cond_t cond_enqueue, cond_dequeue;
};

// Implementation start

template <class T>
TSQueue<T>::TSQueue() : TSQueue(DEFAULT_BUFFER_SIZE) {
}

template <class T>
TSQueue<T>::TSQueue(int buffer_size) : buffer_size(buffer_size) {
	// TODO: implements TSQueue constructor
	buffer = new T[buffer_size];
	pthread_mutex_init(&mutex, nullptr);
	pthread_cond_init(&cond_dequeue, nullptr);
	pthread_cond_init(&cond_enqueue, nullptr);
}

template <class T>
TSQueue<T>::~TSQueue() {
	// TODO: implenents TSQueue destructor
	delete[] buffer;
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_dequeue);
	pthread_cond_destroy(&cond_enqueue);
}

template <class T>
void TSQueue<T>::enqueue(T item) {
	// TODO: enqueues an element to the end of the queue
	pthread_mutex_lock(&mutex);
	while(size == buffer_size) {
		pthread_cond_wait(&cond_enqueue, &mutex);
	}
	buffer[tail] = item;
	tail = (tail + 1) % buffer_size;
	size++;
	pthread_cond_signal(&cond_dequeue);
	pthread_mutex_unlock(&mutex);
}

template <class T>
T TSQueue<T>::dequeue() {
	// TODO: dequeues the first element of the queue
	pthread_mutex_lock(&mutex);
	while(size == 0) {
		pthread_cond_wait(&cond_dequeue, &mutex);
	}
	T item = buffer[head];
	head = (head + 1) % buffer_size;
	size--;
	pthread_cond_signal(&cond_enqueue);
	pthread_mutex_unlock(&mutex);
	return item;
}

template <class T>
int TSQueue<T>::get_size() {
	// TODO: returns the size of the queue
	pthread_mutex_lock(&mutex);
	int s = size;
	pthread_mutex_unlock(&mutex);
	return s;
}

#endif // TS_QUEUE_HPP
