```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <algorithm>
#include <chrono>
#include <string>
#include <utility>
#include <condition_variable>

using namespace std;

void printOne()
{
    cout << "Thread 1: 1" << endl;
}

void printTwo()
{
    cout << "Thread 2: 2" << endl;
}

void runTask_1_2_1()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.1 — Без join() та detach()\n";
    cout << "============================================\n";

    thread t1(printOne);
    thread t2(printTwo);
}

void runTask_1_2_2()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.2 — Метод detach()\n";
    cout << "============================================\n";

    thread t1(printOne);
    thread t2(printTwo);

    t1.detach();
    t2.detach();

    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "Обидва потоки від'єднано." << endl;
}

list<int> list_1_2_3;

void AddToList_Unsafe(int start_val)
{
    for (int i = 0; i < 10; ++i)
    {
        int val = start_val + i;

        list_1_2_3.push_back(val);

        cout << "[AddToList] Додано елемент: "
             << val << endl;

        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void ListContains_Unsafe(int target_val)
{
    for (int i = 0; i < 10; ++i)
    {
        auto it = find(
            list_1_2_3.begin(),
            list_1_2_3.end(),
            target_val
        );

        cout << "[ListContains] Спроба "
             << i + 1
             << ": Елемент "
             << target_val
             << " -> ";

        if (it != list_1_2_3.end())
            cout << "Входить";
        else
            cout << "Не входить";

        cout << endl;

        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void runTask_1_2_3()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.3 — Список без синхронізації\n";
    cout << "============================================\n";

    list_1_2_3.clear();

    int start_val = 10;
    int target_val = 15;

    thread t1(AddToList_Unsafe, start_val);
    thread t2(ListContains_Unsafe, target_val);

    t1.join();
    t2.join();

    cout << "\nРоботу пункту 1.2.3 завершено." << endl;
}

list<int> list_1_2_4;
mutex m_1_2_4;

void AddToList_Mutex(int start_val)
{
    for (int i = 0; i < 10; ++i)
    {
        int val = start_val + i;

        m_1_2_4.lock();

        list_1_2_4.push_back(val);

        cout << "[AddToList] Додано елемент: "
             << val << endl;

        m_1_2_4.unlock();

        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void ListContains_Mutex(int target_val)
{
    for (int i = 0; i < 10; ++i)
    {
        m_1_2_4.lock();

        auto it = find(
            list_1_2_4.begin(),
            list_1_2_4.end(),
            target_val
        );

        cout << "[ListContains] Спроба "
             << i + 1
             << ": Елемент "
             << target_val
             << " -> ";

        if (it != list_1_2_4.end())
            cout << "Входить";
        else
            cout << "Не входить";

        cout << endl;

        m_1_2_4.unlock();

        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void runTask_1_2_4()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.4 — Mutex: lock() / unlock()\n";
    cout << "============================================\n";

    list_1_2_4.clear();

    int start_val = 10;
    int target_val = 15;

    thread t1(AddToList_Mutex, start_val);
    thread t2(ListContains_Mutex, target_val);

    t1.join();
    t2.join();

    cout << "\nРоботу пункту 1.2.4 завершено." << endl;
}

list<int> list_1_2_5;
mutex m_1_2_5;

void AddToList_Once(int val)
{
    lock_guard<mutex> lock(m_1_2_5);

    list_1_2_5.push_back(val);

    cout << "[AddToList] Додано: "
         << val << endl;
}

void ListContains_Once(int target_val)
{
    lock_guard<mutex> lock(m_1_2_5);

    auto it = find(
        list_1_2_5.begin(),
        list_1_2_5.end(),
        target_val
    );

    cout << "[ListContains] Перевірка "
         << target_val
         << " -> ";

    if (it != list_1_2_5.end())
        cout << "Входить";
    else
        cout << "Не входить";

    cout << endl;
}

void runTask_1_2_5()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.5 — 10 потоків + lock_guard\n";
    cout << "============================================\n";

    list_1_2_5.clear();

    int base_val = 100;
    int target_val = 105;

    for (int i = 0; i < 10; ++i)
    {
        thread t_add(
            AddToList_Once,
            base_val + i
        );

        thread t_check(
            ListContains_Once,
            target_val
        );

        t_add.detach();
        t_check.detach();
    }

    this_thread::sleep_for(
        chrono::milliseconds(300)
    );

    cout << "\nРоботу пункту 1.2.5 завершено." << endl;
}

struct someData
{
    string first_name;
    string last_name;
    string address;
    int age;
};

class exchangePerson
{
private:
    someData data;
    mutex m;

public:
    exchangePerson(
        string f = "Default",
        string l = "Default",
        string a = "None",
        int ag = 0
    )
        : data{move(f), move(l), move(a), ag}
    {
    }

    void print(const string& title)
    {
        lock_guard<mutex> lock(m);

        cout << title << " -> "
             << data.first_name << " "
             << data.last_name
             << ", Адреса: "
             << data.address
             << ", Вік: "
             << data.age << endl;
    }

    static void JohnDoe(
        exchangePerson& p,
        condition_variable& cv,
        mutex& syncMutex,
        int& completed
    )
    {
        {
            lock_guard<mutex> lock(p.m);

            p.data = {
                "John",
                "Doe",
                "Unknown",
                120
            };
        }

        {
            lock_guard<mutex> lock(syncMutex);
            ++completed;
        }

        cv.notify_one();
    }

    static void JacobSmith(
        exchangePerson& p,
        condition_variable& cv,
        mutex& syncMutex,
        int& completed
    )
    {
        {
            lock_guard<mutex> lock(p.m);

            p.data = {
                "Jacob",
                "Smith",
                "Known",
                1
            };
        }

        {
            lock_guard<mutex> lock(syncMutex);
            ++completed;
        }

        cv.notify_one();
    }

    static void Swap_AdoptLock(
        exchangePerson& a,
        exchangePerson& b
    )
    {
        lock(a.m, b.m);

        lock_guard<mutex> lockA(a.m, adopt_lock);
        lock_guard<mutex> lockB(b.m, adopt_lock);

        cout << "\n[Swap] Дані до обміну:" << endl;

        cout << "A: "
             << a.data.first_name << " "
             << a.data.last_name
             << ", Адреса: "
             << a.data.address
             << ", Вік: "
             << a.data.age << endl;

        cout << "B: "
             << b.data.first_name << " "
             << b.data.last_name
             << ", Адреса: "
             << b.data.address
             << ", Вік: "
             << b.data.age << endl;

        swap(a.data, b.data);

        cout << "\n[Swap] Дані після обміну:" << endl;

        cout << "A: "
             << a.data.first_name << " "
             << a.data.last_name
             << ", Адреса: "
             << a.data.address
             << ", Вік: "
             << a.data.age << endl;

        cout << "B: "
             << b.data.first_name << " "
             << b.data.last_name
             << ", Адреса: "
             << b.data.address
             << ", Вік: "
             << b.data.age << endl;
    }

    static void Swap_UniqueLock(
        exchangePerson& a,
        exchangePerson& b
    )
    {
        unique_lock<mutex> lockA(a.m, defer_lock);
        unique_lock<mutex> lockB(b.m, defer_lock);

        lock(lockA, lockB);

        cout << "\n[Swap] Дані до обміну:" << endl;

        cout << "A: "
             << a.data.first_name << " "
             << a.data.last_name
             << ", Адреса: "
             << a.data.address
             << ", Вік: "
             << a.data.age << endl;

        cout << "B: "
             << b.data.first_name << " "
             << b.data.last_name
             << ", Адреса: "
             << b.data.address
             << ", Вік: "
             << b.data.age << endl;

        swap(a.data, b.data);

        cout << "\n[Swap] Дані після обміну:" << endl;

        cout << "A: "
             << a.data.first_name << " "
             << a.data.last_name
             << ", Адреса: "
             << a.data.address
             << ", Вік: "
             << a.data.age << endl;

        cout << "B: "
             << b.data.first_name << " "
             << b.data.last_name
             << ", Адреса: "
             << b.data.address
             << ", Вік: "
             << b.data.age << endl;
    }
};

void runTask_1_2_6()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.6 — lock() + adopt_lock\n";
    cout << "============================================\n";

    exchangePerson p1;
    exchangePerson p2;

    mutex syncMutex;
    condition_variable cv;
    int completed = 0;

    thread t1(
        &exchangePerson::JohnDoe,
        ref(p1),
        ref(cv),
        ref(syncMutex),
        ref(completed)
    );

    thread t2(
        &exchangePerson::JacobSmith,
        ref(p2),
        ref(cv),
        ref(syncMutex),
        ref(completed)
    );

    t1.detach();
    t2.detach();

    {
        unique_lock<mutex> lock(syncMutex);

        cv.wait(
            lock,
            [&]()
            {
                return completed == 2;
            }
        );
    }

    thread t_swap(
        &exchangePerson::Swap_AdoptLock,
        ref(p1),
        ref(p2)
    );

    t_swap.join();

    cout << "\nФінальний стан об'єктів:" << endl;

    p1.print("p1");
    p2.print("p2");
}

void runTask_1_2_7()
{
    cout << "\n============================================\n";
    cout << "ПУНКТ 1.2.7 — unique_lock + defer_lock\n";
    cout << "============================================\n";

    exchangePerson p1;
    exchangePerson p2;

    mutex syncMutex;
    condition_variable cv;
    int completed = 0;

    thread t1(
        &exchangePerson::JohnDoe,
        ref(p1),
        ref(cv),
        ref(syncMutex),
        ref(completed)
    );

    thread t2(
        &exchangePerson::JacobSmith,
        ref(p2),
        ref(cv),
        ref(syncMutex),
        ref(completed)
    );

    t1.detach();
    t2.detach();

    {
        unique_lock<mutex> lock(syncMutex);

        cv.wait(
            lock,
            [&]()
            {
                return completed == 2;
            }
        );
    }

    thread t_swap(
        &exchangePerson::Swap_UniqueLock,
        ref(p1),
        ref(p2)
    );

    t_swap.join();

    cout << "\nФінальний стан об'єктів:" << endl;

    p1.print("p1");
    p2.print("p2");
}

int main()
{
    int choice;

    while (true)
    {
        cout << "\n\n";
        cout << "============================================\n";
        cout << "        ЛАБОРАТОРНА РОБОТА №1\n";
        cout << "  Застосування м'ютексів та потоків C++\n";
        cout << "============================================\n";
        cout << "  1. 1.2.1  — Потоки без join / detach\n";
        cout << "  2. 1.2.2  — Метод detach()\n";
        cout << "  3. 1.2.3  — Список без синхронізації\n";
        cout << "  4. 1.2.4  — Mutex lock() / unlock()\n";
        cout << "  5. 1.2.5  — lock_guard\n";
        cout << "  6. 1.2.6  — lock() + adopt_lock\n";
        cout << "  7. 1.2.7  — unique_lock + defer_lock\n";
        cout << "  0. Вихід\n";
        cout << "============================================\n";
        cout << "Оберіть пункт: ";

        cin >> choice;

        switch (choice)
        {
            case 1:
                runTask_1_2_1();
                break;

            case 2:
                runTask_1_2_2();
                break;

            case 3:
                runTask_1_2_3();
                break;

            case 4:
                runTask_1_2_4();
                break;

            case 5:
                runTask_1_2_5();
                break;

            case 6:
                runTask_1_2_6();
                break;

            case 7:
                runTask_1_2_7();
                break;

            case 0:
                cout << "\nПрограму завершено.\n";
                return 0;

            default:
                cout << "\nНевірний вибір. Спробуйте ще раз.\n";
        }
    }
}
```
