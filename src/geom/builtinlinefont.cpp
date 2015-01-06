/** @file builtinlinefont.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.12.2014</p>
*/

#include <vector>
#include <map>
#include <memory>

#include "builtinlinefont.h"
#include "io/error.h"

namespace MO {


class BuiltInLineFont::Private
{
public:

    /** Singelton instance */
    static BuiltInLineFont * instance()
    {
        static BuiltInLineFont * instance_ = 0;

        if (instance_ == 0)
            instance_ = new BuiltInLineFont();

        return instance_;
    }

    void createData();
    void makeInterface();

    struct Font
    {
        /** The public interface */
        BuiltInLineFont::Font interface;

        /** 4 coords == one line == two vertices */
        std::vector<float> verts;
    };

    std::map<int16_t, std::shared_ptr<Font>> fonts;
};


BuiltInLineFont::BuiltInLineFont()
    : p_    (new Private())
{
    p_->createData();
    p_->makeInterface();
}

BuiltInLineFont::~BuiltInLineFont()
{
    delete p_;
}

const BuiltInLineFont::Font * BuiltInLineFont::getFont(uint16_t utf16)
{
    auto B = Private::instance()->p_;

    auto i = B->fonts.find(utf16);
    if (i == B->fonts.end())
        return 0;

    return &i->second.get()->interface;
}




void BuiltInLineFont::Private::createData()
{
    // ----- create the vertices --------
    // (as GL_LINES style, e.g. two vertices - one line)

    #define FONT_ALLOC(char__, unused__) \
        MO_ASSERT( fonts.find(char__) == fonts.end(), "duplicate font character " << char__ ); \
        font = new Font(); \
        fonts.insert(std::make_pair(char__, std::shared_ptr<Font>(font)));

    // adds a line
    #define FONT_ADD(x1,y1, x2,y2) \
        font->verts.push_back(x1); \
        font->verts.push_back((y1)*(scaley)); \
        font->verts.push_back(x2); lastx = (x2); \
        font->verts.push_back((y2)*(scaley)); lasty = ((y2)*(scaley));

    // adds a line connecting to the previous end
    #define FONT_CON(x2,y2) \
        font->verts.push_back(lastx); \
        font->verts.push_back(lasty); \
        font->verts.push_back(x2); lastx = (x2); \
        font->verts.push_back((y2)*(scaley)); lasty = ((y2)*(scaley));

    Font * font;
    float lastx, lasty,
          scaley = 1.f;

    FONT_ALLOC('.', 1);
    FONT_ADD(0.5,0, 0.5,0.1);

    FONT_ALLOC(',', 1);
    FONT_ADD(0.5,-0.2, 0.5,0.3);

    FONT_ALLOC('+', 2);
    FONT_ADD(0.2,0.5, 0.8,0.5);
    FONT_ADD(0.5,0.2, 0.5,0.8);

    FONT_ALLOC('-', 1);
    FONT_ADD(0.2,0.5, 0.8,0.5);

    FONT_ALLOC('*', 4);
    FONT_ADD(0.2,0.5, 0.8,0.5);
    FONT_ADD(0.5,0.2, 0.5,0.8);
    FONT_ADD(0.25,0.25, 0.75,0.75);
    FONT_ADD(0.25,0.75, 0.75,0.25);

    FONT_ALLOC('/', 1);
    FONT_ADD(0,0, 1,1);

    FONT_ALLOC('\\', 1);
    FONT_ADD(0,1, 1,0);

    FONT_ALLOC(':', 2);
    FONT_ADD(0,0.2, 0,0.3);
    FONT_ADD(0,0.7, 0,0.8);

    FONT_ALLOC('|', 2);
    FONT_ADD(0.5,0, 0.5,0.4);
    FONT_ADD(0.5,0.6, 0.5,1);

    FONT_ALLOC(';', 2);
    FONT_ADD(0,-0.2, 0,0.3);
    FONT_ADD(0,0.7, 0,0.8);

    FONT_ALLOC('(', 3);
    FONT_ADD(0.625,1, 0.4,0.75);
    FONT_CON(0.4,0.25);
    FONT_CON(0.625, 0);

    FONT_ALLOC(')', 3);
    FONT_ADD(0.325,1, 0.6,0.75);
    FONT_CON(0.6,0.25);
    FONT_CON(0.325, 0);

    FONT_ALLOC('{', 6);
    FONT_ADD(0.625,1, 0.4,0.75);
    FONT_CON(0.4,0.6);
    FONT_CON(0.1,0.5);
    FONT_CON(0.4,0.4);
    FONT_CON(0.4,0.25);
    FONT_CON(0.625, 0);

    FONT_ALLOC('}', 6);
    FONT_ADD(0.325,1, 0.6,0.75);
    FONT_CON(0.6,0.6);
    FONT_CON(0.9,0.5);
    FONT_CON(0.6,0.5);
    FONT_CON(0.6,0.25);
    FONT_CON(0.325, 0);

    FONT_ALLOC('#', 4);
    FONT_ADD(0,0.25, 1,0.25);
    FONT_ADD(0,0.75, 1,0.75);
    FONT_ADD(0.25,0, 0.25,1);
    FONT_ADD(0.75,0, 0.75,1);

    FONT_ALLOC('\"', 2);
    FONT_ADD(0.25,0.7, 0.25,1);
    FONT_ADD(0.75,0.7, 0.75,1);

    FONT_ALLOC('<', 2);
    FONT_ADD(0.7,0.8, 0.3,0.5);
    FONT_CON(0.7,0.2);

    FONT_ALLOC('>', 2);
    FONT_ADD(0.3,0.8, 0.7,0.5);
    FONT_CON(0.3,0.2);

    // 0
    FONT_ALLOC(48, 9);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_ADD(0.75,1, 0.25,1);
    FONT_ADD(0.25,1, 0,0.75);
    FONT_ADD(0,0.75, 0,0.25);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 1,0.75);
    FONT_ADD(0.125,0.125, 0.875,0.875);

