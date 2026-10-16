#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <deque>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <future>
#include <cmath>
#include <string>

using namespace std;

bool isPrime(long long n) {
    if (n <= 1) return false;
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

long long getNthPrime(int n) {
    int count = 0;
    long long current = 1;
    while (count < n) {
        current++;
        if (isPrime(current)) count++;
    }
    return current;
}

void task_1_2_1() {
    cout << "\n=== 1.2.1 ===\n";
    queue<int> data_queue;
    bool flag = false;
    mutex mtx;

    auto prep = [&]() {
        cout << "Enter integers (enter non-number to finish):\n";
        int val;
        while (cin >> val) {
            lock_guard<mutex> lk(mtx);
            data_queue.push(val);
        }
        cin.clear();
        cin.ignore(10000, '\n');
        lock_guard<mutex> lk(mtx);
        flag = true;
    };

    auto proc = [&]() {
        while (true) {
            unique_lock<mutex> lk(mtx);
            if (!data_queue.empty()) {
                int val = data_queue.front();
                data_queue.pop();
                lk.unlock();
                if (isPrime(val)) {
                    cout << "Prime: " << val << endl;
                }
            } else if (flag) {
                break;
            } else {
                lk.unlock();
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
    };

    thread t1(prep);
    thread t2(proc);
    t1.detach();
    t2.join();
    cout << "Done.\n";
}

void task_1_2_2() {
    cout << "\n=== 1.2.2 ===\n";
    mutex mtx;
    condition_variable cv;
    int i = 0;

    auto waits = [&](int id) {
        unique_lock<mutex> lk(mtx);
        cout << "Thread " << id << " waiting...\n";
        cv.wait(lk, [&] { return i == 1; });
        cout << "Thread " << id << " done!\n";
    };

    auto awake = [&]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "Awake: notify_all #1\n";
        cv.notify_all();

        this_thread::sleep_for(chrono::milliseconds(100));
        {
            lock_guard<mutex> lk(mtx);
            i = 1;
        }
        cout << "Awake: i=1, notify_all #2\n";
        cv.notify_all();
    };

    thread t1(waits, 1);
    thread t2(waits, 2);
    thread t3(waits, 3);
    thread tAwake(awake);

    t1.join();
    t2.join();
    t3.join();
    tAwake.join();
    cout << "Done.\n";
}

void task_1_2_3() {
    cout << "\n=== 1.2.3 ===\n";
    mutex mtx;
    condition_variable cv;
    int i = 0;

    auto thread_func = [&](int id) {
        unique_lock<mutex> lk(mtx);
        cout << "Thread " << id << " waiting...\n";
        cv.wait(lk, [&] { return i == 1; });
        cout << "Message from thread " << id << endl;
    };

    auto notify_func = [&]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        {
            lock_guard<mutex> lk(mtx);
            i = 1;
        }
        cout << "Notify: notify_one\n";
        cv.notify_one(); 
    };

    thread t1(thread_func, 1);
    thread t2(thread_func, 2);
    thread t3(thread_func, 3);
    thread tNotify(notify_func);

    t1.detach();
    t2.detach();
    t3.detach();

    tNotify.join();
    
    this_thread::sleep_for(chrono::milliseconds(50));
    cout << "Done.\n";
}

void task_1_2_4() {
    cout << "\n=== 1.2.4 ===\n";
    queue<int> data_queue;
    bool finished = false;
    mutex mtx;
    condition_variable cv;

    auto prep = [&]() {
        cout << "Enter numbers (non-number to finish):\n";
        int val;
        while (cin >> val) {
            {
                lock_guard<mutex> lk(mtx);
                data_queue.push(val);
            }
            cv.notify_one();
        }
        cin.clear();
        cin.ignore(10000, '\n');
        {
            lock_guard<mutex> lk(mtx);
            finished = true;
        }
        cv.notify_all();
    };

    auto proc = [&]() {
        while (true) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&] { return !data_queue.empty() || finished; });

            while (!data_queue.empty()) {
                int val = data_queue.front();
                data_queue.pop();
                lk.unlock();
                if (isPrime(val)) {
                    cout << "Prime: " << val << endl;
                }
                lk.lock();
            }

            if (finished && data_queue.empty()) {
                break;
            }
        }
    };

    thread t1(prep);
    thread t2(proc);
    t1.detach();
    t2.join();
    cout << "Done.\n";
}

