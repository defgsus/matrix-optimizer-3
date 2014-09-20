/** @file opengl_undef.h

    @brief undefines opengl defines...

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

/* This file should follow all includes of Qt's opengl headers,
 * like QOpenGLWidget and so on.
 * The opengl defines break glbinding code..
 */



/* Boolean values */
#undef GL_FALSE
#undef GL_TRUE

/* Data types */
#undef GL_BYTE
#undef GL_UNSIGNED_BYTE
#undef GL_SHORT
#undef GL_UNSIGNED_SHORT
#undef GL_INT
#undef GL_UNSIGNED_INT
#undef GL_FLOAT
#undef GL_2_BYTES
#undef GL_3_BYTES
#undef GL_4_BYTES
#undef GL_DOUBLE

/* Primitives */
#undef GL_POINTS
#undef GL_LINES
#undef GL_LINE_LOOP
#undef GL_LINE_STRIP
#undef GL_TRIANGLES
#undef GL_TRIANGLE_STRIP
#undef GL_TRIANGLE_FAN
#undef GL_QUADS
#undef GL_QUAD_STRIP
#undef GL_POLYGON

/* Vertex Arrays */
#undef GL_VERTEX_ARRAY
#undef GL_NORMAL_ARRAY
#undef GL_COLOR_ARRAY
#undef GL_INDEX_ARRAY
#undef GL_TEXTURE_COORD_ARRAY
#undef GL_EDGE_FLAG_ARRAY
#undef GL_VERTEX_ARRAY_SIZE
#undef GL_VERTEX_ARRAY_TYPE
#undef GL_VERTEX_ARRAY_STRIDE
#undef GL_NORMAL_ARRAY_TYPE
#undef GL_NORMAL_ARRAY_STRIDE
#undef GL_COLOR_ARRAY_SIZE
#undef GL_COLOR_ARRAY_TYPE
#undef GL_COLOR_ARRAY_STRIDE
#undef GL_INDEX_ARRAY_TYPE
#undef GL_INDEX_ARRAY_STRIDE
#undef GL_TEXTURE_COORD_ARRAY_SIZE
#undef GL_TEXTURE_COORD_ARRAY_TYPE
#undef GL_TEXTURE_COORD_ARRAY_STRIDE
#undef GL_EDGE_FLAG_ARRAY_STRIDE
#undef GL_VERTEX_ARRAY_POINTER
#undef GL_NORMAL_ARRAY_POINTER
#undef GL_COLOR_ARRAY_POINTER
#undef GL_INDEX_ARRAY_POINTER
#undef GL_TEXTURE_COORD_ARRAY_POINTER
#undef GL_EDGE_FLAG_ARRAY_POINTER
#undef GL_V2F
#undef GL_V3F
#undef GL_C4UB_V2F
#undef GL_C4UB_V3F
#undef GL_C3F_V3F
#undef GL_N3F_V3F
#undef GL_C4F_N3F_V3F
#undef GL_T2F_V3F
#undef GL_T4F_V4F
#undef GL_T2F_C4UB_V3F
#undef GL_T2F_C3F_V3F
#undef GL_T2F_N3F_V3F
#undef GL_T2F_C4F_N3F_V3F
#undef GL_T4F_C4F_N3F_V4F

/* Matrix Mode */
#undef GL_MATRIX_MODE
#undef GL_MODELVIEW
#undef GL_PROJECTION
#undef GL_TEXTURE

/* Points */
#undef GL_POINT_SMOOTH
#undef GL_POINT_SIZE
#undef GL_POINT_SIZE_GRANULARITY
#undef GL_POINT_SIZE_RANGE

/* Lines */
#undef GL_LINE_SMOOTH
#undef GL_LINE_STIPPLE
#undef GL_LINE_STIPPLE_PATTERN
#undef GL_LINE_STIPPLE_REPEAT
#undef GL_LINE_WIDTH
#undef GL_LINE_WIDTH_GRANULARITY
#undef GL_LINE_WIDTH_RANGE

/* Polygons */
#undef GL_POINT
#undef GL_LINE
#undef GL_FILL
#undef GL_CW
#undef GL_CCW
#undef GL_FRONT
#undef GL_BACK
#undef GL_POLYGON_MODE
#undef GL_POLYGON_SMOOTH
#undef GL_POLYGON_STIPPLE
#undef GL_EDGE_FLAG
#undef GL_CULL_FACE
#undef GL_CULL_FACE_MODE
#undef GL_FRONT_FACE
#undef GL_POLYGON_OFFSET_FACTOR
#undef GL_POLYGON_OFFSET_UNITS
#undef GL_POLYGON_OFFSET_POINT
#undef GL_POLYGON_OFFSET_LINE
#undef GL_POLYGON_OFFSET_FILL

/* Display Lists */
#undef GL_COMPILE
#undef GL_COMPILE_AND_EXECUTE
#undef GL_LIST_BASE
#undef GL_LIST_INDEX
#undef GL_LIST_MODE

/* Depth buffer */
#undef GL_NEVER
#undef GL_LESS
#undef GL_EQUAL
#undef GL_LEQUAL
#undef GL_GREATER
#undef GL_NOTEQUAL
#undef GL_GEQUAL
#undef GL_ALWAYS
#undef GL_DEPTH_TEST
#undef GL_DEPTH_BITS
#undef GL_DEPTH_CLEAR_VALUE
#undef GL_DEPTH_FUNC
#undef GL_DEPTH_RANGE
#undef GL_DEPTH_WRITEMASK
#undef GL_DEPTH_COMPONENT

