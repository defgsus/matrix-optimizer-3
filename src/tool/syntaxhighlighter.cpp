/** @file syntaxhighlighter.cpp

    @brief Generic syntax highlighter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT
#include <angelscript.h>
#endif

#include <QDebug>

#include "syntaxhighlighter.h"
//#include "io/log.h"

namespace MO {

SyntaxHighlighter::SyntaxHighlighter(QObject * parent) :
    QSyntaxHighlighter  (parent)
{
    // setup multiline comments
    commentStartExpression_ = QRegExp("/\\*");
    commentEndExpression_ = QRegExp("\\*/");
    commentFormat_.setForeground(QBrush(QColor(140,140,140)));
    markFormat_.setForeground(QBrush(Qt::white));
    markFormat_.setBackground(QBrush(Qt::darkGreen));
}


void SyntaxHighlighter::setNames(const QStringList &variables,
                                 const QStringList &functions,
                                 const QStringList &types,
                                 const QStringList &reserved)
{
    //qDebug() << "SyntaxHighligher::setNames:\nvars " << variables << "\nfuncs " << functions;

    rules_.clear();

    HighlightingRule rule;

    // -- styles for each category --

    QTextCharFormat
        functionFormat,
        variableFormat,
        reservedFormat,
        typesFormat;

    // functions
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(QBrush(QColor(200,200,210)));
    // variables
    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(QBrush(QColor(200,210,200)));
    // variables
    typesFormat.setFontWeight(QFont::Bold);
    typesFormat.setForeground(QBrush(QColor(200,210,210)));
    // reserved words
    reservedFormat.setFontWeight(QFont::Bold);
    reservedFormat.setForeground(QBrush(QColor(200,200,200)));

    // create rules for each reserved word

    for (auto &k : variables)
    {
        // match whole-word with word boundaries
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = variableFormat;
        rules_.append(rule);
    }

    for (auto &k : functions)
    {
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = functionFormat;
        rules_.append(rule);
    }

    for (auto &k : types)
    {
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = typesFormat;
        rules_.append(rule);
    }

    for (auto &k : reserved)
    {
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = reservedFormat;
        rules_.append(rule);
    }

    // single line comments
    // [They are at the end here,
    //  so keywords in comments will not be highlighted.]
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = commentFormat_;
    rules_.append(rule);

}

void SyntaxHighlighter::setMarkText(const QString& text)
{
    if (text.isEmpty())
        setMarkText(QStringList());
    else
        setMarkText(QStringList() << text);
}

void SyntaxHighlighter::setMarkText(const QStringList& texts)
{
    markRules_.clear();

    for (const QString& text : texts)
    {
        HighlightingRule rule;
        rule.pattern = QRegExp( "\\b" + text + "\\b" );
        markRules_.append(rule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // apply rules
    for (const HighlightingRule& rule : rules_)
    {
        QRegExp expression(rule.pattern);
        // find the string
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            // set format
            setFormat(index, length, rule.format);
            // find again
            index = expression.indexIn(text, index + length);
        }
    }


    // --- multiline comments ---

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression_.indexIn(text);

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression_.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                            + commentEndExpression_.matchedLength();
        }
        setFormat(startIndex, commentLength, commentFormat_);
        startIndex = commentStartExpression_.indexIn(text, startIndex + commentLength);
    }


    // ---- marked text ----

    for (const HighlightingRule& rule : markRules_)
    {
        QRegExp expression(rule.pattern);
        // find the string
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            // set format
            setFormat(index, length, markFormat_);
            // find again
            index = expression.indexIn(text, index + length);
        }
    }
}





void SyntaxHighlighter::initForStyleSheet()
{
    QStringList vars;
    vars
            << "color"
            << "background-color"
            << "subcontrol-origin"
            << "selection-color"
            << "selection-background-color"
            << "border"
            << "image"
            << "margin"
            << "padding"
            << "position"
            << "top"
            << "left"
               ;

    QStringList selects;
    selects
            << ":hover"
            << ":checked"
            << ":pressed"
            << ":enabled"
            << ":active"
            << ":alternate"
            << "::drop-down"
            << "::up-arrow"
            << "::down-arrow"
            << "::item"
            << "GroupHeader="
               ;

    setNames(vars, selects);
}


#ifndef MO_DISABLE_ANGELSCRIPT

