#include <stdlib.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 640;

const unsigned int PIXELS_WIDTH = 128;
const unsigned int PIXELS_HEIGHT = 64;

const int PADDLE_WIDTH = 1;
const int PADDLE_HEIGHT = 8;

typedef enum {
    SIZE,
    POSITION,
    INPUT,
    RENDERER
} component_uid_t;

typedef struct
{
    int x, y;
    int w, h;
    int input_x, input_y;
    bool visible;
} entity_resource_t;

typedef struct
{
    int w, h;
} extension_t;

typedef struct
{
    int x, y;
} point_t;

typedef struct
{
    int x, y;
} input_t;

typedef struct
{
    bool visible;
} renderer_t;


typedef struct
{
    unsigned int components[3];
    extension_t sizes[3];
    point_t positions[3];
    input_t inputs[3];
    renderer_t renderers[3];
    int length;
} entity_manager_t;

typedef struct
{
    int left_paddle_up;
    int left_paddle_down;
    int right_paddle_up;
    int right_paddle_down;
} key_mapping_t;

key_mapping_t key_mapping;

int create_entity(entity_manager_t* entity_manager, unsigned int components);
void setup_component(entity_manager_t* entity_manager, int entity, entity_resource_t resource);
void movement_system(entity_manager_t* entity_manager);
void renderer_system(entity_manager_t* entity_manager, GLubyte* pixels_buffer);
void update_ball(entity_manager_t* entity_manager, int ball, int paddles[2]);
void update_paddle(entity_manager_t* entity_manager, int paddle);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(int argc, char **argv)
{
    GLFWwindow* window;

    if(glfwInit() == false)
    {
        return -1;
    }

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pong", NULL, NULL);
    if(window == false)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    GLubyte* pixels_buffer = new GLubyte[PIXELS_WIDTH * PIXELS_HEIGHT * 3];

    entity_manager_t entity_manager = {};
    int point = rand() % 2 == 0 ? 1 : -1;
    float seconds = 0;

    int left_paddle = create_entity(&entity_manager, SIZE | POSITION | INPUT);
    entity_resource_t left_paddle_rsc = {2, 2, PADDLE_WIDTH, PADDLE_HEIGHT, 0, 0, true};
    setup_component(&entity_manager, left_paddle, left_paddle_rsc);

    int right_paddle = create_entity(&entity_manager, SIZE | POSITION | INPUT);
    entity_resource_t right_paddle_rsc = {PIXELS_WIDTH - 3, 15, PADDLE_WIDTH, PADDLE_HEIGHT, 0, 0, true};
    setup_component(&entity_manager, right_paddle, right_paddle_rsc);

    int ball = create_entity(&entity_manager, SIZE | POSITION | INPUT);
    entity_resource_t ball_rsc = {0, 0, 1, 1, 0, 0, false};
    setup_component(&entity_manager, ball, ball_rsc);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    while(glfwWindowShouldClose(window) == false)
    {
        // time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // game
        {
            for (int x = 0; x < PIXELS_WIDTH; x++)
            {
                for (int y = 0; y < PIXELS_HEIGHT; y++)
                {
                    int position = (x + y * PIXELS_WIDTH) * 3;
                    pixels_buffer[position] = 0;
                    pixels_buffer[position + 1] = 0;
                    pixels_buffer[position + 2] = 0;
                }
            }

            entity_manager.inputs[left_paddle].y = 0;
            if(key_mapping.left_paddle_up)
            {
                entity_manager.inputs[left_paddle].y += 1;
            }
            if(key_mapping.left_paddle_down)
            {
                entity_manager.inputs[left_paddle].y -= 1;
            }

            entity_manager.inputs[right_paddle].y = 0;
            if(key_mapping.right_paddle_up)
            {
                entity_manager.inputs[right_paddle].y += 1;
            }
            if(key_mapping.right_paddle_down)
            {
                entity_manager.inputs[right_paddle].y -= 1;
            }

            movement_system(&entity_manager);

            int entities[2] = {left_paddle, right_paddle};
            update_ball(&entity_manager, ball, entities);

            point_t ball_point = entity_manager.positions[ball];
            extension_t ball_extension = entity_manager.sizes[ball];
            if(ball_point.x < 0 || (ball_point.x + ball_extension.w - 1) > PIXELS_WIDTH - 1)
            {
                point = ball_point.x <= 0 ? -1 : 1;
                entity_manager.renderers[ball].visible = false;
            }

            update_paddle(&entity_manager, left_paddle);
            update_paddle(&entity_manager, right_paddle);

            if(point != 0)
            {
                seconds += deltaTime;
                if(seconds >= 2.0)
                {
                    entity_manager.positions[ball].x = (PIXELS_WIDTH / 2) + point * -2;
                    entity_manager.positions[ball].y = (PIXELS_HEIGHT / 2);
                    entity_manager.inputs[ball].x = point;
                    entity_manager.inputs[ball].y = rand() % 2 == 0 ? 1 : -1;
                    entity_manager.renderers[ball].visible = true;

                    seconds = 0.0;
                    point = 0;
                }
            }

            renderer_system(&entity_manager, pixels_buffer);
        }

        // render
        glClear(GL_COLOR_BUFFER_BIT);
        glPixelZoom(SCR_WIDTH / PIXELS_WIDTH, SCR_HEIGHT / PIXELS_HEIGHT);
        glDrawPixels(PIXELS_WIDTH, PIXELS_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels_buffer);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

int create_entity(entity_manager_t* entity_manager, unsigned int components)
{
    int entity = entity_manager->length++;
    entity_manager->components[entity] = components;
    return entity;
}

void setup_component(entity_manager_t* entity_manager, int entity, entity_resource_t resource)
{
    unsigned int components_mask = entity_manager->components[entity];

    if((components_mask & SIZE) == SIZE)
    {
        entity_manager->sizes[entity].w = resource.w;
        entity_manager->sizes[entity].h = resource.h;
    }

    if((components_mask & POSITION) == POSITION)
    {
        entity_manager->positions[entity].x = resource.x;
        entity_manager->positions[entity].y = resource.y;
    }

    if((components_mask & INPUT) == INPUT)
    {
        entity_manager->inputs[entity].x = resource.input_x;
        entity_manager->inputs[entity].y = resource.input_y;
    }

    if((components_mask & RENDERER) == RENDERER)
    {
        entity_manager->renderers[entity].visible = resource.visible;
    }
}

void movement_system(entity_manager_t* entity_manager)
{
    const unsigned int REQUIRED_COMPONENTS = SIZE | POSITION | INPUT;
    for (int entity = 0; entity < entity_manager->length; entity++)
    {
        unsigned int components_mask = entity_manager->components[entity];

        if((components_mask & REQUIRED_COMPONENTS) != REQUIRED_COMPONENTS) continue;

        extension_t size = entity_manager->sizes[entity];
        point_t position = entity_manager->positions[entity];
        input_t input = entity_manager->inputs[entity];

        position.x += input.x;
        position.y += input.y;

        entity_manager->positions[entity] = position;
    }
}

void renderer_system(entity_manager_t* entity_manager, GLubyte* pixels_buffer)
{
    const unsigned int REQUIRED_COMPONENTS = SIZE | POSITION | RENDERER;
    for (int entity = 0; entity < entity_manager->length; entity++)
    {
        unsigned int components_mask = entity_manager->components[entity];

        if((components_mask & REQUIRED_COMPONENTS) != REQUIRED_COMPONENTS) continue;

        renderer_t renderer = entity_manager->renderers[entity];
        if(renderer.visible == false) continue;

        extension_t size = entity_manager->sizes[entity];
        point_t position = entity_manager->positions[entity];
        for (int w = 0; w < size.w; w++)
        {
            for (int h = 0; h < size.h; h++)
            {
                int i = ((position.x + w) + (position.y + h) * PIXELS_WIDTH) * 3;
                pixels_buffer[i] = 255;
                pixels_buffer[i + 1] = 255;
                pixels_buffer[i + 2] = 255;
            }
        }
    }
}

void update_ball(entity_manager_t* entity_manager, int ball, int paddles[2])
{
    if(entity_manager->renderers[ball].visible == false) return;

    point_t ball_point = entity_manager->positions[ball];
    extension_t ball_extension = entity_manager->sizes[ball];
    input_t ball_input = entity_manager->inputs[ball];

    if(ball_point.y <= 0 || (ball_point.y + ball_extension.h - 1) >= PIXELS_HEIGHT - 1)
    {
        ball_point.y = ball_point.y <= 0 ? 0 : PIXELS_HEIGHT - ball_extension.h;
        ball_input.y *= -1;
    }

    for (int i = 0; i < 2; i++)
    {
        point_t paddle_point = entity_manager->positions[paddles[i]];
        extension_t paddle_extension = entity_manager->sizes[paddles[i]];

        if(ball_point.x >= paddle_point.x && ball_point.x <= paddle_point.x + paddle_extension.w - 1)
        {
            if(ball_point.y >= paddle_point.y && ball_point.y <= paddle_point.y + paddle_extension.h - 1)
            {
                if(ball_input.x == -1)
                {
                    ball_point.x = paddle_point.x + paddle_extension.w;
                } 
                else if(ball_input.x == 1)
                {
                    ball_point.x = paddle_point.x - ball_extension.w;
                }

                ball_input.x *= -1;
            }
        }
    } 

    entity_manager->positions[ball] = ball_point;
    entity_manager->inputs[ball] = ball_input;
}

void update_paddle(entity_manager_t* entity_manager, int paddle)
{
    point_t paddle_point = entity_manager->positions[paddle];
    extension_t paddle_extension = entity_manager->sizes[paddle];

    if(paddle_point.y <= 0)
    {
        paddle_point.y = 0;
    }
    else if(paddle_point.y + paddle_extension.h - 1 >= PIXELS_HEIGHT - 1)
    {
        paddle_point.y = PIXELS_HEIGHT - paddle_extension.h;
    }

    entity_manager->positions[paddle] = paddle_point;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool pressing = action == GLFW_PRESS || action == GLFW_REPEAT;

    switch (key)
    {
        case GLFW_KEY_W:
        key_mapping.left_paddle_up = pressing;
            break;
        case GLFW_KEY_S:
        key_mapping.left_paddle_down = pressing;
            break;
        case GLFW_KEY_UP:
        key_mapping.right_paddle_up = pressing;
            break;
        case GLFW_KEY_DOWN:
        key_mapping.right_paddle_down = pressing;
            break;
        default:
            break;
    }
}