/* Lighting */
#undef GL_LIGHTING
#undef GL_LIGHT0
#undef GL_LIGHT1
#undef GL_LIGHT2
#undef GL_LIGHT3
#undef GL_LIGHT4
#undef GL_LIGHT5
#undef GL_LIGHT6
#undef GL_LIGHT7
#undef GL_SPOT_EXPONENT
#undef GL_SPOT_CUTOFF
#undef GL_CONSTANT_ATTENUATION
#undef GL_LINEAR_ATTENUATION
#undef GL_QUADRATIC_ATTENUATION
#undef GL_AMBIENT
#undef GL_DIFFUSE
#undef GL_SPECULAR
#undef GL_SHININESS
#undef GL_EMISSION
#undef GL_POSITION
#undef GL_SPOT_DIRECTION
#undef GL_AMBIENT_AND_DIFFUSE
#undef GL_COLOR_INDEXES
#undef GL_LIGHT_MODEL_TWO_SIDE
#undef GL_LIGHT_MODEL_LOCAL_VIEWER
#undef GL_LIGHT_MODEL_AMBIENT
#undef GL_FRONT_AND_BACK
#undef GL_SHADE_MODEL
#undef GL_FLAT
#undef GL_SMOOTH
#undef GL_COLOR_MATERIAL
#undef GL_COLOR_MATERIAL_FACE
#undef GL_COLOR_MATERIAL_PARAMETER
#undef GL_NORMALIZE

/* User clipping planes */
#undef GL_CLIP_PLANE0
#undef GL_CLIP_PLANE1
#undef GL_CLIP_PLANE2
#undef GL_CLIP_PLANE3
#undef GL_CLIP_PLANE4
#undef GL_CLIP_PLANE5

/* Accumulation buffer */
#undef GL_ACCUM_RED_BITS
#undef GL_ACCUM_GREEN_BITS
#undef GL_ACCUM_BLUE_BITS
#undef GL_ACCUM_ALPHA_BITS
#undef GL_ACCUM_CLEAR_VALUE
#undef GL_ACCUM
#undef GL_ADD
#undef GL_LOAD
#undef GL_MULT
#undef GL_RETURN

/* Alpha testing */
#undef GL_ALPHA_TEST
#undef GL_ALPHA_TEST_REF
#undef GL_ALPHA_TEST_FUNC

/* Blending */
#undef GL_BLEND
#undef GL_BLEND_SRC
#undef GL_BLEND_DST
#undef GL_ZERO
#undef GL_ONE
#undef GL_SRC_COLOR
#undef GL_ONE_MINUS_SRC_COLOR
#undef GL_SRC_ALPHA
#undef GL_ONE_MINUS_SRC_ALPHA
#undef GL_DST_ALPHA
#undef GL_ONE_MINUS_DST_ALPHA
#undef GL_DST_COLOR
#undef GL_ONE_MINUS_DST_COLOR
#undef GL_SRC_ALPHA_SATURATE

/* Render Mode */
#undef GL_FEEDBACK
#undef GL_RENDER
#undef GL_SELECT

/* Feedback */
#undef GL_2D
#undef GL_3D
#undef GL_3D_COLOR
#undef GL_3D_COLOR_TEXTURE
#undef GL_4D_COLOR_TEXTURE
#undef GL_POINT_TOKEN
#undef GL_LINE_TOKEN
#undef GL_LINE_RESET_TOKEN
#undef GL_POLYGON_TOKEN
#undef GL_BITMAP_TOKEN
#undef GL_DRAW_PIXEL_TOKEN
#undef GL_COPY_PIXEL_TOKEN
#undef GL_PASS_THROUGH_TOKEN
#undef GL_FEEDBACK_BUFFER_POINTER
#undef GL_FEEDBACK_BUFFER_SIZE
#undef GL_FEEDBACK_BUFFER_TYPE

/* Selection */
#undef GL_SELECTION_BUFFER_POINTER
#undef GL_SELECTION_BUFFER_SIZE

/* Fog */
#undef GL_FOG
#undef GL_FOG_MODE
#undef GL_FOG_DENSITY
#undef GL_FOG_COLOR
#undef GL_FOG_INDEX
#undef GL_FOG_START
#undef GL_FOG_END
#undef GL_LINEAR
#undef GL_EXP
#undef GL_EXP2

/* Logic Ops */
#undef GL_LOGIC_OP
#undef GL_INDEX_LOGIC_OP
#undef GL_COLOR_LOGIC_OP
#undef GL_LOGIC_OP_MODE
#undef GL_CLEAR
#undef GL_SET
#undef GL_COPY
#undef GL_COPY_INVERTED
#undef GL_NOOP
#undef GL_INVERT
#undef GL_AND
#undef GL_NAND
#undef GL_OR
#undef GL_NOR
#undef GL_XOR
#undef GL_EQUIV
#undef GL_AND_REVERSE
#undef GL_AND_INVERTED
#undef GL_OR_REVERSE
#undef GL_OR_INVERTED

/* Stencil */
#undef GL_STENCIL_BITS
#undef GL_STENCIL_TEST
#undef GL_STENCIL_CLEAR_VALUE
#undef GL_STENCIL_FUNC
#undef GL_STENCIL_VALUE_MASK
#undef GL_STENCIL_FAIL
#undef GL_STENCIL_PASS_DEPTH_FAIL
#undef GL_STENCIL_PASS_DEPTH_PASS
#undef GL_STENCIL_REF
#undef GL_STENCIL_WRITEMASK
#undef GL_STENCIL_INDEX
#undef GL_KEEP
#undef GL_REPLACE
#undef GL_INCR
#undef GL_DECR

