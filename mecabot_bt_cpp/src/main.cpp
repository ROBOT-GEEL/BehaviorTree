#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp_v3/bt_factory.h>
#include <iostream>
#include <behaviortree_cpp_v3/loggers/bt_zmq_publisher.h>

// === Behavior node 1 ===
class SayHello : public BT::SyncActionNode
{
public:
  SayHello(const std::string &name) : BT::SyncActionNode(name, {}) {}
  BT::NodeStatus tick() override
  {
    std::cout << "[SayHello] Hello from Mecabot!" << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

// === Behavior node 2 ===
class CheckBattery : public BT::SyncActionNode
{
public:
  CheckBattery(const std::string &name) : BT::SyncActionNode(name, {}) {}
  BT::NodeStatus tick() override
  {
    int battery = rand() % 100; // Simuleer batterijpercentage
    std::cout << "[CheckBattery] Battery level: " << battery << "%" << std::endl;

    if (battery < 30)
    {
      std::cout << "[CheckBattery] Battery too low!" << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    else
    {
      std::cout << "[CheckBattery] Battery OK!" << std::endl;
      return BT::NodeStatus::SUCCESS;
    }
  }
};

// === Behavior node 3 ===
class Drive : public BT::SyncActionNode
{
public:
  Drive(const std::string &name) : BT::SyncActionNode(name, {}) {}
  BT::NodeStatus tick() override
  {
    std::cout << "[Drive] Mecabot is driving around..." << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);

  // Factory aanmaken en nodes registreren
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<SayHello>("SayHello");
  factory.registerNodeType<CheckBattery>("CheckBattery");
  factory.registerNodeType<Drive>("Drive");

  // XML-tree laden
  auto tree = factory.createTreeFromFile("src/mecabot_bt_cpp/trees/simple_tree.xml");
  BT::PublisherZMQ publisher_zmq(tree);

  // Herhaaldelijk uitvoeren
  for (int i = 0; i < 5; ++i)
  {
    std::cout << "\n--- Tick " << i + 1 << " ---" << std::endl;
    tree.tickRoot();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  rclcpp::shutdown();
  return 0;
}

