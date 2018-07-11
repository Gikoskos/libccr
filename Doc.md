# libccr documentation

## macro API

* `CCR_DECLARE(label)`

	Declare a region named `label`. Should be called only outside functions (in global scope).

* `CCR_INIT(label)`

	Initialize the region that was declared with the name `label`. Should be called only within functions.

* `CCR_EXEC(label, cond, body)`

	Execute the code in the critical section `body`, under the condition `cond`, within the region `label`. To execute `body`, `cond` must be true within the region `label`, otherwise the thread will block.


## library API

* `typedef struct ccr_s ccr_s`

	Opaque type that represents a CCR object.

* `typedef int (*condition_func)(void *param)`

	Function pointer type that represents a condition function. Returns 1 if the condition is true, 0 if it's false.

* `typedef void (*cs_body_func)(void *param)`

	Function pointer type that represents the body of code inside the critical section.

* `int ccr_init(ccr_s **ccr)`

	Allocate the memory for a `ccr_s` object and initialize it. Returns 0 on success and an errno error code on failure (doesn't set the global errno):

```c
ccr_s *my_region;
int err;

err = ccr_init(&my_region);

if (err) {
	printf("ccr_init failed with error code %d\n", err);
	exit(1);
}
```

* `int ccr_exec(ccr_s *ccr, condition_func cond, void cond_param, cs_body_func body, void *body_param)`

	Execute the function, pointed to by `body`, when the function, pointed to by `cond`, returns 1, within the CCR object `ccr`. As long as `cond` returns 0, the thread blocks. `cond_param` and `body_param` are optional parameters that `ccr_exec` passes on the `cond` and `body` functions respectively.

* `void ccr_destroy(ccr_s *ccr)`

	Releases the memory of the `ccr` object. It's not necessary to call this function, but if you do, make sure the `ccr` object isn't used by any threads.


## macro usage tutorial

To use the macro API, you simply need to include `ccr.h` in your project.

Define the macro `CCR_MACRO_LIB` with the value 1 and include `ccr.h` below that, to let the header file know you're using the macro API

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"

int main(void)
{
	return 0;
}
```

___

Create a new region by calling `CCR_DECLARE`. `CCR_DECLARE`'s only argument is the name of your region.  `CCR_DECLARE` should only be called in global scope (outside of any function). Don't declare more than one regions of the same name within the same scope.

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"

CCR_DECLARE(my_region)

int main(void)
{
	return 0;
}
```

___

Call `CCR_INIT` within a function to initialize the region you declared

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"

CCR_DECLARE(my_region)

int main(void)
{
	CCR_INIT(my_region)
	return 0;
}
```

You are now ready to use the region `my_region` from within any threads.

___

Let's create some threads with pthreads

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"

#define NUM_OF_THREADS 10

CCR_DECLARE(my_region)


void *thread_func(void *args)
{
	return NULL;
}

int main(void)
{
	CCR_INIT(my_region)

	pthread_t threads[NUM_OF_THREADS];

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_create(&threads[i], NULL, thread_func, NULL);

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
```

In this program, the main thread simply creates 10 threads, and waits until each of the threads is finished, before it can terminate.

##### Note:

Error checking is ommited for simplicity. However the CCR macros do their own error checking on the pthreads functions, and will produce a descriptive error message, before terminating the program, in case of an error.

##### Note:

Don't forget to initialize your region by calling `CCR_INIT`, before creating any threads that use it.

___

At its simplest, a CCR can be used as a mutex, with a condition that is always true:

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"

#define NUM_OF_THREADS 10

CCR_DECLARE(my_region)

int counter;

void *thread_func(void *args)
{
	CCR_EXEC(
		 /* name of the region */
		 my_region,

		 /* condition */
		 1,

		 /* critical section */
		 {
		  counter + 1;
		 }
		)
	return NULL;
}

int main(void)
{
	CCR_INIT(my_region)

	pthread_t threads[NUM_OF_THREADS];

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_create(&threads[i], NULL, thread_func, NULL);

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
```

In this example, the condition is 1 (always true in C), so the region will operate just like a mutex. A thread will block while trying to enter the critical section, only if another thread is in the critical section at the same moment.

___

Let's see a slightly more complicated example. This is what, a simplified version of the producer/consumer problem looks like, when we use CCRs

```c
#define CCR_MACRO_LIB 1
#include "ccr.h"


#define NUM_OF_THREADS   10
#define BUFFER_CAPACITY   3


CCR_DECLARE(buffer_region)


char buffer[BUFFER_CAPACITY];
int total_items = 0;


void *producer_thread_func(void *args)
{
    /* producing the data */
	char new_data = rand();

	CCR_EXEC(
	         buffer_region,

		 total_items < BUFFER_CAPACITY,

		 {
		   buffer[total_items++] = new_data;
		   printf("Producer put %d\n", (int)new_data);
		 }
		)

	return NULL;
}

void *consumer_thread_func(void *args)
{
	char read_data;

	CCR_EXEC(
	         buffer_region,

		 total_items > 0,

		 {
		   read_data = buffer[total_items--];
		 }
		)

    /* consuming the data */
    printf("Consumer read %d\n", (int)read_data);

	return NULL;
}

int main(void)
{
	CCR_INIT(my_region)

	pthread_t threads[NUM_OF_THREADS];

	for (int i = 0; i < NUM_OF_THREADS; i++) {
		if (i < 5) {
			pthread_create(&threads[i], NULL, consumer_thread_func, NULL);
		} else {
			pthread_create(&threads[i], NULL, producer_thread_func, NULL);
		}
	}

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
```

In this program, we create a region, 5 consumer threads and 5 producer threads.

Each producer thread "produces" some data (in this case a random character)

```c
   	char new_data = rand();
```

and then tries to put it in the buffer, using the region. If the buffer is full, the producer waits until there's available positions, before it can insert the produced data in the buffer.

```c
	CCR_EXEC(
	         buffer_region,

		 total_items < BUFFER_CAPACITY,

		 {
		   buffer[total_items++] = new_data;
		 }
		)
```

So the producer thread blocks, as long as there's not enough space in the buffer to put his data in. 

The consumer thread, on the other hand, tries to read data from the buffer and "consume" it somehow. If the buffer is empty, the consumer will wait until there's at least one item to get.

```c
	char read_data;

	CCR_EXEC(
	         buffer_region,

		 total_items > 0,

		 {
		   read_data = buffer[total_items--];
		 }
		)
```


## library usage tutorial

To build the static library `libccr` either use `cmake` or `make` (only tested on Linux). With `cmake` create a `build` folder within the root of the repo, and run

`cmake .. && make`

on Linux. On Windows you can use mingw32 (with libwinpthread) and call

`cmake .. -G "MinGW Makefiles" & mingw32-make`

from within the `build` folder.

If you want to install the library and its headers, in system folders (eg under `Program Files` on Windows and `/usr/local/lib` and `/usr/local/include` on Linux) then run

`make install`

or

`mingw32-make install`

___

To use the library after installing it, link your source code to the static library (with `-lccr` usually) and pthreads (with `-pthread`), and include `ccr.h`

```c
#include <ccr.h>

int main(void)
{
	return 0;
}
```

___

Create a `ccr_s` object and initialize it

```c
#include <ccr.h>

int main(void)
{
	ccr_s *my_region;

	if (ccr_init(&my_region)) {
		exit(1);
	}

	ccr_destroy(my_region); //optional release of resources
	return 0;
}
```

You are now ready to use the `ccr_s` object `my_region` from within any threads.

___

Let's write the same producer/consumer problem as above, but using the static library API instead

```c
#include <ccr.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


#define NUM_OF_THREADS   10
#define BUFFER_CAPACITY   3


char buffer[BUFFER_CAPACITY];
int total_items = 0;


int producer_cond(void *args)
{
	return (total_items < BUFFER_CAPACITY);
}

void producer_body(void *args)
{
	char new_data = *(char *)args;

	buffer[total_items++] = new_data;
	printf("Producer put %d\n", new_data);
}

void *producer_thread_func(void *args)
{
	ccr_s *buffer_region = (ccr_s *)args;

	/* producing the data */
	char new_data = rand();

	ccr_exec(buffer_region,
		 producer_cond,
		 NULL,
		 producer_body,
		 (void*)&new_data
		);

	return NULL;
}

int consumer_cond(void *args)
{
	return (total_items > 0);
}

void consumer_body(void *args)
{
	char *read_data = (char *)args;

	*read_data = buffer[total_items--];
}

void *consumer_thread_func(void *args)
{
	ccr_s *buffer_region = (ccr_s *)args;
	char read_data;

	ccr_exec(buffer_region,
		 consumer_cond,
		 NULL,
		 consumer_body,
		 (void*)&read_data
		);

	/* consuming the data */
	printf("Consumer read %d\n", (int)read_data);

	return NULL;
}

int main(void)
{
	ccr_s *my_region;

	if (ccr_init(&my_region)) {
		exit(1);
	}

	pthread_t threads[NUM_OF_THREADS];

	for (int i = 0; i < NUM_OF_THREADS; i++) {
		if (i < 5) {
			pthread_create(&threads[i], NULL, consumer_thread_func, (void *)my_region);
		} else {
			pthread_create(&threads[i], NULL, producer_thread_func, (void *)my_region);
		}
	}

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
```

This approach is similar to the macro API, but the code looks much more elegant and maintainable (avoids various pitfalls that can occur with erroneous macro usage).


For more, see the `examples` folder.