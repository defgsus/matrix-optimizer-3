/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

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

    int testVector();
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

    err += testVector();

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

int TestPython::Private::testVector()
{
    MO::PYTHON34::PythonInterpreter interp;

    try
    {
        interp.execute(
                    "from matrixoptimizer import *\n"
                    "v = Vec(1.)\n"
                    "print(v)\n"
                    "v = Vec(1., 2.)\n"
                    "print(v)\n"
                    "v = Vec(1., 2., 3.)\n"
                    "print(v)\n"
                    "v = Vec(1., 2., 3., 4.)\n"
                    "print(v)\n"
                    "print('print(v.x, v.y, v.z, v.w):', v.x, v.y, v.z, v.w)\n"
                    "print('print(v.xx, v.xxx, v.zyxw):', v.xx, v.xxx, v.zyxw)\n"
                    "v.w = 10; print('v.w = 10: ', v)\n"
                    "v.yw = 5, 6; print('v.yw = 5, 6: ', v)\n"

                    "v = Vec([0])\n"
                    "print(v)\n"
                    "v = Vec([0, 1])\n"
                    "print(v)\n"
                    "v = Vec([0, 1, 2])\n"
                    "print(v)\n"
                    "v = Vec([0, 1, 2, 3])\n"
                    "print(v)\n"
                    "v = Vec([0, 1], 2, [3])\n"
                    "print(v)\n"
                    "def test_func(code):\n"
                    "    print(code, \": \", )\n"
                    "    eval(\"print('   ', \" + code + \")\")\n"
                    "\n"
                    "test_func(\"Vec(1,2,3) + (3, 2, 1)\")\n"
                    "test_func(\"Vec(1,2,3) - (3, 2, 1)\")\n"
                    "test_func(\"Vec(1,2,3) * (3, 2, 1)\")\n"
                    "test_func(\"Vec(1,2,3) / (3, 2, 1)\")\n"
                    "print(\"Vec(1,2,3,4) += (4,3,2,1)\"); v = Vec(1,2,3,4); v += (4,3,2,1); print('   ', v)\n"
                    "print(\"Vec(1,2,3,4) -= (4,3,2,1)\"); v = Vec(1,2,3,4); v -= (4,3,2,1); print('   ', v)\n"
                    "print(\"Vec(1,2,3,4) *= (4,3,2,1)\"); v = Vec(1,2,3,4); v *= (4,3,2,1); print('   ', v)\n"
                    "print(\"Vec(1,2,3,4) /= (4,3,2,1)\"); v = Vec(1,2,3,4); v /= (4,3,2,1); print('   ', v)\n"
                    "test_func(\"Vec(2,3,4).pow(2)\")\n"
                    "test_func(\"Vec(2,3,4).pow(.5, 2, 3)\")\n"
                    );
    }
    catch (const MO::Exception& e)
    {
        MO_PRINT("EXCEPTION: " << e.what());
        return 1;
    }


    return 0;
}

#endif // #ifdef MO_ENABLE_PYTHON34