void SyntaxHighlighter::initForAngelScript(asIScriptModule * mod)
{
    QStringList vars, funcs, types, reserved;

    asIScriptEngine * engine = mod->GetEngine();

    for (asUINT i=0; i<engine->GetEnumCount(); ++i)
    {
        int typeId;
        types << QString( engine->GetEnumByIndex(i, &typeId) );

        for (int j=0; j < engine->GetEnumValueCount(typeId); ++j)
            vars << QString( engine->GetEnumValueByIndex(typeId, j, 0) );
    }

    for (asUINT i=0; i<mod->GetGlobalVarCount(); ++i)
    {
        const char * name;
        mod->GetGlobalVar(i, &name);
        vars << QString( name );
    }

    for (asUINT i=0; i<mod->GetObjectTypeCount(); ++i)
    {
        types << QString( mod->GetObjectTypeByIndex(i)->GetName() );
    }

    for (asUINT i=0; i<engine->GetObjectTypeCount(); ++i)
    {
        types << QString( engine->GetObjectTypeByIndex(i)->GetName() );
    }

    for (asUINT i=0; i<engine->GetGlobalPropertyCount(); ++i)
    {
        const char * name;
        //bool isConst;
        engine->GetGlobalPropertyByIndex(i, &name);//, 0, 0, &isConst);
        vars << name;
    }


    for (asUINT i=0; i<mod->GetTypedefCount(); ++i)
    {
        types << QString( mod->GetTypedefByIndex(i, 0) );
    }

    for (asUINT i=0; i<mod->GetFunctionCount(); ++i)
    {
        funcs << QString( mod->GetFunctionByIndex(i)->GetName() );
    }

    for (asUINT i=0; i < engine->GetGlobalFunctionCount(); ++i)
    {
        funcs << QString( engine->GetGlobalFunctionByIndex(i)->GetName() );
    }

    // -- reserved words --

    types << "void" << "float" << "double" << "bool"
          << "int" << "int8" << "int16" << "int" << "int64"
          << "uint" << "uint8" << "uint16" << "uint" << "uint64";

    reserved << "and" << "abstract" << "auto" << "break"
            << "case" << "cast" << "class" << "const" << "continue" << "default"
            << "do" << "else" << "enum" << "false" << "final" << "for" << "from"
            << "funcdef" << "get" << "if" << "import" << "in" << "inout" << "interface"
            << "is" << "mixin" << "namespace" << "not" << "null" << "or" << "out"
            << "override" << "private" << "return" << "set" << "shared" << "super"
            << "switch" << "this" << "true" << "typedef" << "while" << "xor";

    // all operator overloads
    funcs << "opNeg" << "opCom" /* ~ */ << "opPreInc" << "opPreDec" << "opPostInc" << "opPostDec"
          << "opEquals" << "opEquals" << "opCmp"
          << "opAssign" << "opAddAssign" << "opSubAssign" << "opMulAssign" << "opDivAssign"
          << "opPowAssign" << "opModAssign" << "opAndAssign" << "opOrAssign" << "opXorAssign"
          << "opShlAssign" << "opShrAssign" << "opUShrAssign"
          << "opAdd" << "opSub" << "opMul" << "opDiv" << "opMod" << "opPow"
          << "opAnd" << "opOr" << "opXor" << "opShl" << "opShr" << "opUShr"
          << "opAdd_r" << "opSub_r" << "opMul_r" << "opDiv_r" << "opMod_r" << "opPow_r"
          << "opAnd_r" << "opOr_r" << "opXor_r" << "opShl_r" << "opShr_r" << "opUShr_r"
          << "opIndex" << "opCall" << "opConv" << "opImplConv";

    setNames(vars, funcs, types, reserved);
}

#endif


