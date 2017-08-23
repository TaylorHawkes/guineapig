void render();
void multiplyMatrices(float *verticies, float translation[],int verticies_size,int translation_size);
void getRotateYMatrix(float angle,float *r_matrix);
void getRotateXMatrix(float angle,float *r_matrix);
void getViewMatrix(float *r_matrix);
void getModelMatrix( float *r_matrix);
void get_indices(short *indicies ,int indicies_size);

void getProjectionMatrix( float *r_matrix,float angleOfView,float imageAspectRatio, float zMin, float zMax);

static GLuint make_buffer( GLenum target, const void *buffer_data, GLsizei buffer_size);

static int make_resources(void);
static GLuint make_shader(GLenum type, const char *filename);
static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

typedef struct {
    float x,y,z;
} Vec;

float v_distance(Vec v1,Vec v2);
Vec v_mult_s(Vec v,float m);
Vec v_divide_s(Vec v,float m);
Vec v_add(Vec a, Vec b);
