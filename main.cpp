#include<iostream>
#include<thread>
#include<functional>
#include<condition_variable>
#include<future>
#include<chrono>
#include<mutex>

std::mutex mtx;
std::condition_variable cv;  //条件变量cv
bool ready = false;
int cargo = 0;

//消费者线程
void consumer(int n)
{
	for (int i = 0; i < n; i++)
	{
		std::unique_lock<std::mutex> lck(mtx);
		while(cargo == 0)
			cv.wait(lck);
		std::cout << cargo << "\n";
		cargo = 0;
	}
}

void do_print_id(int id)
{
	std::unique_lock<std::mutex> lck(mtx);          //当前的线程尝试得到mtx的控制权
	while (!ready)                                  //当前的线程已经得到了mtx的控制权，由于ready此时为false，所以函数调用wait函数使得当前的线程进入阻塞，同时释放了对mtx的控制权
		cv.wait(lck);                               //当所有的线程被唤醒时，所有的线程都会去尝试锁住mtx，这也就是一个竞争的过程，先获得锁的线程先运行
	std::cout << "thread " << id << std::endl;      
}

void go()
{
	std::unique_lock<std::mutex> lck(mtx);
	ready = true;
	cv.notify_all();
}

bool checkPrime(int n)
{
	for (int i = 2; i < n; i++)
	{
		if (n % i == 0)
			return false;
	}
	return true;
}

void print_ten(char c, int ms)//函数功能：每ms个毫秒打印一个字符c,打印10个
{
	for (int i = 0; i < 10; i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		std::cout << c;
	}
}

/*void firework()
{
	while (!mtx.try_lock_for(std::chrono::milliseconds(200)))
	{
		std::cout << "-";
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "*" << std::endl;
	mtx.unlock();
}*/

