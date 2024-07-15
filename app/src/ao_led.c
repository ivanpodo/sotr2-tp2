/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"
#include "timers.h"
#include "board.h"
#include "logger.h"
#include "dwt.h"

#include "ao_led.h"

/********************** macros and definitions *******************************/
#define QUEUE_AO_LED_LENGTH_            (5)
#define QUEUE_AO_LED_ITEM_SIZE_         (sizeof(ao_led_message_t))
#define LED_ON_PERIOD_MS_				(TickType_t)(1000U / portTICK_PERIOD_MS)

#define WAIT_TIME   0U
#define LED_ON_TIME 500U

/********************** internal data declaration ****************************/

ao_led_handle_t ao_led =
				{
					.info[RED].port 	= LD3_GPIO_Port,
					.info[RED].pin 	= LD3_Pin,
					.info[RED].state = GPIO_PIN_RESET,
					.info[RED].colour = "RED",

					.info[GREEN].port 	= LD1_GPIO_Port,
					.info[GREEN].pin 	= LD1_Pin,
					.info[GREEN].state = GPIO_PIN_RESET,
					.info[GREEN].colour = "GREEN",
					
					.info[BLUE].port 	= LD2_GPIO_Port,
					.info[BLUE].pin 	= LD2_Pin,
					.info[BLUE].state = GPIO_PIN_RESET,
					.info[BLUE].colour = "BLUE",
				};

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void ao_task_(void *argument)
{
  ao_led_handle_t *hao = (ao_led_handle_t *)argument;

  LOGGER_INFO("AO LED started")

  while (true)
  {
    ao_led_message_t msg;

    LOGGER_INFO("AO LED \t- Waiting event");

	/* hacemos? bool ao_running = true */

    while (pdPASS == xQueueReceive(hao->hqueue, &msg, portMAX_DELAY))
    {
      HAL_GPIO_WritePin(hao->info[msg.colour].port, hao->info[msg.colour].pin, GPIO_PIN_SET);
	  vTaskDelay(pdMS_TO_TICKS(LED_ON_TIME));
      HAL_GPIO_WritePin(hao->info[msg.colour].port, hao->info[msg.colour].pin, GPIO_PIN_RESET);
    }

	LOGGER_INFO("AO LED - releasing memory");

	/* hacemos? bool ao_running = false */
	
	/*
	 * librar queue
	 * asignar null al ptr
	 * */

	vTaskDelete(NULL);

	/* No se ejecuta*/
  }
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_handle_t* hao_led, ao_led_message_t msg)
{
	/* (verificar si )el ao esta corriendo
	 * crear queue, reasignar puntero
	 * asignar null al ptr
	 * enviar evento a la cola
	 * crear tarea, reasignar puntero
	 * */
  return (pdPASS == xQueueSend(hao_led->hqueue, (void*)&msg, (TickType_t)0U));
}

/*
 * Hacemos init o no hace falta? porque si vamos a crear y destruir queue y task,
 * no es requerido. Podemos utilizarlo en 'ao_led_send()
 */

void ao_leds_init(ao_led_handle_t* hao_led)
{
  // Queues
  hao_led->hqueue = xQueueCreate(QUEUE_AO_LED_LENGTH_, QUEUE_AO_LED_ITEM_SIZE_);
  configASSERT(NULL != hao_led->hqueue);

  // Tasks
  BaseType_t status;
  status = xTaskCreate
		  (
			  ao_task_,
			  "task_ao_led",
			  128,
			  (void* const)hao_led,
			  (tskIDLE_PRIORITY + 1),
			  &hao_led->htask
		  );

  configASSERT(pdPASS == status);
}

/********************** end of file ******************************************/