/* Buffers, Pixel Drawing/Reading */
#undef GL_NONE
#undef GL_LEFT
#undef GL_RIGHT
/*GL_FRONT					 */
/*GL_BACK					 */
/*GL_FRONT_AND_BACK				 */
#undef GL_FRONT_LEFT
#undef GL_FRONT_RIGHT
#undef GL_BACK_LEFT
#undef GL_BACK_RIGHT
#undef GL_AUX0
#undef GL_AUX1
#undef GL_AUX2
#undef GL_AUX3
#undef GL_COLOR_INDEX
#undef GL_RED
#undef GL_GREEN
#undef GL_BLUE
#undef GL_ALPHA
#undef GL_LUMINANCE
#undef GL_LUMINANCE_ALPHA
#undef GL_ALPHA_BITS
#undef GL_RED_BITS
#undef GL_GREEN_BITS
#undef GL_BLUE_BITS
#undef GL_INDEX_BITS
#undef GL_SUBPIXEL_BITS
#undef GL_AUX_BUFFERS
#undef GL_READ_BUFFER
#undef GL_DRAW_BUFFER
#undef GL_DOUBLEBUFFER
#undef GL_STEREO
#undef GL_BITMAP
#undef GL_COLOR
#undef GL_DEPTH
#undef GL_STENCIL
#undef GL_DITHER
#undef GL_RGB
#undef GL_RGBA

/* Implementation limits */
#undef GL_MAX_LIST_NESTING
#undef GL_MAX_EVAL_ORDER
#undef GL_MAX_LIGHTS
#undef GL_MAX_CLIP_PLANES
#undef GL_MAX_TEXTURE_SIZE
#undef GL_MAX_PIXEL_MAP_TABLE
#undef GL_MAX_ATTRIB_STACK_DEPTH
#undef GL_MAX_MODELVIEW_STACK_DEPTH
#undef GL_MAX_NAME_STACK_DEPTH
#undef GL_MAX_PROJECTION_STACK_DEPTH
#undef GL_MAX_TEXTURE_STACK_DEPTH
#undef GL_MAX_VIEWPORT_DIMS
#undef GL_MAX_CLIENT_ATTRIB_STACK_DEPTH

/* Gets */
#undef GL_ATTRIB_STACK_DEPTH
#undef GL_CLIENT_ATTRIB_STACK_DEPTH
#undef GL_COLOR_CLEAR_VALUE
#undef GL_COLOR_WRITEMASK
#undef GL_CURRENT_INDEX
#undef GL_CURRENT_COLOR
#undef GL_CURRENT_NORMAL
#undef GL_CURRENT_RASTER_COLOR
#undef GL_CURRENT_RASTER_DISTANCE
#undef GL_CURRENT_RASTER_INDEX
#undef GL_CURRENT_RASTER_POSITION
#undef GL_CURRENT_RASTER_TEXTURE_COORDS
#undef GL_CURRENT_RASTER_POSITION_VALID
#undef GL_CURRENT_TEXTURE_COORDS
#undef GL_INDEX_CLEAR_VALUE
#undef GL_INDEX_MODE
#undef GL_INDEX_WRITEMASK
#undef GL_MODELVIEW_MATRIX
#undef GL_MODELVIEW_STACK_DEPTH
#undef GL_NAME_STACK_DEPTH
#undef GL_PROJECTION_MATRIX
#undef GL_PROJECTION_STACK_DEPTH
#undef GL_RENDER_MODE
#undef GL_RGBA_MODE
#undef GL_TEXTURE_MATRIX
#undef GL_TEXTURE_STACK_DEPTH
#undef GL_VIEWPORT

/* Evaluators */
#undef GL_AUTO_NORMAL
#undef GL_MAP1_COLOR_4
#undef GL_MAP1_INDEX
#undef GL_MAP1_NORMAL
#undef GL_MAP1_TEXTURE_COORD_1
#undef GL_MAP1_TEXTURE_COORD_2
#undef GL_MAP1_TEXTURE_COORD_3
#undef GL_MAP1_TEXTURE_COORD_4
#undef GL_MAP1_VERTEX_3
#undef GL_MAP1_VERTEX_4
#undef GL_MAP2_COLOR_4
#undef GL_MAP2_INDEX
#undef GL_MAP2_NORMAL
#undef GL_MAP2_TEXTURE_COORD_1
#undef GL_MAP2_TEXTURE_COORD_2
#undef GL_MAP2_TEXTURE_COORD_3
#undef GL_MAP2_TEXTURE_COORD_4
#undef GL_MAP2_VERTEX_3
#undef GL_MAP2_VERTEX_4
#undef GL_MAP1_GRID_DOMAIN
#undef GL_MAP1_GRID_SEGMENTS
#undef GL_MAP2_GRID_DOMAIN
#undef GL_MAP2_GRID_SEGMENTS
#undef GL_COEFF
#undef GL_ORDER
#undef GL_DOMAIN

/* Hints */
#undef GL_PERSPECTIVE_CORRECTION_HINT
#undef GL_POINT_SMOOTH_HINT
#undef GL_LINE_SMOOTH_HINT
#undef GL_POLYGON_SMOOTH_HINT
#undef GL_FOG_HINT
#undef GL_DONT_CARE
#undef GL_FASTEST
#undef GL_NICEST

