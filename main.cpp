#include <stdlib.h>
#include <cmath>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 640;

const unsigned int PIXELS_WIDTH = 128;
const unsigned int PIXELS_HEIGHT = 64;

const int PADDLE_WIDTH = 1;
const int PADDLE_HEIGHT = 8;

int numbers[][15] = {
    {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
    },
    {
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    }
};

typedef enum
{
    IDLE,
    PREPARATION,
    GAMEPLAY,
    POINT
} game_state_t;

typedef enum {
    EXTENSION,
    POSITION,
    MOVEMENT,
    RENDERER
} component_uid_t;

typedef struct
{
    float x, y;
    int w, h;
    float speed;
    bool visible;
} entity_resource_t;

typedef struct
{
    int w, h;
} extension_t;

typedef struct
{
    float x, y;
    int pixel_x, pixel_y;
} position_t;

typedef struct
{
    float dir_x, dir_y;
    float speed;
} movement_t;

typedef struct
{
    bool visible;
} renderer_t;

typedef struct
{
    unsigned int components[3];

    extension_t extensions[3];
    position_t position[3];
    movement_t movements[3];
    renderer_t renderers[3];

    int length;
} entity_manager_t;

typedef struct
{
    int left_paddle_up;
    int left_paddle_down;
    int right_paddle_up;
    int right_paddle_down;
    int enter;
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

    entity_resource_t left_paddle_rsc = {2, 2, PADDLE_WIDTH, PADDLE_HEIGHT, 1, false};
    entity_resource_t right_paddle_rsc = {PIXELS_WIDTH - 3, 15, PADDLE_WIDTH, PADDLE_HEIGHT, 1, false};
    entity_resource_t ball_rsc = {PIXELS_WIDTH / 2, PIXELS_HEIGHT / 2, 1, 1, 1, true};

    entity_manager_t entity_manager = {};
    game_state_t game_state = IDLE;
    int left_score = 0, right_score = 0;
    int point = 0;
    float seconds = 0;

    int left_paddle = create_entity(&entity_manager, EXTENSION | POSITION | MOVEMENT);
    setup_component(&entity_manager, left_paddle, left_paddle_rsc);

    int right_paddle = create_entity(&entity_manager, EXTENSION | POSITION | MOVEMENT);
    setup_component(&entity_manager, right_paddle, right_paddle_rsc);

    int ball = create_entity(&entity_manager, EXTENSION | POSITION | MOVEMENT);
    setup_component(&entity_manager, ball, ball_rsc);
    entity_manager.movements[ball].dir_x = rand() % 2 == 0 ? 1 : -1;
    entity_manager.movements[ball].dir_y = rand() % 2 == 0 ? 1 : -1;

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
            entity_manager.movements[left_paddle].dir_y = 0;
            if(key_mapping.left_paddle_up)
            {
                entity_manager.movements[left_paddle].dir_y += 1;
            }
            if(key_mapping.left_paddle_down)
            {
                entity_manager.movements[left_paddle].dir_y -= 1;
            }

            entity_manager.movements[right_paddle].dir_y = 0;
            if(key_mapping.right_paddle_up)
            {
                entity_manager.movements[right_paddle].dir_y += 1;
            }
            if(key_mapping.right_paddle_down)
            {
                entity_manager.movements[right_paddle].dir_y -= 1;
            }

            movement_system(&entity_manager);

            switch(game_state)
            {
                case IDLE:
                {
                    if(key_mapping.enter)
                    {
                        entity_manager.movements[ball].dir_x = rand() % 2 == 0 ? 1 : -1;
                        entity_manager.movements[ball].dir_y = rand() % 2 == 0 ? 1 : -1;

                        entity_manager.renderers[ball].visible = false;
                        entity_manager.renderers[left_paddle].visible = true;
                        entity_manager.renderers[right_paddle].visible = true;

                        game_state = PREPARATION;
                    }
                    break;
                }
                case PREPARATION:
                {
                    seconds += deltaTime;
                    if(seconds >= 2.0)
                    {
                        setup_component(&entity_manager, ball, ball_rsc);
                        entity_manager.movements[ball].dir_y = rand() % 2 == 0 ? 1 : -1;

                        seconds = 0.0;
                        point = 0;
                        game_state = GAMEPLAY;
                    }
                    break;
                }
                case GAMEPLAY:
                {
                    position_t ball_point = entity_manager.position[ball];
                    extension_t ball_extension = entity_manager.extensions[ball];

                    if(ball_point.x <= 0 || (ball_point.x + ball_extension.w - 1) >= PIXELS_WIDTH - 1)
                    {
                        if(ball_point.x <= 0)
                        {
                            point = -1;
                            left_score++;
                        } 
                        else
                        {
                            point = 1;
                            right_score++;
                        }

                        entity_manager.renderers[ball].visible = false;
                        game_state = POINT;
                    }
                    break;
                }
                case POINT:
                {
                    game_state = PREPARATION;

                    if(left_score >= 10 || right_score >= 10)
                    {
                        setup_component(&entity_manager, ball, ball_rsc);
                        setup_component(&entity_manager, left_paddle, left_paddle_rsc);
                        setup_component(&entity_manager, right_paddle, right_paddle_rsc);

                        left_score = right_score = 0;

                        game_state = IDLE;
                    }
                    break;
                }
            }

            update_paddle(&entity_manager, left_paddle);
            update_paddle(&entity_manager, right_paddle);

            int entities[2] = {left_paddle, right_paddle};
            update_ball(&entity_manager, ball, entities);

            renderer_system(&entity_manager, pixels_buffer);

            const int x_offset = 4;
            const int y_offset = 2;
            for (int i = 0; i < 15; i++)
            {
                int pixel = numbers[right_score][i];

                if(pixel == 0) continue;

                int index = ((((PIXELS_WIDTH / 2) - 3 - x_offset) + (i % 3)) + ((PIXELS_HEIGHT - 1) - (i / 3) - y_offset) * PIXELS_WIDTH) * 3;
                pixels_buffer[index] = 255;
                pixels_buffer[index + 1] = 255;
                pixels_buffer[index + 2] = 255;
            }
            for (int i = 0; i < 15; i++)
            {
                int pixel = numbers[left_score][i];

                if(pixel == 0) continue;

                int index = (((PIXELS_WIDTH / 2) + (i % 3) + x_offset) + ((PIXELS_HEIGHT - 1) - (i / 3) - y_offset) * PIXELS_WIDTH) * 3;
                pixels_buffer[index] = 255;
                pixels_buffer[index + 1] = 255;
                pixels_buffer[index + 2] = 255;
            }
        }

        // render
        glClear(GL_COLOR_BUFFER_BIT);
        glPixelZoom(SCR_WIDTH / PIXELS_WIDTH, SCR_HEIGHT / PIXELS_HEIGHT);
        glDrawPixels(PIXELS_WIDTH, PIXELS_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels_buffer);

        glLineStipple(10, 0xAAAA);
        glEnable(GL_LINE_STIPPLE);
        glLineWidth(3);
        glBegin(GL_LINES);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2f(0.0f, -1.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();

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

    if((components_mask & EXTENSION) == EXTENSION)
    {
        entity_manager->extensions[entity].w = resource.w;
        entity_manager->extensions[entity].h = resource.h;
    }

    if((components_mask & POSITION) == POSITION)
    {
        entity_manager->position[entity].x = resource.x;
        entity_manager->position[entity].y = resource.y;
    }

    if((components_mask & MOVEMENT) == MOVEMENT)
    {
        entity_manager->movements[entity].speed = resource.speed;
    }

    if((components_mask & RENDERER) == RENDERER)
    {
        entity_manager->renderers[entity].visible = resource.visible;
    }
}

