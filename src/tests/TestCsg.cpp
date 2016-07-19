#include "TestCsg.h"
#include "math/CsgCombine.h"
#include "math/CsgPrimitives.h"
#include "types/Properties.h"
#include "io/log.h"

namespace MO {


int TestCsg::run()
{
    auto root = new CsgRoot;

        auto c = new CsgUnion;
        root->addChildren(c);
        c->setName("big union");

            auto s = new CsgSphere;
            auto props = s->properties();
            props.set("x", 1);
            s->setProperties(props);
            c->addChildren(s);
            c->addChildren(new CsgPlane);

    MO_PRINT("----\n" << root->toGlsl());

            c->addChildren(new CsgSphere);

    MO_PRINT("----\n" << root->toGlsl());


            auto c1 = new CsgUnion;
            c->addChildren(c1);

                c1->addChildren(new CsgPlane);
                c1->addChildren(new CsgSphere);
                c1->addChildren(new CsgSphere);

    MO_PRINT("----\n" << root->toGlsl());

    return 0;
}

} // namespace MO