/* Scissor box */
#undef GL_SCISSOR_BOX
#undef GL_SCISSOR_TEST

/* Pixel Mode / Transfer */
#undef GL_MAP_COLOR
#undef GL_MAP_STENCIL
#undef GL_INDEX_SHIFT
#undef GL_INDEX_OFFSET
#undef GL_RED_SCALE
#undef GL_RED_BIAS
#undef GL_GREEN_SCALE
#undef GL_GREEN_BIAS
#undef GL_BLUE_SCALE
#undef GL_BLUE_BIAS
#undef GL_ALPHA_SCALE
#undef GL_ALPHA_BIAS
#undef GL_DEPTH_SCALE
#undef GL_DEPTH_BIAS
#undef GL_PIXEL_MAP_S_TO_S_SIZE
#undef GL_PIXEL_MAP_I_TO_I_SIZE
#undef GL_PIXEL_MAP_I_TO_R_SIZE
#undef GL_PIXEL_MAP_I_TO_G_SIZE
#undef GL_PIXEL_MAP_I_TO_B_SIZE
#undef GL_PIXEL_MAP_I_TO_A_SIZE
#undef GL_PIXEL_MAP_R_TO_R_SIZE
#undef GL_PIXEL_MAP_G_TO_G_SIZE
#undef GL_PIXEL_MAP_B_TO_B_SIZE
#undef GL_PIXEL_MAP_A_TO_A_SIZE
#undef GL_PIXEL_MAP_S_TO_S
#undef GL_PIXEL_MAP_I_TO_I
#undef GL_PIXEL_MAP_I_TO_R
#undef GL_PIXEL_MAP_I_TO_G
#undef GL_PIXEL_MAP_I_TO_B
#undef GL_PIXEL_MAP_I_TO_A
#undef GL_PIXEL_MAP_R_TO_R
#undef GL_PIXEL_MAP_G_TO_G
#undef GL_PIXEL_MAP_B_TO_B
#undef GL_PIXEL_MAP_A_TO_A
#undef GL_PACK_ALIGNMENT
#undef GL_PACK_LSB_FIRST
#undef GL_PACK_ROW_LENGTH
#undef GL_PACK_SKIP_PIXELS
#undef GL_PACK_SKIP_ROWS
#undef GL_PACK_SWAP_BYTES
#undef GL_UNPACK_ALIGNMENT
#undef GL_UNPACK_LSB_FIRST
#undef GL_UNPACK_ROW_LENGTH
#undef GL_UNPACK_SKIP_PIXELS
#undef GL_UNPACK_SKIP_ROWS
#undef GL_UNPACK_SWAP_BYTES
#undef GL_ZOOM_X
#undef GL_ZOOM_Y

/* Texture mapping */
#undef GL_TEXTURE_ENV
#undef GL_TEXTURE_ENV_MODE
#undef GL_TEXTURE_1D
#undef GL_TEXTURE_2D
#undef GL_TEXTURE_WRAP_S
#undef GL_TEXTURE_WRAP_T
#undef GL_TEXTURE_MAG_FILTER
#undef GL_TEXTURE_MIN_FILTER
#undef GL_TEXTURE_ENV_COLOR
#undef GL_TEXTURE_GEN_S
#undef GL_TEXTURE_GEN_T
#undef GL_TEXTURE_GEN_R
#undef GL_TEXTURE_GEN_Q
#undef GL_TEXTURE_GEN_MODE
#undef GL_TEXTURE_BORDER_COLOR
#undef GL_TEXTURE_WIDTH
#undef GL_TEXTURE_HEIGHT
#undef GL_TEXTURE_BORDER
#undef GL_TEXTURE_COMPONENTS
#undef GL_TEXTURE_RED_SIZE
#undef GL_TEXTURE_GREEN_SIZE
#undef GL_TEXTURE_BLUE_SIZE
#undef GL_TEXTURE_ALPHA_SIZE
#undef GL_TEXTURE_LUMINANCE_SIZE
#undef GL_TEXTURE_INTENSITY_SIZE
#undef GL_NEAREST_MIPMAP_NEAREST
#undef GL_NEAREST_MIPMAP_LINEAR
#undef GL_LINEAR_MIPMAP_NEAREST
#undef GL_LINEAR_MIPMAP_LINEAR
#undef GL_OBJECT_LINEAR
#undef GL_OBJECT_PLANE
#undef GL_EYE_LINEAR
#undef GL_EYE_PLANE
#undef GL_SPHERE_MAP
#undef GL_DECAL
#undef GL_MODULATE
#undef GL_NEAREST
#undef GL_REPEAT
#undef GL_CLAMP
#undef GL_S
#undef GL_T
#undef GL_R
#undef GL_Q

/* Utility */
#undef GL_VENDOR
#undef GL_RENDERER
#undef GL_VERSION
#undef GL_EXTENSIONS

/* Errors */
#undef GL_NO_ERROR
#undef GL_INVALID_ENUM
#undef GL_INVALID_VALUE
#undef GL_INVALID_OPERATION
#undef GL_STACK_OVERFLOW
#undef GL_STACK_UNDERFLOW
#undef GL_OUT_OF_MEMORY