    // 1
    FONT_ALLOC(49, 2);
    FONT_ADD(0.5,0, 0.5,1);
    FONT_CON(0.2,0.7);

    // 2
    FONT_ALLOC(50, 6);
    FONT_ADD(0,0.75, 0.25,1);
    FONT_ADD(0.25,1, 0.75,1);
    FONT_ADD(0.75,1, 1,0.75);
    FONT_ADD(1,0.75, 0,0.25);
    FONT_ADD(0,0.25, 0,0);
    FONT_ADD(0,0, 1,0);

    // 3
    FONT_ALLOC(51, 9);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0.4,0.5);
    FONT_ADD(0.75,0.5, 1,0.75);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_ADD(0.75,1, 0.25,1);
    FONT_ADD(0.25,1, 0,0.75);

    // 4
    FONT_ALLOC(52,4);
    FONT_ADD(0.5,0, 0.5,0.7);
    FONT_ADD(0.25,1, 0,0.75);
    FONT_ADD(0,0.75, 0,0.5);
    FONT_ADD(0,0.5, 1,0.5);

    // 5
    FONT_ALLOC(53, 8);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0,0.5);
    FONT_ADD(0,0.5, 0,1);
    FONT_ADD(0,1, 1,1);
    FONT_ADD(1,1, 1,0.8);

    // 6
    FONT_ALLOC(54, 9);
    FONT_ADD(0.75,1, 0.25,1);
    FONT_ADD(0.25,1, 0,0.75);
    FONT_ADD(0,0.75, 0,0.25);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0.25,0.5);
    FONT_ADD(0.25,0.5, 0,0.25);

    // 7
    FONT_ALLOC(55, 5);
    FONT_ADD(0.5,0, 0.5,0.25);
    FONT_ADD(0.5,0.25, 1,0.75);
    FONT_ADD(1,0.75, 1,1);
    FONT_ADD(1,1, 0,1);
    FONT_ADD(0,1, 0,0.8);

    // 8
    FONT_ALLOC(56, 11);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0.25,0.5);
    FONT_ADD(0.25,0.5, 0,0.25);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.75,0.5, 1,0.75);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_ADD(0.75,1, 0.25,1);
    FONT_ADD(0.25,1, 0,0.75);
    FONT_ADD(0,0.75, 0.25,0.5);

    // 9
    FONT_ALLOC(57, 9);
    FONT_ADD(1,0.75, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0.25,0.5);
    FONT_ADD(0.25,0.5, 0,0.75);
    FONT_ADD(0,0.75, 0.25,1);
    FONT_ADD(0.25,1, 0.75,1);
    FONT_ADD(0.75,1, 1,0.75);
    FONT_ADD(1,0.75, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0);
    FONT_ADD(0.75,0, 0.25,0);

    // A
    FONT_ALLOC(65, 6);
    FONT_ADD(0,0, 0,0.75);
    FONT_ADD(0,0.75, 0.25,1);
    FONT_ADD(0.25,1, 0.75,1);
    FONT_ADD(0.75,1, 1,0.75);
    FONT_ADD(1,0.75, 1,0);
    FONT_ADD(0,0.5, 1,0.5);

    // B
    FONT_ALLOC(66, 8);
    FONT_ADD(0,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);
    FONT_ADD(1,0.25, 0.75,0.5);
    FONT_ADD(0.75,0.5, 0,0.5);
    FONT_ADD(0.75,0.5, 1,0.75);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_ADD(0.75,1, 0,1);
    FONT_ADD(0,1, 0,0);

    // C
    FONT_ALLOC(67, 7);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_ADD(0.75,1, 0.25,1);
    FONT_ADD(0.25,1, 0,0.75);
    FONT_ADD(0,0.75, 0,0.25);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_ADD(0.25,0, 0.75,0);
    FONT_ADD(0.75,0, 1,0.25);

    // D
    FONT_ALLOC(68, 6);
    FONT_ADD(0,0, 0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0,1);
    FONT_CON(0,0);

    // E
    FONT_ALLOC(69, 4);
    FONT_ADD(1,1, 0,1);
    FONT_CON(0,0);
    FONT_CON(1,0);
    FONT_ADD(0,0.5, 0.6,0.5);

    // F
    FONT_ALLOC(70, 3);
    FONT_ADD(0,0, 0,1);
    FONT_CON(1,1);
    FONT_ADD(0,0.5, 0.6,0.5);

    // G
    FONT_ALLOC(71, 9);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.5);
    FONT_CON(0.5,0.5);

    // H
    FONT_ALLOC(72, 3);
    FONT_ADD(0,0, 0,1);
    FONT_ADD(1,0, 1,1);
    FONT_ADD(0,0.5, 1,0.5);

    // I
    FONT_ALLOC(73, 3);
    FONT_ADD(0.5,0, 0.5,1);
    FONT_ADD(0.4,0, 0.6,0);
    FONT_ADD(0.4,1, 0.6,1);

    // J
    FONT_ALLOC(74, 5);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,1);
    FONT_CON(0,1);

    // K
    FONT_ALLOC(75, 4);
    FONT_ADD(0,0, 0,1);
    FONT_ADD(0,0.5, 0.5,0.5);
    FONT_CON(1,1);
    FONT_ADD(0.5,0.5, 1,0);

    // L
    FONT_ALLOC(76, 2);
    FONT_ADD(0,1, 0,0);
    FONT_CON(1,0);

    // M
    FONT_ALLOC(77, 4);
    FONT_ADD(0,0, 0,1);
    FONT_CON(0.5,0.5);
    FONT_CON(1,1);
    FONT_CON(1,0);

    // N
    FONT_ALLOC(78, 3);
    FONT_ADD(0,0, 0,1);
    FONT_CON(1,0);
    FONT_CON(1,1);

    // O
    FONT_ALLOC(79, 8);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);

    // P
    FONT_ALLOC(80, 5);
    FONT_ADD(0,0, 0,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(0.75,0.5);
    FONT_CON(0,0.5);

    // Q
    FONT_ALLOC(81, 9);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);
    FONT_ADD(0.75,0.25, 1,0);

    // R
    FONT_ALLOC(82, 7);
    FONT_ADD(0,0, 0,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(0.75,0.5);
    FONT_CON(0,0.5);
    FONT_ADD(0.75,0.5, 1,0.25);
    FONT_CON(1,0);

    // S
    FONT_ALLOC(83, 9);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(0.75,0.5);
    FONT_CON(0.25,0.5);
    FONT_CON(0,0.75);
    FONT_CON(0.25,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);

    // T
    FONT_ALLOC(84, 2);
    FONT_ADD(0,1, 1,1);
    FONT_ADD(0.5,0, 0.5,1);

    // U
    FONT_ALLOC(85, 5);
    FONT_ADD(0,1, 0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,1);

    // V
    FONT_ALLOC(86, 4);
    FONT_ADD(0,1, 0,0.5);
    FONT_CON(0.5,0);
    FONT_CON(1,0.5)
    FONT_CON(1,1);

    // W
    FONT_ALLOC(87, 4);
    FONT_ADD(0,1, 0,0);
    FONT_CON(0.5,0.5);
    FONT_CON(1,0);
    FONT_CON(1,1);

    // X
    FONT_ALLOC(88, 2);
    FONT_ADD(0,0, 1,1);
    FONT_ADD(0,1, 1,0);

    // Y
    FONT_ALLOC(89, 3);
    FONT_ADD(0,1, 0.5,0.5);
    FONT_CON(1,1);
    FONT_ADD(0.5,0.5, 0.5,0);

    // Z
    FONT_ALLOC(90, 3);
    FONT_ADD(0,1, 1,1);
    FONT_CON(0,0);
    FONT_CON(1,0);


    // small letters
    scaley *= 0.5;
    float ov = 1.5;

    FONT_ALLOC('a', 8);
    FONT_ADD(0,1, 0.25,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(1,0);
    FONT_ADD(1,0.5, 0.25,0.5);
    FONT_CON(0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(1,0);

    FONT_ALLOC('b', 6);
    FONT_ADD(0,ov, 0,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0,1);

    FONT_ALLOC('c', 7);
    FONT_ADD(1,0.75, 0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);

    FONT_ALLOC('d', 6);
    FONT_ADD(1,ov, 1,0);
    FONT_CON(0.25,0);
    FONT_CON(0,0.25);
    FONT_CON(0,0.75);
    FONT_CON(0.25,1);
    FONT_CON(1,1);

    FONT_ALLOC('e', 8);
    FONT_ADD(0,0.5, 1,0.5);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.9,0);

    FONT_ALLOC('f', 3);
    FONT_ADD(0.4,0, 0.4,(ov-0.25));
    FONT_CON(0.7,ov);
    FONT_ADD(0.2,0.5, 0.7,0.5);

    FONT_ALLOC('g', 10);
    FONT_ADD(1,0.25, 0.75,0);
    FONT_CON(0.25,0);
    FONT_CON(0,0.25);
    FONT_CON(0,0.75);
    FONT_CON(0.25,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(1,-0.25);
    FONT_CON(0.75,-0.5);
    FONT_CON(0.1,-0.5);

    FONT_ALLOC('h', 4);
    FONT_ADD(0,0, 0,ov);
    FONT_ADD(0,1, 0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(1,0);

    FONT_ALLOC('i', 4);
    FONT_ADD(0.5,0, 0.5,1);
    FONT_ADD(0.4,0, 0.6,0);
    FONT_ADD(0.4,1, 0.6,1);
    FONT_ADD(0.5,ov, 0.5,(ov-0.2));

    FONT_ALLOC('j', 5);
    FONT_ADD(0.5,1, 0.5,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0,0.25);
    FONT_ADD(0.4,1, 0.6,1);
    FONT_ADD(0.5,ov, 0.5,(ov-0.2));

    FONT_ALLOC('k', 4);
    FONT_ADD(0,0, 0,ov);
    FONT_ADD(0,0.5, 0.5,0.5);
    FONT_CON(1,1);
    FONT_ADD(0.5,0.5, 1,0);

    FONT_ALLOC('l', 3);
    FONT_ADD(0.4,ov, 0.4,0.25);
    FONT_CON(0.65,0);
    FONT_CON(0.85,0);

    FONT_ALLOC('m', 7);
    FONT_ADD(0,0, 0,1);
    FONT_ADD(0,0.75, 0.25,1);
    FONT_CON(0.5,0.75);
    FONT_CON(0.5,0);
    FONT_ADD(0.5,0.75, 0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(1,0);

    FONT_ALLOC('n', 4);
    FONT_ADD(0,0, 0,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);
    FONT_CON(1,0);

    FONT_ALLOC('o', 8);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);

    FONT_ALLOC('p', 6);
    FONT_ADD(0,0, 0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0,1);
    FONT_CON(0,-0.5);

    FONT_ALLOC('q', 9);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,0.75);
    FONT_CON(0.75,1);
    FONT_CON(0.25,1);
    FONT_CON(0,0.75);
    FONT_CON(0,0.25);
    FONT_ADD(1,0.25, 1,-0.5);

    FONT_ALLOC('r', 2);
    FONT_ADD(0.5,0, 0.5,1);
    FONT_ADD(0.5,0.75, 0.8,1);

    FONT_ALLOC('s', 9);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(0.75,0.5);
    FONT_CON(0.25,0.5);
    FONT_CON(0,0.75);
    FONT_CON(0.25,1);
    FONT_CON(0.75,1);
    FONT_CON(1,0.75);

    FONT_ALLOC('t', 2);
    FONT_ADD(0.5,0, 0.5,ov);
    FONT_ADD(0.2,1, 0.8,1);

    FONT_ALLOC('u', 5);
    FONT_ADD(0,1, 0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,1);

    FONT_ALLOC('v', 4);
    FONT_ADD(0,1, 0,0.5);
    FONT_CON(0.5,0);
    FONT_CON(1,0.5)
    FONT_CON(1,1);

    FONT_ALLOC('w', 8);
    FONT_ADD(0,0, 0,1);
    FONT_ADD(0,0.25, 0.25,0);
    FONT_CON(0.5,0.25);
    FONT_CON(0.5,1);
    FONT_CON(0.5,0.25);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,1);

    FONT_ALLOC('x', 2);
    FONT_ADD(0,0, 1,1);
    FONT_ADD(0,1, 1,0);

    FONT_ALLOC('y', 8);
    FONT_ADD(0,1, 0,0.25);
    FONT_CON(0.25,0);
    FONT_CON(0.75,0);
    FONT_CON(1,0.25);
    FONT_CON(1,1);
    FONT_ADD(1,0.25, 1,-0.25)
    FONT_CON(0.75,-0.5);
    FONT_CON(0.1,-0.5);

    FONT_ALLOC('z', 3);
    FONT_ADD(0,1, 1,1);
    FONT_CON(0,0);
    FONT_CON(1,0);

#undef FONT_CON
#undef FONT_ADD
#undef FONT_ALLOC
}

void BuiltInLineFont::Private::makeInterface()
{
    for (auto i = fonts.begin(); i != fonts.end(); ++i)
    {
        Font * f = i->second.get();

        // copy data to public interface

        f->interface.utf16 = i->first;
        f->interface.num = f->verts.size() / 4;
        f->interface.data = &f->verts[0];

        // add some padding to the [0,1] rect
        const float
                pad = 0.075f,
                sc = 1.f - 2.f * pad;
        for (uint i=0; i < f->verts.size(); ++i)
        {
            f->verts[i] = pad + sc * f->verts[i];
        }

    }
}












/*
    // get size through left and right extents
    for (int i=0;i<maxChar;i++)
    {
        // init
        GLfloat minx = 1, maxx = 0;
        // match
        for (GLuint j=0;j<font_Nr[i];j++)
        {
            minx = std::min(minx, font_Pos[i][j*2]);
            maxx = std::max(maxx, font_Pos[i][j*2]);
        }

        // store
        data.width[i] = std::max((GLfloat)0, maxx-minx + (GLfloat)0.1);
        data.height[i] = 1.0;
        font_Xoff[i] = minx;
    }

    // --- transfer to display lists ---
    for (int i=0;i<maxChar;i++)
    if (font_Nr[i])
    {
        data.displayList[i] = gl->recordList(false);

        // draw font
        glBegin(GL_LINES);
        for (GLuint j=0;j<font_Nr[i];j++)
        {
            glVertex2f(
                (font_Pos[i][j*2] - font_Xoff[i]) * 0.8,
                font_Pos[i][j*2+1]);
        }
        glEnd();

        gl->endList();
    }

    // --- transfer italics to display lists ---
    for (int i=0;i<maxChar;i++)
    if (font_Nr[i])
    {
        data.displayList[i+maxChar] = gl->recordList(false);

        // draw font
        glBegin(GL_LINES);
        for (GLuint j=0;j<font_Nr[i];j++)
        {
            glVertex2f(
                (font_Pos[i][j*2] - font_Xoff[i]
                    + font_Pos[i][j*2+1]*0.5) * 0.8,
                font_Pos[i][j*2+1]);
        }
        glEnd();

        gl->endList();
    }
*/



} // namespace MO