void task_1_2_5() {
    cout << "\n=== 1.2.5 ===\n";
    int n;
    cout << "Enter n: ";
    cin >> n;

    cout << "\nAsync mode:\n";
    auto start = chrono::high_resolution_clock::now();
    future<long long> fut_async = async(launch::async, getNthPrime, n);

    int choice;
    cout << "While calculating, choose:\n1 - sqrt(n)\n2 - sin(n)\n3 - log(n)\nChoice: ";
    cin >> choice;

    double result = 0;
    if (choice == 1) {
        result = sqrt(n);
        cout << "sqrt(" << n << ") = " << result << endl;
    } else if (choice == 2) {
        result = sin(n);
        cout << "sin(" << n << ") = " << result << endl;
    } else if (choice == 3) {
        result = log(n);
        cout << "log(" << n << ") = " << result << endl;
    }

    long long res_async = fut_async.get();
    auto end = chrono::high_resolution_clock::now();
    cout << "async result: " << res_async << ", time: " << chrono::duration<double>(end - start).count() << "s\n";

    cout << "\nDeferred mode:\n";
    start = chrono::high_resolution_clock::now();
    future<long long> fut_def = async(launch::deferred, getNthPrime, n);
    long long res_def = fut_def.get();
    end = chrono::high_resolution_clock::now();
    cout << "deferred result: " << res_def << ", time: " << chrono::duration<double>(end - start).count() << "s\n";
    cout << "Done.\n";
}

void task_1_2_6() {
    cout << "\n=== 1.2.6 ===\n";
    deque<packaged_task<long long()>> tasks_deque;
    deque<int> numbers_deque;
    mutex mtx;
    condition_variable cv;
    bool stop = false;

    auto worker = [&]() {
        while (true) {
            packaged_task<long long()> task;
            {
                unique_lock<mutex> lk(mtx);
                cv.wait(lk, [&] { return !tasks_deque.empty() || stop; });
                if (tasks_deque.empty() && stop) break;
                task = move(tasks_deque.front());
                tasks_deque.pop_front();
                numbers_deque.pop_front();
            }
            if (task.valid()) {
                task();
            }
        }
    };

    thread tWorker(worker);

    cout << "Enter n (0 or 'stop' to exit):\n";
    string input;
    while (cin >> input) {
        if (input == "0" || input == "stop") break;
        try {
            int n = stoi(input);
            packaged_task<long long()> task([n]() { return getNthPrime(n); });
            auto fut = task.get_future();
            {
                lock_guard<mutex> lk(mtx);
                tasks_deque.push_back(move(task));
                numbers_deque.push_back(n);
            }
            cv.notify_one();
            cout << "Result: " << fut.get() << endl;
        } catch (...) {
            cout << "Invalid input.\n";
        }
    }
    cin.clear();
    cin.ignore(10000, '\n');

    {
        lock_guard<mutex> lk(mtx);
        stop = true;
    }
    cv.notify_all();
    tWorker.join();
    cout << "Done.\n";
}

void task_1_2_7() {
    cout << "\n=== 1.2.7 ===\n";
    int n;
    cout << "Enter n: ";
    cin >> n;

    auto first = [](int num, promise<long long> p1, promise<bool> pBool, promise<long long> p2) {
        p1.set_value(getNthPrime(num));
        pBool.set_value(true);
        p2.set_value(getNthPrime(num * 10));
    };

    auto second = [](int num, future<bool> fBool) {
        fBool.get();
        this_thread::sleep_for(chrono::seconds(2));
        cout << "sqrt(" << num << ") = " << sqrt(num) << endl;
    };

    promise<long long> prom1;
    future<long long> fut1 = prom1.get_future();
    promise<bool> promBool;
    future<bool> futBool = promBool.get_future();
    promise<long long> prom2;
    future<long long> fut2 = prom2.get_future();

    thread t1(first, n, move(prom1), move(promBool), move(prom2));
    thread t2(second, n, move(futBool));

    cout << n << "-th prime: " << fut1.get() << endl;
    cout << (n * 10) << "-th prime: " << fut2.get() << endl;

    t1.join();
    t2.join();
    cout << "Done.\n";
}

int main() {
    cout << "Lab #2 - Multithreading\n\n";

    int choice;
    do {
        cout << "Choose task (1-7, 0 for all): ";
        cin >> choice;

        switch (choice) {
            case 1: task_1_2_1(); break;
            case 2: task_1_2_2(); break;
            case 3: task_1_2_3(); break;
            case 4: task_1_2_4(); break;
            case 5: task_1_2_5(); break;
            case 6: task_1_2_6(); break;
            case 7: task_1_2_7(); break;
            case 0:
                task_1_2_1();
                task_1_2_2();
                task_1_2_3();
                task_1_2_4();
                task_1_2_5();
                task_1_2_6();
                task_1_2_7();
                cout << "\nAll done!\n";
                break;
            default: cout << "Invalid.\n";
        }
        cout << "\n";
    } while (choice != 0);

    return 0;
}