/* glPush/PopAttrib bits */
#undef GL_CURRENT_BIT
#undef GL_POINT_BIT
#undef GL_LINE_BIT
#undef GL_POLYGON_BIT
#undef GL_POLYGON_STIPPLE_BIT
#undef GL_PIXEL_MODE_BIT
#undef GL_LIGHTING_BIT
#undef GL_FOG_BIT
#undef GL_DEPTH_BUFFER_BIT
#undef GL_ACCUM_BUFFER_BIT
#undef GL_STENCIL_BUFFER_BIT
#undef GL_VIEWPORT_BIT
#undef GL_TRANSFORM_BIT
#undef GL_ENABLE_BIT
#undef GL_COLOR_BUFFER_BIT
#undef GL_HINT_BIT
#undef GL_EVAL_BIT
#undef GL_LIST_BIT
#undef GL_TEXTURE_BIT
#undef GL_SCISSOR_BIT
#undef GL_ALL_ATTRIB_BITS


/* OpenGL 1.1 */
#undef GL_PROXY_TEXTURE_1D
#undef GL_PROXY_TEXTURE_2D
#undef GL_TEXTURE_PRIORITY
#undef GL_TEXTURE_RESIDENT
#undef GL_TEXTURE_BINDING_1D
#undef GL_TEXTURE_BINDING_2D
#undef GL_TEXTURE_INTERNAL_FORMAT
#undef GL_ALPHA4
#undef GL_ALPHA8
#undef GL_ALPHA12
#undef GL_ALPHA16
#undef GL_LUMINANCE4
#undef GL_LUMINANCE8
#undef GL_LUMINANCE12
#undef GL_LUMINANCE16
#undef GL_LUMINANCE4_ALPHA4
#undef GL_LUMINANCE6_ALPHA2
#undef GL_LUMINANCE8_ALPHA8
#undef GL_LUMINANCE12_ALPHA4
#undef GL_LUMINANCE12_ALPHA12
#undef GL_LUMINANCE16_ALPHA16
#undef GL_INTENSITY
#undef GL_INTENSITY4
#undef GL_INTENSITY8
#undef GL_INTENSITY12
#undef GL_INTENSITY16
#undef GL_R3_G3_B2
#undef GL_RGB4
#undef GL_RGB5
#undef GL_RGB8
#undef GL_RGB10
#undef GL_RGB12
#undef GL_RGB16
#undef GL_RGBA2
#undef GL_RGBA4
#undef GL_RGB5_A1
#undef GL_RGBA8
#undef GL_RGB10_A2
#undef GL_RGBA12
#undef GL_RGBA16
#undef GL_CLIENT_PIXEL_STORE_BIT
#undef GL_CLIENT_VERTEX_ARRAY_BIT
#undef GL_ALL_CLIENT_ATTRIB_BITS
#undef GL_CLIENT_ALL_ATTRIB_BITS



/*
 * OpenGL 1.3
 */

/* multitexture */
#undef GL_TEXTURE0
#undef GL_TEXTURE1
#undef GL_TEXTURE2
#undef GL_TEXTURE3
#undef GL_TEXTURE4
#undef GL_TEXTURE5
#undef GL_TEXTURE6
#undef GL_TEXTURE7
#undef GL_TEXTURE8
#undef GL_TEXTURE9
#undef GL_TEXTURE10
#undef GL_TEXTURE11
#undef GL_TEXTURE12
#undef GL_TEXTURE13
#undef GL_TEXTURE14
#undef GL_TEXTURE15
#undef GL_TEXTURE16
#undef GL_TEXTURE17
#undef GL_TEXTURE18
#undef GL_TEXTURE19
#undef GL_TEXTURE20
#undef GL_TEXTURE21
#undef GL_TEXTURE22
#undef GL_TEXTURE23
#undef GL_TEXTURE24
#undef GL_TEXTURE25
#undef GL_TEXTURE26
#undef GL_TEXTURE27
#undef GL_TEXTURE28
#undef GL_TEXTURE29
#undef GL_TEXTURE30
#undef GL_TEXTURE31
#undef GL_ACTIVE_TEXTURE
#undef GL_CLIENT_ACTIVE_TEXTURE
#undef GL_MAX_TEXTURE_UNITS
/* texture_cube_map */
#undef GL_NORMAL_MAP
#undef GL_REFLECTION_MAP
#undef GL_TEXTURE_CUBE_MAP
#undef GL_TEXTURE_BINDING_CUBE_MAP
#undef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#undef GL_TEXTURE_CUBE_MAP_NEGATIVE_X
#undef GL_TEXTURE_CUBE_MAP_POSITIVE_Y
#undef GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#undef GL_TEXTURE_CUBE_MAP_POSITIVE_Z
#undef GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
#undef GL_PROXY_TEXTURE_CUBE_MAP
#undef GL_MAX_CUBE_MAP_TEXTURE_SIZE
/* texture_compression */
#undef GL_COMPRESSED_ALPHA
#undef GL_COMPRESSED_LUMINANCE
#undef GL_COMPRESSED_LUMINANCE_ALPHA
#undef GL_COMPRESSED_INTENSITY
#undef GL_COMPRESSED_RGB
#undef GL_COMPRESSED_RGBA
#undef GL_TEXTURE_COMPRESSION_HINT
#undef GL_TEXTURE_COMPRESSED_IMAGE_SIZE
#undef GL_TEXTURE_COMPRESSED
#undef GL_NUM_COMPRESSED_TEXTURE_FORMATS
#undef GL_COMPRESSED_TEXTURE_FORMATS
/* multisample */
#undef GL_MULTISAMPLE
#undef GL_SAMPLE_ALPHA_TO_COVERAGE
#undef GL_SAMPLE_ALPHA_TO_ONE
#undef GL_SAMPLE_COVERAGE
#undef GL_SAMPLE_BUFFERS
#undef GL_SAMPLES
#undef GL_SAMPLE_COVERAGE_VALUE
#undef GL_SAMPLE_COVERAGE_INVERT
#undef GL_MULTISAMPLE_BIT
/* transpose_matrix */
#undef GL_TRANSPOSE_MODELVIEW_MATRIX
#undef GL_TRANSPOSE_PROJECTION_MATRIX
#undef GL_TRANSPOSE_TEXTURE_MATRIX
#undef GL_TRANSPOSE_COLOR_MATRIX
/* texture_env_combine */
#undef GL_COMBINE
#undef GL_COMBINE_RGB
#undef GL_COMBINE_ALPHA
#undef GL_SOURCE0_RGB
#undef GL_SOURCE1_RGB
#undef GL_SOURCE2_RGB
#undef GL_SOURCE0_ALPHA
#undef GL_SOURCE1_ALPHA
#undef GL_SOURCE2_ALPHA
#undef GL_OPERAND0_RGB
#undef GL_OPERAND1_RGB
#undef GL_OPERAND2_RGB
#undef GL_OPERAND0_ALPHA
#undef GL_OPERAND1_ALPHA
#undef GL_OPERAND2_ALPHA
#undef GL_RGB_SCALE
#undef GL_ADD_SIGNED
#undef GL_INTERPOLATE
#undef GL_SUBTRACT
#undef GL_CONSTANT
#undef GL_PRIMARY_COLOR
#undef GL_PREVIOUS
/* texture_env_dot3 */
#undef GL_DOT3_RGB
#undef GL_DOT3_RGBA
/* texture_border_clamp */
#undef GL_CLAMP_TO_BORDER




