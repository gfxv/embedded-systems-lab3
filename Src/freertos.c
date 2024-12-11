/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"---------------------------------------------------------*/
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "kb.h"
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct snake {
	struct snake *prev;
	struct snake *next;
	uint32_t x;
	uint32_t y;
} snake;

enum direction {
	LEFT,
	UP,
	RIGHT,
	DOWN
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint32_t random_seed();
void print_start_screen();
void print_end_screen(size_t score);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
bool game_started = false;
uint8_t direction = LEFT;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osMessageQId myQueue01Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of myQueue01 */
  osMessageQDef(myQueue01, 16, uint16_t);
  myQueue01Handle = osMessageCreate(osMessageQ(myQueue01), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, StartTask02, osPriorityNormal, 0, 128);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}


/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	oled_Init();

	uint32_t width_screen = 128;
	uint32_t height_screen = 64;
	uint32_t width_snake = 2;
	uint32_t height_snake = 2;
	uint32_t x_cell = width_screen / width_snake;
	uint32_t y_cell = height_screen / height_snake;
	osEvent event;

	for (;;) {
		print_start_screen();

		while(true) {
			event = osMessageGet(myQueue01Handle, 100);
			if (event.status == osEventMessage) {
				break;
			}
		}

		direction = LEFT;
		size_t score = 0;
		event = osMessageGet(myQueue01Handle, 100);
		while (event.status == osEventMessage) {
			event = osMessageGet(myQueue01Handle, 100);
		}
		oled_Reset();
		snake *head = (snake*) pvPortMalloc(sizeof(snake));
		head->x = x_cell/2;
		head->y = y_cell/2;
		snake *tail = head;
		for (int i = 1; i < 5; i++) {
			snake *node = (snake*) pvPortMalloc(sizeof(snake));
			node->next = tail;
			node->x = x_cell/2 + i;
			node->y = y_cell/2;
			tail->prev = node;
			tail = node;
		}
		snake *current_node = head;
		while (current_node != NULL) {
			uint16_t x1 = current_node->x * width_snake, y1 = current_node->y * height_snake;
			uint16_t x2 = x1 + width_snake, y2 = y1 + height_snake;
			oled_DrawSquare(x1, x2, y1, y2, White);
			oled_UpdateScreen();
			osDelay(10);
			current_node = current_node->prev;
		}

		game_started = true;


		int16_t xR = -1, yR = -1;
		while(true) {
			event = osMessageGet(myQueue01Handle, 100);
			if (event.status == osEventMessage) {
				direction = event.value.v;
			}

			snake * node = (snake*) pvPortMalloc(sizeof(snake));
			node->x = head->x;
			node->y = head->y;
			switch (direction) {
				case DOWN:
					node->y += width_snake;
					break;
				case UP:
					node->y -= width_snake;
					break;
				case RIGHT:
					node->x += width_snake;
					break;
				case LEFT:
					node->x -= width_snake;
					break;
			}
			head->next = node;
			node->prev = head;
			head = node;

			uint16_t x1 = head->x * width_snake;
			uint16_t y1 = head->y * height_snake;
			uint16_t x2 = x1 + width_snake;
			uint16_t y2 = y1 + height_snake;
			oled_DrawSquare(x1, x2, y1, y2, White);

			if (head->x == xR && head->y == yR) {
				xR = -1;
				yR = -1;
				score += 1;
			} else {
				x1 = tail->x * width_snake;
				y1 = tail->y * height_snake;
				x2 = x1 + width_snake;
				y2 = y1 + height_snake;
				oled_DrawSquare(x1, x2, y1, y2, Black);

				tail = tail->next;
				vPortFree(tail->prev);
				tail->prev = NULL;
			}


			if (head->x >= x_cell || head->y >= y_cell || head->x < 0 || head->y < 0) {
				break;
			}

			snake *current_node = head->prev;
			uint8_t flag = 0x00;
			while (current_node != NULL) {
				if (head->x == current_node->x && head->y == current_node->y) {
					flag = 0x01;
					break;
				}
				current_node = current_node->prev;
			}
			if (flag == 0x01) {
				break;
			}

			if (xR == -1 && yR == -1) {
				for (;;) {
					xR = (int16_t) random_seed() % (x_cell - 1);;
					xR -= xR % width_snake;
					yR = (int16_t) random_seed() % (y_cell - 1);
					yR -= yR % width_snake;
					current_node = head;
					flag = 0x00;
					while (current_node) {
						if (xR == current_node->x && yR == current_node->y) {
							flag = 0x01;
							break;
						}
						current_node = current_node->prev;
					}
					if (flag == 0x00) {
						break;
					}
				}

			}
			oled_DrawSquare(xR * width_snake, (xR + 1) * width_snake,
					yR * height_snake, (yR + 1) * height_snake, White);

			oled_UpdateScreen();
			osDelay(100);
		}

		current_node = head;
		while (current_node) {
			uint16_t x1 = current_node->x * width_snake;
			uint16_t y1 = current_node->y * height_snake;
			uint16_t x2 = x1 + width_snake;
			uint16_t y2 = y1 + height_snake;
			oled_DrawSquare(x1, x2, y1, y2, Black);
			oled_UpdateScreen();
			osDelay(10);
			current_node = current_node->prev;
		}

		oled_Fill(Black);
		oled_UpdateScreen();
		osDelay(200);

		OLED_COLOR color = White;
		for (int i = 0; i < 4; i++) {
			oled_Fill(color);
			oled_UpdateScreen();
			osDelay(100);
			color = color == White ? Black : White;
		}


		print_end_screen(score);
		game_started = false;

		while(true) {
			event = osMessageGet(myQueue01Handle, 100);
			if (event.status == osEventMessage) {
				break;
			}
		}
	}

  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
	for (;;) {
		uint8_t Row[4] = {ROW4, ROW3, ROW2, ROW1};
		for ( int i = 0; i < 4; i++ ) {
			uint8_t Key = Check_Row( Row[i] );

			if (game_started) {
				if (i == 1 && Key == 0x02 && direction != UP && direction != DOWN) {
					direction = DOWN;
					osMessagePut(myQueue01Handle, direction, 100);
				} else if ( i == 2 && Key == 0x04 && direction != RIGHT && direction != LEFT) {
					direction = LEFT;
					osMessagePut(myQueue01Handle, direction, 100);
				} else if ( i == 2 && Key == 0x01 && direction != LEFT && direction != RIGHT) {
					direction = RIGHT;
					osMessagePut(myQueue01Handle, direction, 100);
				} else if ( i == 3 && Key == 0x02 && direction != DOWN && direction != UP) {
					direction = UP;
					osMessagePut(myQueue01Handle, direction, 100);
				}
			} else {
				if (Key) osMessagePut(myQueue01Handle, 0, 100);
			}

		}
		osDelay(10);
	}
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
uint32_t random_seed() {
	uint32_t seed = xTaskGetTickCount();
	static const uint32_t A = 25234532, C = 4366346, M = 428942347;

	return (A * seed + C) % M;
}

void print_start_screen() {
	static char *snake_game_logo = "SNAKE GAME";
	static char *press_text = "press any button";

	oled_Fill(Black);

	oled_SetCursor(9, 10);
	oled_WriteString(snake_game_logo, Font_11x18, White);

	oled_SetCursor(8, 40);
	oled_WriteString(press_text, Font_7x10, White);

	oled_UpdateScreen();
}


void print_end_screen(size_t score) {
	static char *game_over_logo = "GAME OVER";
	static char score_text[12];

	sprintf(score_text, "score: %04d", score);


	oled_Fill(Black);

	oled_SetCursor(14, 7);
	oled_WriteString(game_over_logo, Font_11x18, White);

	oled_SetCursor(25, 30);
	oled_WriteString(score_text, Font_7x10, White);

	oled_UpdateScreen();
}
/* USER CODE END Application */