void movement_system(entity_manager_t* entity_manager)
{
    const unsigned int REQUIRED_COMPONENTS = EXTENSION | POSITION | MOVEMENT;
    for (int entity = 0; entity < entity_manager->length; entity++)
    {
        unsigned int components_mask = entity_manager->components[entity];

        if((components_mask & REQUIRED_COMPONENTS) != REQUIRED_COMPONENTS) continue;

        extension_t size = entity_manager->extensions[entity];
        position_t position = entity_manager->position[entity];
        movement_t movement = entity_manager->movements[entity];

        //float m = sqrt(movement.dir_x * movement.dir_x + movement.dir_y * movement.dir_y);
        //movement.dir_x /= m;
        //movement.dir_y /= m;

        position.x += movement.dir_x * movement.speed;
        position.y += movement.dir_y * movement.speed;

        position.pixel_x = static_cast <int> (position.x);
        position.pixel_y = static_cast <int> (position.y);

        entity_manager->position[entity] = position;
    }
}

void renderer_system(entity_manager_t* entity_manager, GLubyte* pixels_buffer)
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

    const unsigned int REQUIRED_COMPONENTS = EXTENSION | POSITION | RENDERER;
    for (int entity = 0; entity < entity_manager->length; entity++)
    {
        unsigned int components_mask = entity_manager->components[entity];

        if((components_mask & REQUIRED_COMPONENTS) != REQUIRED_COMPONENTS) continue;

        renderer_t renderer = entity_manager->renderers[entity];
        if(renderer.visible == false) continue;

        extension_t size = entity_manager->extensions[entity];
        position_t position = entity_manager->position[entity];
        for (int w = 0; w < size.w; w++)
        {
            for (int h = 0; h < size.h; h++)
            {
                int i = ((position.pixel_x + w) + (position.pixel_y + h) * PIXELS_WIDTH) * 3;
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

    position_t ball_position = entity_manager->position[ball];
    extension_t ball_extension = entity_manager->extensions[ball];
    movement_t ball_movement = entity_manager->movements[ball];

    if(ball_position.x <= 0 || (ball_position.x + ball_extension.w - 1) >= PIXELS_WIDTH - 1)
    {
        ball_position.x = ball_position.x <= 0 ? 0 : PIXELS_WIDTH - ball_extension.w;
        ball_movement.dir_x *= -1;
    }
    if(ball_position.y <= 0 || (ball_position.y + ball_extension.h - 1) >= PIXELS_HEIGHT - 1)
    {
        ball_position.y = ball_position.y <= 0 ? 0 : PIXELS_HEIGHT - ball_extension.h;
        ball_movement.dir_y *= -1;
    }

    for (int i = 0; i < 2; i++)
    {
        if(entity_manager->renderers[paddles[i]].visible == false) continue;

        position_t paddle_point = entity_manager->position[paddles[i]];
        extension_t paddle_extension = entity_manager->extensions[paddles[i]];

        if(ball_position.x >= paddle_point.x && ball_position.x <= paddle_point.x + paddle_extension.w - 1)
        {
            if(ball_position.y >= paddle_point.y && ball_position.y <= paddle_point.y + paddle_extension.h - 1)
            {
                if(ball_movement.dir_x == -1)
                {
                    ball_position.x = paddle_point.x + paddle_extension.w;
                } 
                else if(ball_movement.dir_x == 1)
                {
                    ball_position.x = paddle_point.x - ball_extension.w;
                }

                ball_movement.dir_x *= -1;
                //ball_movement.speed += 0.1f;
            }
        }
    } 

    entity_manager->position[ball] = ball_position;
    entity_manager->movements[ball] = ball_movement;
}

void update_paddle(entity_manager_t* entity_manager, int paddle)
{
    if(entity_manager->renderers[paddle].visible == false) return;

    position_t paddle_point = entity_manager->position[paddle];
    extension_t paddle_extension = entity_manager->extensions[paddle];

    if(paddle_point.y <= 0)
    {
        paddle_point.y = 0;
    }
    else if(paddle_point.y + paddle_extension.h - 1 >= PIXELS_HEIGHT - 1)
    {
        paddle_point.y = PIXELS_HEIGHT - paddle_extension.h;
    }

    entity_manager->position[paddle] = paddle_point;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool pressing = action == GLFW_PRESS || action == GLFW_REPEAT;

    switch (key)
    {
        case GLFW_KEY_ENTER:
        key_mapping.enter = action == GLFW_PRESS;
            break;
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