/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */
#undef GL_TEXTURE0_ARB
#undef GL_TEXTURE1_ARB
#undef GL_TEXTURE2_ARB
#undef GL_TEXTURE3_ARB
#undef GL_TEXTURE4_ARB
#undef GL_TEXTURE5_ARB
#undef GL_TEXTURE6_ARB
#undef GL_TEXTURE7_ARB
#undef GL_TEXTURE8_ARB
#undef GL_TEXTURE9_ARB
#undef GL_TEXTURE10_ARB
#undef GL_TEXTURE11_ARB
#undef GL_TEXTURE12_ARB
#undef GL_TEXTURE13_ARB
#undef GL_TEXTURE14_ARB
#undef GL_TEXTURE15_ARB
#undef GL_TEXTURE16_ARB
#undef GL_TEXTURE17_ARB
#undef GL_TEXTURE18_ARB
#undef GL_TEXTURE19_ARB
#undef GL_TEXTURE20_ARB
#undef GL_TEXTURE21_ARB
#undef GL_TEXTURE22_ARB
#undef GL_TEXTURE23_ARB
#undef GL_TEXTURE24_ARB
#undef GL_TEXTURE25_ARB
#undef GL_TEXTURE26_ARB
#undef GL_TEXTURE27_ARB
#undef GL_TEXTURE28_ARB
#undef GL_TEXTURE29_ARB
#undef GL_TEXTURE30_ARB
#undef GL_TEXTURE31_ARB
#undef GL_ACTIVE_TEXTURE_ARB
#undef GL_CLIENT_ACTIVE_TEXTURE_ARB
#undef GL_MAX_TEXTURE_UNITS_ARB



#undef GL_DEPTH_STENCIL_MESA
#undef GL_UNSIGNED_INT_24_8_MESA
#undef GL_UNSIGNED_INT_8_24_REV_MESA
#undef GL_UNSIGNED_SHORT_15_1_MESA
#undef GL_UNSIGNED_SHORT_1_15_REV_MESA



/* GL_VERSION_1_5 */
#undef GL_BUFFER_SIZE
#undef GL_BUFFER_USAGE
#undef GL_QUERY_COUNTER_BITS
#undef GL_CURRENT_QUERY
#undef GL_QUERY_RESULT
#undef GL_QUERY_RESULT_AVAILABLE
#undef GL_ARRAY_BUFFER
#undef GL_ELEMENT_ARRAY_BUFFER
#undef GL_ARRAY_BUFFER_BINDING
#undef GL_ELEMENT_ARRAY_BUFFER_BINDING
#undef GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING
#undef GL_READ_ONLY
#undef GL_WRITE_ONLY
#undef GL_READ_WRITE
#undef GL_BUFFER_ACCESS
#undef GL_BUFFER_MAPPED
#undef GL_BUFFER_MAP_POINTER
#undef GL_STREAM_DRAW
#undef GL_STREAM_READ
#undef GL_STREAM_COPY
#undef GL_STATIC_DRAW
#undef GL_STATIC_READ
#undef GL_STATIC_COPY
#undef GL_DYNAMIC_DRAW
#undef GL_DYNAMIC_READ
#undef GL_DYNAMIC_COPY
#undef GL_SAMPLES_PASSED
#undef GL_SRC1_ALPHA
#undef GL_VERTEX_ARRAY_BUFFER_BINDING
#undef GL_NORMAL_ARRAY_BUFFER_BINDING
#undef GL_COLOR_ARRAY_BUFFER_BINDING
#undef GL_INDEX_ARRAY_BUFFER_BINDING
#undef GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING
#undef GL_EDGE_FLAG_ARRAY_BUFFER_BINDING
#undef GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING
#undef GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#undef GL_WEIGHT_ARRAY_BUFFER_BINDING
#undef GL_FOG_COORD_SRC
#undef GL_FOG_COORD
#undef GL_CURRENT_FOG_COORD
#undef GL_FOG_COORD_ARRAY_TYPE
#undef GL_FOG_COORD_ARRAY_STRIDE
#undef GL_FOG_COORD_ARRAY_POINTER
#undef GL_FOG_COORD_ARRAY
#undef GL_FOG_COORD_ARRAY_BUFFER_BINDING
#undef GL_SRC0_RGB
#undef GL_SRC1_RGB
#undef GL_SRC2_RGB
#undef GL_SRC0_ALPHA
#undef GL_SRC2_ALPHA


