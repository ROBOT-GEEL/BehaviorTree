#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import Jetson.GPIO as GPIO
import time

class GPIOReaderNode(Node):
    def __init__(self):
        super().__init__('gpio_reader_node')
        self.publisher_ = self.create_publisher(String, 'gpio_state', 10)

        GPIO.setmode(GPIO.BOARD)  # BOARD nummering
        self.PIN = 12              # Pas aan naar jouw pin
        GPIO.setup(self.PIN, GPIO.IN)

        self.timer = self.create_timer(0.5, self.read_gpio)

    def read_gpio(self):
        msg = String()
        if GPIO.input(self.PIN):
            msg.data = "HIGH"
        else:
            msg.data = "LOW"
        self.publisher_.publish(msg)
        self.get_logger().info(f'GPIO Pin {self.PIN} is {msg.data}')

    def destroy_node(self):
        GPIO.cleanup()
        super().destroy_node()

def main(args=None):
    rclpy.init(args=args)
    node = GPIOReaderNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()

