#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>

// Global ROS 2 variables
rcl_subscription_t subscriber;
std_msgs__msg__String msg;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rclc_executor_t executor;

Servo sL, sR;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void subscription_callback(const void *msgin) {
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  String cmd = String(msg->data.data);
  
  if (cmd.startsWith("EXP:")) { /* Trigger OLED */ }
  else if (cmd.startsWith("SRVL:")) { sL.write(cmd.substring(5).toInt()); }
}

void setup() {
  // Use char arrays for transport credentials
  char ssid[] = "YOUR_HOTSPOT_NAME";
  char pass[] = "YOUR_PASSWORD";
  char ip[] = "192.168.4.2"; // Replace with your Termux environment IP
  
  set_microros_wifi_transports(ssid, pass, ip, 8888);
  
  delay(2000); // Allow time for network connection
  
  sL.attach(13); 
  sR.attach(12);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "alex_bot", "", &support);
  
  rclc_subscription_init_default(&subscriber, &node, 
                                 ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), 
                                 "alex_cmd");
  
  executor = rclc_executor_get_zero_initialized_executor();
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &subscriber, &msg, 
                                 &subscription_callback, ON_NEW_DATA);
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}