/* GL_VERSION_2_0 */
#undef GL_BLEND_EQUATION_RGB
#undef GL_VERTEX_ATTRIB_ARRAY_ENABLED
#undef GL_VERTEX_ATTRIB_ARRAY_SIZE
#undef GL_VERTEX_ATTRIB_ARRAY_STRIDE
#undef GL_VERTEX_ATTRIB_ARRAY_TYPE
#undef GL_CURRENT_VERTEX_ATTRIB
#undef GL_VERTEX_PROGRAM_POINT_SIZE
#undef GL_VERTEX_ATTRIB_ARRAY_POINTER
#undef GL_STENCIL_BACK_FUNC
#undef GL_STENCIL_BACK_FAIL
#undef GL_STENCIL_BACK_PASS_DEPTH_FAIL
#undef GL_STENCIL_BACK_PASS_DEPTH_PASS
#undef GL_MAX_DRAW_BUFFERS
#undef GL_DRAW_BUFFER0
#undef GL_DRAW_BUFFER1
#undef GL_DRAW_BUFFER2
#undef GL_DRAW_BUFFER3
#undef GL_DRAW_BUFFER4
#undef GL_DRAW_BUFFER5
#undef GL_DRAW_BUFFER6
#undef GL_DRAW_BUFFER7
#undef GL_DRAW_BUFFER8
#undef GL_DRAW_BUFFER9
#undef GL_DRAW_BUFFER10
#undef GL_DRAW_BUFFER11
#undef GL_DRAW_BUFFER12
#undef GL_DRAW_BUFFER13
#undef GL_DRAW_BUFFER14
#undef GL_DRAW_BUFFER15
#undef GL_BLEND_EQUATION_ALPHA
#undef GL_MAX_VERTEX_ATTRIBS
#undef GL_VERTEX_ATTRIB_ARRAY_NORMALIZED
#undef GL_MAX_TEXTURE_IMAGE_UNITS
#undef GL_FRAGMENT_SHADER
#undef GL_VERTEX_SHADER
#undef GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
#undef GL_MAX_VERTEX_UNIFORM_COMPONENTS
#undef GL_MAX_VARYING_FLOATS
#undef GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
#undef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
#undef GL_SHADER_TYPE
#undef GL_FLOAT_VEC2
#undef GL_FLOAT_VEC3
#undef GL_FLOAT_VEC4
#undef GL_INT_VEC2
#undef GL_INT_VEC3
#undef GL_INT_VEC4
#undef GL_BOOL
#undef GL_BOOL_VEC2
#undef GL_BOOL_VEC3
#undef GL_BOOL_VEC4
#undef GL_FLOAT_MAT2
#undef GL_FLOAT_MAT3
#undef GL_FLOAT_MAT4
#undef GL_SAMPLER_1D
#undef GL_SAMPLER_2D
#undef GL_SAMPLER_3D
#undef GL_SAMPLER_CUBE
#undef GL_SAMPLER_1D_SHADOW
#undef GL_SAMPLER_2D_SHADOW
#undef GL_DELETE_STATUS
#undef GL_COMPILE_STATUS
#undef GL_LINK_STATUS
#undef GL_VALIDATE_STATUS
#undef GL_INFO_LOG_LENGTH
#undef GL_ATTACHED_SHADERS
#undef GL_ACTIVE_UNIFORMS
#undef GL_ACTIVE_UNIFORM_MAX_LENGTH
#undef GL_SHADER_SOURCE_LENGTH
#undef GL_ACTIVE_ATTRIBUTES
#undef GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
#undef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
#undef GL_SHADING_LANGUAGE_VERSION
#undef GL_CURRENT_PROGRAM
#undef GL_POINT_SPRITE_COORD_ORIGIN
#undef GL_LOWER_LEFT
#undef GL_UPPER_LEFT
#undef GL_STENCIL_BACK_REF
#undef GL_STENCIL_BACK_VALUE_MASK
#undef GL_STENCIL_BACK_WRITEMASK
#undef GL_VERTEX_PROGRAM_TWO_SIDE
#undef GL_POINT_SPRITE
#undef GL_COORD_REPLACE
#undef GL_MAX_TEXTURE_COORDS

/* GL_VERSION_2_1 */
#undef GL_PIXEL_PACK_BUFFER
#undef GL_PIXEL_UNPACK_BUFFER
#undef GL_PIXEL_PACK_BUFFER_BINDING
#undef GL_PIXEL_UNPACK_BUFFER_BINDING
#undef GL_FLOAT_MAT2x3
#undef GL_FLOAT_MAT2x4
#undef GL_FLOAT_MAT3x2
#undef GL_FLOAT_MAT3x4
#undef GL_FLOAT_MAT4x2
#undef GL_FLOAT_MAT4x3
#undef GL_SRGB
#undef GL_SRGB8
#undef GL_SRGB_ALPHA
#undef GL_SRGB8_ALPHA8
#undef GL_COMPRESSED_SRGB
#undef GL_COMPRESSED_SRGB_ALPHA
#undef GL_CURRENT_RASTER_SECONDARY_COLOR
#undef GL_SLUMINANCE_ALPHA
#undef GL_SLUMINANCE8_ALPHA8
#undef GL_SLUMINANCE
#undef GL_SLUMINANCE8
#undef GL_COMPRESSED_SLUMINANCE
#undef GL_COMPRESSED_SLUMINANCE_ALPHA

