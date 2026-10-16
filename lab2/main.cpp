#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <deque>
#include <vector>
#include <future>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace std;
using namespace chrono;

bool isPrime(long long n) {
    if (n < 2) {
        return false;
    }

    if (n == 2) {
        return true;
    }

    if (n % 2 == 0) {
        return false;
    }

    for (long long i = 3; i <= n / i; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

long long getNthPrime(int n) {
    if (n <= 0) {
        throw invalid_argument("n must be positive");
    }

    int count = 0;
    long long number = 1;

    while (count < n) {
        ++number;

        if (isPrime(number)) {
            ++count;
        }
    }

    return number;
}

void task1() {
    cout << "\n=== Task 1.2.1 ===\n";
    cout << "Enter integers separated by space.\n";
    cout << "To finish, enter q and press Enter.\n";

    queue<int> dataQueue;
    mutex mtx;
    bool finished = false;

    thread dataPreparation([&]() {
        int value;

        while (cin >> value) {
            {
                lock_guard<mutex> lock(mtx);
                dataQueue.push(value);
            }
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        {
            lock_guard<mutex> lock(mtx);
            finished = true;
        }
    });

    thread dataProcessing([&]() {
        while (true) {
            int value = 0;
            bool hasValue = false;

            {
                unique_lock<mutex> lock(mtx);

                if (!dataQueue.empty()) {
                    value = dataQueue.front();
                    dataQueue.pop();
                    hasValue = true;
                } else if (finished) {
                    break;
                }
            }

            if (hasValue) {
                if (isPrime(value)) {
                    cout << "Prime number: " << value << '\n';
                }
            } else {
                this_thread::sleep_for(milliseconds(100));
            }
        }
    });

    dataPreparation.join();
    dataProcessing.join();

    cout << "Processing completed.\n";
}

void task2() {
    cout << "\n=== Task 1.2.2 ===\n";

    mutex mtx;
    condition_variable cv;
    int i = 0;

    vector<thread> waiters;

    for (int j = 0; j < 3; ++j) {
        waiters.emplace_back([&, j]() {
            unique_lock<mutex> lock(mtx);

            cout << "Thread Waits " << j + 1
                 << " is waiting for signal.\n";

            cv.wait(lock, [&]() {
                return i == 1;
            });

            cout << "Thread Waits " << j + 1
                 << " resumed execution.\n";
        });
    }

    thread awake([&]() {
        this_thread::sleep_for(milliseconds(100));

        cout << "Awake: first notify_all().\n";
        cv.notify_all();

        this_thread::sleep_for(milliseconds(100));

        {
            lock_guard<mutex> lock(mtx);
            i = 1;
        }

        cout << "Awake: condition changed, second notify_all().\n";
        cv.notify_all();
    });

    awake.join();

    for (auto& t : waiters) {
        t.join();
    }

    cout << "All threads have finished.\n";
}

void task3() {
    cout << "\n=== Task 1.2.3 ===\n";

    mutex mtx;
    condition_variable cv;
    int i = 0;

    vector<thread> waiters;

    for (int j = 0; j < 3; ++j) {
        waiters.emplace_back([&, j]() {
            unique_lock<mutex> lock(mtx);

            cout << "Thread Waits " << j + 1
                 << " is waiting for signal.\n";

            cv.wait(lock, [&]() {
                return i == 1;
            });

            cout << "Thread Waits " << j + 1
                 << " woke up.\n";

            lock.unlock();

            cv.notify_one();
        });
    }

    thread notifier([&]() {
        this_thread::sleep_for(milliseconds(100));

        {
            lock_guard<mutex> lock(mtx);
            i = 1;
        }

        cout << "Notify: notify_one().\n";
        cv.notify_one();
    });

    notifier.join();

    for (auto& t : waiters) {
        t.join();
    }

    cout << "All threads have finished.\n";
}

void task4() {
    cout << "\n=== Task 1.2.4 ===\n";
    cout << "Enter integers separated by space.\n";
    cout << "To finish, enter q and press Enter.\n";

    queue<int> dataQueue;
    mutex mtx;
    condition_variable cv;
    bool finished = false;

    thread dataPreparation([&]() {
        int value;

        while (cin >> value) {
            {
                lock_guard<mutex> lock(mtx);
                dataQueue.push(value);
            }

            cv.notify_one();
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        {
            lock_guard<mutex> lock(mtx);
            finished = true;
        }

        cv.notify_all();
    });

    thread dataProcessing([&]() {
        while (true) {
            int value;

            {
                unique_lock<mutex> lock(mtx);

                cv.wait(lock, [&]() {
                    return !dataQueue.empty() || finished;
                });

                if (dataQueue.empty() && finished) {
                    break;
                }

                value = dataQueue.front();
                dataQueue.pop();
            }

            if (isPrime(value)) {
                cout << "Prime number: " << value << '\n';
            }
        }
    });

    dataPreparation.join();
    dataProcessing.join();

    cout << "Processing completed.\n";
}

void task5() {
    cout << "\n=== Task 1.2.5 ===\n";

    int n;

    cout << "Enter the prime number index n: ";

    if (!(cin >> n) || n <= 0) {
        cout << "Error: n must be a positive integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    auto computeTimed = [n]() {
        auto start = steady_clock::now();

        long long result = getNthPrime(n);

        auto finish = steady_clock::now();

        double elapsed =
            duration<double>(finish - start).count();

        return make_pair(result, elapsed);
    };

    cout << "\nStarting async...\n";

    auto asyncFuture = async(launch::async, computeTimed);

    double x;

    cout << "Enter a positive number x for sqrt, sin, and log: ";

    while (!(cin >> x) || !isfinite(x) || x <= 0) {
        cout << "Enter a valid positive number x: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "sqrt(" << x << ") = " << sqrt(x) << '\n';
    cout << "sin(" << x << ") = " << sin(x)
         << " (x in radians)\n";
    cout << "log(" << x << ") = " << log(x) << '\n';

    auto asyncResult = asyncFuture.get();

    cout << "\nlaunch::async mode:\n";
    cout << n << "-th prime number: "
         << asyncResult.first << '\n';
    cout << "Computation time: "
         << asyncResult.second << " s\n";

    cout << "\nStarting launch::deferred...\n";

    auto deferredFuture = async(launch::deferred, computeTimed);

    auto deferredResult = deferredFuture.get();

    cout << "launch::deferred mode:\n";
    cout << n << "-th prime number: "
         << deferredResult.first << '\n';
    cout << "Computation time: "
         << deferredResult.second << " s\n";

    cout << "\nComparison:\n";
    cout << "async executes the task asynchronously.\n";
    cout << "deferred executes the task during get() or wait().\n";
    cout << "Actual time depends on the computer and the value of n.\n";
}

void task6() {
    cout << "\n=== Task 1.2.6 ===\n";
    cout << "Enter n to calculate the n-th prime number.\n";
    cout << "To finish, enter q and press Enter.\n";

    deque<packaged_task<long long(int)>> tasksDeque;
    deque<int> numbersDeque;

    vector<future<long long>> results;

    mutex mtx;
    condition_variable cv;
    bool stopWorker = false;

    thread worker([&]() {
        while (true) {
            packaged_task<long long(int)> task;
            int number = 0;

            {
                unique_lock<mutex> lock(mtx);

                cv.wait(lock, [&]() {
                    return stopWorker ||
                           (!tasksDeque.empty() &&
                            !numbersDeque.empty());
                });

                if (stopWorker &&
                    tasksDeque.empty() &&
                    numbersDeque.empty()) {
                    break;
                }

                if (tasksDeque.empty() || numbersDeque.empty()) {
                    continue;
                }

                task = move(tasksDeque.front());
                tasksDeque.pop_front();

                number = numbersDeque.front();
                numbersDeque.pop_front();
            }

            task(number);
        }
    });

    int n;

    while (cin >> n) {
        if (n <= 0) {
            cout << "n must be positive. Try again.\n";
            continue;
        }

        packaged_task<long long(int)> task(
            [](int value) {
                return getNthPrime(value);
            }
        );

        future<long long> result = task.get_future();

        {
            lock_guard<mutex> lock(mtx);

            tasksDeque.push_back(move(task));
            numbersDeque.push_back(n);

            results.push_back(move(result));
        }

        cv.notify_one();

        cout << "Task for n = " << n
             << " added to the queue.\n";
    }

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    {
        lock_guard<mutex> lock(mtx);
        stopWorker = true;
    }

    cv.notify_all();
    worker.join();

    cout << "\nComputation results:\n";

    for (size_t j = 0; j < results.size(); ++j) {
        try {
            cout << "Task " << j + 1
                 << ": " << results[j].get() << '\n';
        } catch (const exception& e) {
            cout << "Computation error: "
                 << e.what() << '\n';
        }
    }

    cout << "All tasks processed.\n";
}

void task7() {
    cout << "\n=== Task 1.2.7 ===\n";

    int n;

    cout << "Enter a positive number n: ";

    if (!(cin >> n) ||
        n <= 0 ||
        n > numeric_limits<int>::max() / 10) {
        cout << "Error: n must be positive and not too large.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    promise<long long> primePromise;
    promise<long long> prime10Promise;
    promise<bool> startSecondPromise;

    future<long long> primeFuture = primePromise.get_future();
    future<long long> prime10Future = prime10Promise.get_future();
    future<bool> startSecondFuture = startSecondPromise.get_future();

    thread firstThread(
        [n,
         primePromise = move(primePromise),
         prime10Promise = move(prime10Promise),
         startSecondPromise = move(startSecondPromise)]() mutable {

            long long prime = getNthPrime(n);
            primePromise.set_value(prime);

            startSecondPromise.set_value(true);

            long long prime10 = getNthPrime(n * 10);
            prime10Promise.set_value(prime10);
        }
    );

    thread secondThread(
        [n, startSecondFuture = move(startSecondFuture)]() mutable {
            bool start = startSecondFuture.get();

            if (start) {
                this_thread::sleep_for(seconds(2));

                cout << "Second thread: sqrt(n) = "
                     << sqrt(static_cast<double>(n)) << '\n';
            }
        }
    );

    cout << n << "-th prime number: "
         << primeFuture.get() << '\n';

    cout << (n * 10) << "-th prime number: "
         << prime10Future.get() << '\n';

    firstThread.join();
    secondThread.join();

    cout << "Both threads have finished.\n";
}


int main() {
    int choice;

    while (true) {
        cout << "\n====================================\n";
        cout << "Laboratory Work #2\n";
        cout << "====================================\n";
        cout << "1 - Task 1.2.1\n";
        cout << "2 - Task 1.2.2\n";
        cout << "3 - Task 1.2.3\n";
        cout << "4 - Task 1.2.4\n";
        cout << "5 - Task 1.2.5\n";
        cout << "6 - Task 1.2.6\n";
        cout << "7 - Task 1.2.7\n";
        cout << "0 - Run all tasks sequentially\n";
        cout << "-1 - Exit\n";
        cout << "Your choice: ";

        if (!(cin >> choice)) {
            break;
        }

        try {
            switch (choice) {
                case 1:
                    task1();
                    break;

                case 2:
                    task2();
                    break;

                case 3:
                    task3();
                    break;

                case 4:
                    task4();
                    break;

                case 5:
                    task5();
                    break;

                case 6:
                    task6();
                    break;

                case 7:
                    task7();
                    break;

                case 0:
                    task1();
                    task2();
                    task3();
                    task4();
                    task5();
                    task6();
                    task7();
                    cout << "\nAll done!\n";
                    break;

                case -1:
                    cout << "Program terminated.\n";
                    return 0;

                default:
                    cout << "Unknown menu option. Try again.\n";
                    break;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << '\n';
        }
    }

    return 0;
}
