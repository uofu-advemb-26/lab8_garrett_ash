#include <FreeRTOS.h>
#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <pico/stdlib.h>
#include <queue.h>
#include <stdio.h>
#include <task.h>

static struct can2040 cbus;

QueueHandle_t messages;

static void can2040_cb(struct can2040 *cd, uint32_t notify,
                       struct can2040_msg *msg) {
  // Send the received message to the queue, spend less time in the interrupt
  // handler
  if (xQueueSendToBack(messages, msg, 50) == pdPASS)
    printf("successfully added to queue\n");
  else
    printf("failed to add to queue\n");
}

static void PIOx_IRQHandler(void) { can2040_pio_irq_handler(&cbus); }

void canbus_setup(void) {
  uint32_t pio_num = 0;
  uint32_t sys_clock = 125000000, bitrate = 500000;
  uint32_t gpio_rx = 4, gpio_tx = 5;

  // Setup canbus
  can2040_setup(&cbus, pio_num);
  can2040_callback_config(&cbus, can2040_cb);

  // Enable irqs
  irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
  irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
  irq_set_enabled(PIO0_IRQ_0, 1);

  // Start canbus
  can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

// Task to receive CAN messages
void canbus_task_receive_function(void *params) {
  struct can2040_msg msg;
  while (1) {
    vTaskDelay(500);
    printf("receiving...\n");
    if (xQueueReceive(messages, &msg, 50))
      printf("Received CAN message\n");
    else
      printf("Did NOT receive CAN msg\n");
  }
}

// Task to send CAN messages
void canbus_task_send_function(void *params) {
  struct can2040_msg msg;
  msg.id = 0x123; // identifier for the message
  msg.dlc = 8;    // number of data bytes
  for (int i = 0; i < 8; i++) {
    msg.data[i] = i;
  }
  while (1) {
    can2040_transmit(&cbus, &msg);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void canbus_task_send_and_recv_cont(void *params) {
  struct can2040_msg sent_msg, recv_msg;

  // send init
  sent_msg.id = 0x123;
  sent_msg.dlc = 8;
  for (int i = 0; i < 8; i++)
    sent_msg.data[i] = i;

  while (1) {
    printf("sending...\n");
    can2040_transmit(&cbus, &sent_msg);
    printf("receiving...\n");
    if (xQueueReceive(messages, &recv_msg, 50))
      printf("Received CAN message\n");
    else
      printf("Did NOT receive CAN msg\n");
  }
}

int main(void) {
  stdio_init_all();
  printf("connected or something\n");
  messages = xQueueCreate(100, sizeof(struct can2040_msg));
  canbus_setup();
  TaskHandle_t canbus_task;
  xTaskCreate(canbus_task_receive_function, "CAN_Bus_Task",
              configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL,
              &canbus_task);
  vTaskStartScheduler();
  return 0;
}
