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

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Quat;

float v_distance(Vec v1,Vec v2);
Vec v_mult_s(Vec v,float m);
Vec v_divide_s(Vec v,float m);
Vec v_add(Vec a, Vec b);

float v_dot( Vec v1, Vec v2);
Vec v_mult( Vec a, Vec b);
Vec v_mult_compenent_product(Vec a, Vec b );
Vec v_norm( Vec v);
void quat_to_matrix(Quat q,float *r_matrix);
Quat quat_mul(Quat q1,Quat q2);
void quat_norm(Quat *q);
Quat new_quat(float angle,Vec axis);
Vec v_sub( Vec a, Vec b);
Quat updateQuatByRotation(Quat update_quat, Vec v,float seconds);

float v_mag( Vec v);
void mult_matrix(float *a,float *b);
void get_position_matrix(float * m,Vec p);
float quat_dot(Quat a, Quat b);

typedef struct {
    Vec point;//point on object force is applied
    Vec force;//direction and magnitude of force neutons
    float start;
    float stop;
} Force;


typedef struct {
  Vec center_of_mass;
  Vec ineritia;
  Vec velocity;
  Vec angular_velocity;
 float mass;

 Force forces[10];
 int forces_count;

 float *verticies;
 float *normals;
 float *indicies;

 int verticies_size; 
 int indicies_size;
 int normals_size;

 GLfloat model_matrix[16];

 float inertia_tensor[16];

 Quat orientation;
 Vec position;//toto:maybe should be center_of_maxx

} Solid ;

typedef struct {
  Solid s;
} Cube;

void new_cube(Cube *cube);

typedef struct{
    Cube cube;
} Quadcopter;

typedef struct{
   int first_run;
   float start;
   float stop;
   char * action;
   int a_value;
} Control;

float min_0(float t);


Quadcopter_hover(Quadcopter * q);
Quadcopter new_quadcopter();

//void Quadcopter_go_to_height(float height);
//void Quadcopter_apply_to_all_thrust(float thrust);