/* GL_VERSION_3_0 */
#undef GL_COMPARE_REF_TO_TEXTURE
#undef GL_CLIP_DISTANCE0
#undef GL_CLIP_DISTANCE1
#undef GL_CLIP_DISTANCE2
#undef GL_CLIP_DISTANCE3
#undef GL_CLIP_DISTANCE4
#undef GL_CLIP_DISTANCE5
#undef GL_CLIP_DISTANCE6
#undef GL_CLIP_DISTANCE7
#undef GL_MAX_CLIP_DISTANCES
#undef GL_MAJOR_VERSION
#undef GL_MINOR_VERSION
#undef GL_NUM_EXTENSIONS
#undef GL_CONTEXT_FLAGS
#undef GL_COMPRESSED_RED
#undef GL_COMPRESSED_RG
#undef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT
#undef GL_RGBA32F
#undef GL_RGB32F
#undef GL_RGBA16F
#undef GL_RGB16F
#undef GL_VERTEX_ATTRIB_ARRAY_INTEGER
#undef GL_MAX_ARRAY_TEXTURE_LAYERS
#undef GL_MIN_PROGRAM_TEXEL_OFFSET
#undef GL_MAX_PROGRAM_TEXEL_OFFSET
#undef GL_CLAMP_READ_COLOR
#undef GL_FIXED_ONLY
#undef GL_MAX_VARYING_COMPONENTS
#undef GL_TEXTURE_1D_ARRAY
#undef GL_PROXY_TEXTURE_1D_ARRAY
#undef GL_TEXTURE_2D_ARRAY
#undef GL_PROXY_TEXTURE_2D_ARRAY
#undef GL_TEXTURE_BINDING_1D_ARRAY
#undef GL_TEXTURE_BINDING_2D_ARRAY
#undef GL_R11F_G11F_B10F
#undef GL_UNSIGNED_INT_10F_11F_11F_REV
#undef GL_RGB9_E5
#undef GL_UNSIGNED_INT_5_9_9_9_REV
#undef GL_TEXTURE_SHARED_SIZE
#undef GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH
#undef GL_TRANSFORM_FEEDBACK_BUFFER_MODE
#undef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
#undef GL_TRANSFORM_FEEDBACK_VARYINGS
#undef GL_TRANSFORM_FEEDBACK_BUFFER_START
#undef GL_TRANSFORM_FEEDBACK_BUFFER_SIZE
#undef GL_PRIMITIVES_GENERATED
#undef GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
#undef GL_RASTERIZER_DISCARD
#undef GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
#undef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS
#undef GL_INTERLEAVED_ATTRIBS
#undef GL_SEPARATE_ATTRIBS
#undef GL_TRANSFORM_FEEDBACK_BUFFER
#undef GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
#undef GL_RGBA32UI
#undef GL_RGB32UI
#undef GL_RGBA16UI
#undef GL_RGB16UI
#undef GL_RGBA8UI
#undef GL_RGB8UI
#undef GL_RGBA32I
#undef GL_RGB32I
#undef GL_RGBA16I
#undef GL_RGB16I
#undef GL_RGBA8I
#undef GL_RGB8I
#undef GL_RED_INTEGER
#undef GL_GREEN_INTEGER
#undef GL_BLUE_INTEGER
#undef GL_RGB_INTEGER
#undef GL_RGBA_INTEGER
#undef GL_BGR_INTEGER
#undef GL_BGRA_INTEGER
#undef GL_SAMPLER_1D_ARRAY
#undef GL_SAMPLER_2D_ARRAY
#undef GL_SAMPLER_1D_ARRAY_SHADOW
#undef GL_SAMPLER_2D_ARRAY_SHADOW
#undef GL_SAMPLER_CUBE_SHADOW
#undef GL_UNSIGNED_INT_VEC2
#undef GL_UNSIGNED_INT_VEC3
#undef GL_UNSIGNED_INT_VEC4
#undef GL_INT_SAMPLER_1D
#undef GL_INT_SAMPLER_2D
#undef GL_INT_SAMPLER_3D
#undef GL_INT_SAMPLER_CUBE
#undef GL_INT_SAMPLER_1D_ARRAY
#undef GL_INT_SAMPLER_2D_ARRAY
#undef GL_UNSIGNED_INT_SAMPLER_1D
#undef GL_UNSIGNED_INT_SAMPLER_2D
#undef GL_UNSIGNED_INT_SAMPLER_3D
#undef GL_UNSIGNED_INT_SAMPLER_CUBE
#undef GL_UNSIGNED_INT_SAMPLER_1D_ARRAY
#undef GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
#undef GL_QUERY_WAIT
#undef GL_QUERY_NO_WAIT
#undef GL_QUERY_BY_REGION_WAIT
#undef GL_QUERY_BY_REGION_NO_WAIT
#undef GL_BUFFER_ACCESS_FLAGS
#undef GL_BUFFER_MAP_LENGTH
#undef GL_BUFFER_MAP_OFFSET

