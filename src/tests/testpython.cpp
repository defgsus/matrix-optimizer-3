/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#include <thread>
#include <vector>

#include "testpython.h"
#include "python/34/python.h"
#include "io/error.h"
#include "io/log.h"


struct TestPython::Private
{
    Private(TestPython* p)
        : p (p)
    { }

    TestPython * p;
    int exec();
    int exec(const char* utf8);
};

TestPython::TestPython()
    : p_        (new Private(this))
{

}

TestPython::~TestPython()
{
    delete p_;
}

int TestPython::run() { return p_->exec(); }



int TestPython::Private::exec()
{
    const char * src =
            "import matrixoptimizer as mo\n"
            "print( mo.instance_id() )\n"
            ;

    MO::PYTHON34::initPython();

    int err = 0;

    for (int i=0; i<10; ++i)
    {
        MO_PRINT("---- run #" << (1+i) << " ----");
        err += exec(src);
    }

    MO_PRINT("\n--------- THREADED ----------");

    std::vector<std::thread*> threads;
    for (int i=0; i<10; ++i)
    {
        auto thread = new std::thread([=]()
        {
            exec(src);
        });

        //thread->join();
        threads.push_back(thread);
    }

    for (auto t : threads)
    {
        if (t->joinable())
            t->join();
        delete t;
    }

    return err;
}

int TestPython::Private::exec(const char* utf8)
{
    MO::PYTHON34::PythonInterpreter interp;

    try
    {
        interp.execute(utf8);
    }
    catch (const MO::Exception& e)
    {
        MO_PRINT("EXCEPTION: " << e.what());
        return 1;
    }

    return 0;
}