int countDown(int from, int to)
{
	for (int i = from; i != to; --i)
	{
		std::cout << i << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "Finished!\n";
	return from - to;
}

void print_int(std::future<int> & fut)
{
	int x = fut.get();
	std::cout << "value: " << x << " " << std::endl;
}

void f1(int n)
{
	for (int i = 0; i < 5; i++)
	{
		std::cout << "Thread " << n << " excuting" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}
}

void f2(int & n)
{
	for (int i = 0; i < 5; i++)
	{
		std::cout << "Thread 2 executing" << std::endl;
		n++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void thread_task(int n)
{
	std::this_thread::sleep_for(std::chrono::seconds(n));
	std::cout << "hello thread" << std::this_thread::get_id() << "paused " << n << "seconds" << std::endl;

}

int main()
{
	//9.生产者消费者问题
	std::thread consumer_thread(consumer, 10);
	for (int i = 0; i < 10; i++)
	{
		while(cargo != 0)
			std::this_thread::yield();    //当前的线程放弃未使用完的时间片
		std::unique_lock<std::mutex> lck(mtx);
		cargo = i + 1;
		cv.notify_one();
	}

	consumer_thread.join();

	getchar();
	return 0;



	//8. std::condition_variable使用方法
	/*std::thread threads[10];
	for (int i = 0; i < 10; i++)
	{
		threads[i] = std::thread(do_print_id, i);    //对于每一个线程，他们建立之初都获得了mutex，但是由于ready的值为false，他们全部调用了wait函数，也自然释放了对mtx的控制
	}                                                //此时每一个线程都处在等待ready变为true以及有线程将他们全部唤醒。
	std::cout << "the threads are ready to race!\n";
	go();                                           //在go()函数中，go函数先得到了mtx的控制权，并且修改了ready的值。在go结束时，自动的释放了对mtx的控制权，十个线程开始竞争
	for (int i = 0; i < 10; i++)
	{
		threads[i].join();
	}
	getchar();
	return 0;
	*/

	//7. std::async()异步任务的启动策略，探究std::launch类的用法
	//当启动策略时std::launch::async时，异步任务会立即在另一个线程中调用，通过共享状态返回异步任务的结果
	/*std::future<void> fut = std::async(std::launch::async, print_ten, '@', 100);
	std::future<void> foo = std::async(std::launch::async, print_ten, '#', 200);

	std::cout << "the two async has begun in two thread!\n";
	fut.get();
	foo.get();
	getchar();
	return 0;*/

	//当启动策略是std::launch::deferred时，异步过程会在共享状态被访问时才被调用，相当于按需调用(future对象调用get函数时线程才启动)
	/*std::future<void> fut = std::async(std::launch::deferred, print_ten, '#', 100);
	std::future<void> foo = std::async(std::launch::deferred, print_ten, '&', 200);
	
	std::cout << "The two async are not running!\n";
	fut.get();
	foo.get();
	getchar();
	return 0;*/



	//6. std::future::wait_for()函数测试
	/*std::packaged_task<bool(int)> task(checkPrime);
	std::future<bool> ret = task.get_future();
	std::chrono::milliseconds span(1);
	std::cout << "Checking!\n";
	task.operator()(194232491);
	while (ret.wait_for(span) == std::future_status::timeout)//每超时一次打印一个“.”
		std::cout << ".";
	std::cout << "\n194232491";
	if (ret.get() == true)
		std::cout << "\n194232491 is prime.\n";
	else
		std::cout << "\n194232491 isn't prime\n";
	getchar();
	return 0;
	*/
	
	//5. std::packaged_task用法    package_task将一个可调用的对象包装后并没有开始执行这个调用对象，需调用成员函数operator()()
	/*std::packaged_task<int(int, int)> task(countDown);     //将函数countDown打包成packaged_task，其中int<int，int>代表了函数的两个参数是int，返回值是int
	std::future<int> ret = task.get_future();              //将future对象ret与task绑定，当前线程可以从ret访问task所在的线程的返回值
	if(task.valid() == true)
		std::cout << "The task is combined with a function"<<std::endl;
	//task.operator()(10, 0);
	std::thread th = std::thread(std::move(task), 10, 0);  //因为所要执行的函数被打包成了packaged_task,所以要用move函数将任务转移到新建的thread中
	int value = ret.get();
	std::cout << "The countDown lasted for " << value << " seconds!" << std::endl;
	th.join();
	getchar();
	return 0;*/
	
	//4. promise + future
	/*std::promise<int> prom;        //声明promise对象
	std::future<int> fut = prom.get_future();    //声明future对象并于prom对象绑定
	std::thread t(print_int, std::ref(fut));     //创建线程并传入future
	prom.set_value(10);                           //设置promise对象的值，新线程中的future对象自此可以调用get方法
	t.join();
	getchar();
	return 0;*/
	
	
	//3. mutex
	/*std::thread threads[10];
	for (int i = 0; i < 10; i++)
	{
		threads[i] = std::thread(firework);
	}
	for (auto & u : threads)
	{
		u.join();
	}
	getchar();
	return 0;
	*/
	
	
	//2. thread各种构造函数例子
	/*
	int n = 0;
	std::thread t1;//t1此时只是一个空的thread执行对象
	std::thread t2(f1, n + 1);//由f1函数的参数可知，函数是单纯的传值引用
	std::thread t3(f2, std::ref(n));//引用传递
	std::thread t4(std::move(t3));//t4现在在跑f2函数，t3不再是一个线程

	t2.join();
	t4.join();
	std::cout << "Final value of n is " << n << std::endl;
	getchar();
	return 0;
	*/

	//1.thread初始化构造函数样例
	/*
	std::thread threads[5];
	std::cout << "spawning 5 threads..." << std::endl;
	for (int i = 0; i < 5; i++)
	{
		threads[i] = std::thread(thread_task, i + 1);
	}
	std::cout << "Done spawning threads! " << std::endl;
	for (auto & u : threads)
	{
		u.join();
	}
	std::cout << "All threads joined." << std::endl;
	return 0;
	*/
}
