#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>

Servo sL, sR;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

rcl_subscription_t subscriber;
std_msgs__msg__String msg;

void subscription_callback(const void *msgin) {
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  String cmd = String(msg->data.data);
  
  if (cmd.startsWith("EXP:")) { /* Trigger OLED */ }
  else if (cmd.startsWith("SRVL:")) { sL.write(cmd.substring(5).toInt()); }
}

void setup() {
  // IMPORTANT: Set this to the IP assigned to your Termux/Linux environment
  set_microros_wifi_transports("YOUR_HOTSPOT_NAME", "YOUR_PASSWORD", "192.168.4.2", 8888);
  
  sL.attach(13); sR.attach(12);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "alex_bot", "", &support);
  rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "alex_cmd");
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}