void SyntaxHighlighter::initForGlsl()
{
    /* Following are all the keywords and variable names of GLSL.
     * TODO: Would be great to divide them into different GLSL versions.
     *       Right now, they are from version 1 to 4
     */

    /* http://www.opengl.org/registry/doc/GLSLangSpec.4.10.6.clean.pdf */
    static const QStringList glsl_types =
    {
        "float", "double", "int", "void", "bool", "true", "false",
        "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4",
        "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4",
        "mat3x2", "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4",
        "mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4",
        "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4",
        "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4",
        "uint", "uvec2", "uvec3", "uvec4",
        "lowp", "mediump", "highp", "precision",
        "sampler1D", "sampler2D", "sampler3D", "samplerCube",
        "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow",
        "sampler1DArray", "sampler2DArray",
        "sampler1DArrayShadow", "sampler2DArrayShadow",
        "isampler1D", "isampler2D", "isampler3D", "isamplerCube",
        "isampler1DArray", "isampler2DArray",
        "usampler1D", "usampler2D", "usampler3D", "usamplerCube",
        "usampler1DArray", "usampler2DArray",
        "sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect",
        "samplerBuffer", "isamplerBuffer", "usamplerBuffer",
        "sampler2DMS", "isampler2DMS", "usampler2DMS",
        "sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray",
        "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray",
        "struct"
    };

    static const QStringList glsl_reserved =
    {
        "attribute", "const", "uniform", "varying", "layout",
        "centroid", "flat", "smooth", "noperspective", "patch",
        "sample", "break", "continue", "do", "for", "while", "switch",
        "case", "default", "if", "else", "subroutine", "in", "out", "inout",
        "invariant", "discard", "return",
        "common", "partition", "active",
        "asm",
        "class", "union", "enum", "typedef", "template", "this", "packed",
        "goto", "inline", "noinline", "volatile", "public", "static", "extern",
        "external", "interface",
        "long", "short", "half", "fixed", "unsigned", "superp",
        "input", "output",
        "hvec2", "hvec3", "hvec4", "fvec2", "fvec3", "fvec4",
        "sampler3DRect",
        "filter",
        "image1D", "image2D", "image3D", "imageCube",
        "iimage1D", "iimage2D", "iimage3D", "iimageCube",
        "uimage1D", "uimage2D", "uimage3D", "uimageCube",
        "image1DArray", "image2DArray",
        "iimage1DArray", "iimage2DArray",
        "uimage1DArray", "uimage2DArray",
        "image1DShadow", "image2DShadow",
        "image1DArrayShadow", "image2DArrayShadow",
        "imageBuffer", "iimageBuffer", "uimageBuffer",
        "sizeof", "cast",
        "namespace", "using",
        "row_major",
        "define", "if", "ifdef", "ifndef", "endif",
        "include"
    };


    static const QStringList glsl_functions =
    {
        "radians", "degrees", "sin", "cos", "tan", "asin", "acos", "atan",
        "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
        "pow", "exp", "log", "exp2", "log2", "sqrt", "inversesqrt",
        "abs", "sign", "floor", "trunc", "round", "roundEven", "ceil", "fract", "mod",
        "modf", "min", "max", "clamp", "mix", "step", "smoothstep", "isnan", "isinf",
        "floatBitsToInt", "floatBitsToUint", "intBitsToFloat", "uintBitsToFloat",
        "fma", "frexp", "ldexp",
        "packUnorm2x16", "packUnorm4x8", "packSnorm4x8",
        "unpackUnorm2x16", "unpackUnorm4x8", "unpackSnorm4x8",
        "packDouble2x32", "unpackDouble2x32",
        "length", "distance", "dot", "cross", "normalize", "ftransform",
        "faceforward", "reflect", "refract",
        "matrixCompMult", "outerProduct", "transpose", "determinant", "inverse",
        "lessThan", "lessThanEqual", "greaterThan", "greaterThanEqual",
        "equal", "notEqual", "any", "all", "not",
        "uaddCarry", "usubBorrow", "umulExtended", "imulExtended",
        "bitfieldExtract", "bitfieldInsert", "bitfieldReverse", "bitCount", "findLSB", "findMSB",
        "textureSize", "textureQueryLod", "textureQueryLevels",
        "texture", "textureProj", "textureLod",
        "textureOffset", "texelFetch", "texelFetchOffset", "textureProjOffset", "textureLodOffset",
        "textureProjLod", "textureProjLodOffset", "textureGrad", "textureGradOffset",
        "textureProjGrad", "textureProjGradOffset",
        "textureGather", "textureGatherOffset", "textureGatherOffsets",
        "texture1D", "texture1DProj", "texture1DLod", "texture1DProjLod",
        "texture2D", "texture2DProj", "texture2DLod", "texture2DProjLod",
        "texture3D", "texture3DProj", "texture3DLod", "texture3DProjLod",
        "textureCube", "textureCubeLod",
        "shadow1D", "shadow1DProj", "shadow1DLod", "shadow1DProjLod",
        "shadow2D", "shadow2DProj", "shadow2DLod", "shadow2DProjLod",
        "atomicCounterIncrement", "atomicCounterDecrement", "atomicCounter",
        "atomicAdd", "atomicMin", "atomicMax", "atomicAnd", "atomicOr", "atomicXor",
        "atomicExchange", "atomicCompSwap",
        "imageSize", "imageLoad", "imageStore",
        "imageAtomicAdd", "imageAtomicMin", "imageAtomicMax", "imageAtomicAnd", "imageAtomicOr", "imageAtomicXor",
        "dFdx", "dFdy", "fwidth",
        "interpolateAtCentroid", "interpolateAtSample", "interpolateAtOffset",
        "noise1", "noise2", "noise3", "noise4",
        "EmitStreamVertex", "EndStreamPrimitive", "EmitVertex", "EndPrimitive",
        "barrier", "memoryBarrier", "memoryBarrierAtomicCounter", "memoryBarrierBuffer",
        "memoryBarrierShared", "memoryBarrier", "memoryBarrierImage", "groupMemoryBarrier"
    };


    /* https://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29 */
    static const QStringList glsl_variables =
    {
        "gl_VertexID", "gl_InstanceID",
        "gl_PerVertex",
        "gl_Position", "gl_PointSize", "gl_ClipDistance",
        "gl_Vertex",
        "gl_PatchVerticesIn", "gl_PrimitiveID", "gl_InvocationID",
        "gl_MaxPatchVertices",
        "gl_TessLevelOuter", "gl_TessLevelInner", "gl_TessCoord",
        "gl_PrimitiveIDIn",
        "gl_Layer", "gl_ViewportIndex",
        "gl_FragCoord", "gl_FrontFacing", "gl_PointCoord",
        "gl_SampleID", "gl_SamplePosition", "gl_SampleMask", "gl_SampleMaskIn",
        "gl_DepthRange",
        "gl_FragColor", "gl_FragData", "gl_FragDepth",
        "gl_Normal", "gl_Color", "gl_SecondaryColor",
        "gl_NormalScale", "gl_ClipPlane",
        "gl_PointParamaters", "gl_MaterialParameters", "gl_LightSourceParameters",
        "gl_LightModelParameters", "gl_LightModelProducts", "gl_LightProducts",
        "gl_FogParameters",
        "gl_FogCoord",
        "gl_FrontColor", "gl_BackColor", "gl_FrontSecondaryColor", "gl_BackSecondaryColor",
        "gl_TexCoord", "gl_FogFragCoord",
        "gl_ModelViewMatrix", "gl_ProjectionMatrix", "gl_ModelViewProjectionMatrix",
        "gl_TextureMatrix", "gl_NormalMatrix",
        "gl_ModelViewMatrixInverse", "gl_ProjectionMatrixInverse",
        "gl_ModelViewProjectionMatrixInverse", "gl_TextureMatrixInverse"
        "gl_ModelViewMatrixTranspose", "gl_ProjectionMatrixTranspose",
        "gl_ModelViewProjectionMatrixTranspose", "gl_TextureMatrixTranspose"
        "gl_ModelViewMatrixInverseTranspose", "gl_ProjectionMatrixInverseTranspose",
        "gl_ModelViewProjectionMatrixInverseTranspose", "gl_TextureMatrixInverseTranspose"
        "gl_MultiTexCoord0", "gl_MultiTexCoord1", "gl_MultiTexCoord2", "gl_MultiTexCoord3",
        "gl_MultiTexCoord4", "gl_MultiTexCoord5", "gl_MultiTexCoord6", "gl_MultiTexCoord7",
        // constants
        "gl_MaxTextureUnits", "gl_MaxVertexAttribs", "gl_MaxVertexUniformComponents",
        "gl_MaxVaryingFloats", "gl_MaxVaryingComponents", "gl_MaxVertextTextureImageUnits",
        "gl_MaxCombinedTextureImageUnits", "gl_MaxTextureImageUnits", "gl_MaxFragmentUniformComponents",
        "gl_MaxDrawBuffers", "gl_MaxClipDistances",
        "gl_MaxClipPlanes", "gl_MaxTextureCoords",

    };

    setNames(QStringList(), glsl_functions, glsl_types, glsl_reserved);
}

#ifdef MO_ENABLE_PYTHON34
void SyntaxHighlighter::initForPython(const PYTHON34::PythonInterpreter*)
{
    static const QStringList python_reserved =
    {
        "False"      , "class"      , "finally"    , "is"         , "return"
        "None"       , "continue"   , "for"        , "lambda"     , "try"
        "True"       , "def"        , "from"       , "nonlocal"   , "while"
        "and"        , "del"        , "global"     , "not"        , "with"
        "as"         , "elif"       , "if"         , "or"         , "yield"
        "assert"     , "else"       , "import"     , "pass"
        "break"      , "except"     , "in"         , "raise"
    };

    setNames(QStringList(), QStringList(), QStringList(), python_reserved);
}

#endif


} // namespace MO
