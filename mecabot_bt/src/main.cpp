#include "rclcpp/rclcpp.hpp"
#include "behaviortree_cpp_v3/bt_factory.h"
#include <chrono>
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include <iostream>
#include <std_msgs/msg/float32.hpp> 
#include <std_msgs/msg/bool.hpp>  
#include <geometry_msgs/msg/twist.hpp>


using namespace std::chrono_literals;


// -------------
// Decorator atijd-SUCCES
class ForceSuccess : public BT::DecoratorNode
{
public:
    ForceSuccess(const std::string& name, const BT::NodeConfiguration& config)
        : BT::DecoratorNode(name, config) {}

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus tick() override
    {
        const BT::NodeStatus child_state = child_node_->executeTick();

        if (child_state == BT::NodeStatus::RUNNING) {
            return BT::NodeStatus::RUNNING;
        }
        // Altijd SUCCESS teruggeven, ongeacht wat het kind doet
        return BT::NodeStatus::SUCCESS;
    }
};

class StopNode : public BT::SyncActionNode
{
public:
    StopNode(const std::string &name) : BT::SyncActionNode(name, {}) {
        node_ = rclcpp::Node::make_shared("btStopNode");
        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }
    BT::NodeStatus tick() override {
        std::cout << "[StopNode] STOP DRIVING" << std::endl;
        std::string state = "StopNode";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_->publish(msg);
        return BT::NodeStatus::SUCCESS;
    }
        private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class WaitDriving : public BT::SyncActionNode
{
public:
    WaitDriving(const std::string &name) : BT::SyncActionNode(name, {}) {
    node_ = rclcpp::Node::make_shared("btWaitDriving");
    pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);}
    BT::NodeStatus tick() override {
        std::string state = "WaitDriving";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_->publish(msg);
        std::cout << "[WaitDriving] STOP DRIVING" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
        private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class CheckNetworkError : public BT::StatefulActionNode
{
public:
    CheckNetworkError(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), level_(100.0)
    {


    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override
    {


        // Als batterij al te laag is, meteen FAILURE
        if (level_ < 30.0)
        {
	std::cout << "[CheckNetworkError] NETWORK ERROR -> FAILURE. Level: " << level_ << std::endl;
	level_ += 0;

            return BT::NodeStatus::FAILURE;
        }

        // Anders RUNNING totdat de volgende tick komt
        level_ -= 0.0;
        std::cout << "[CheckNetworkError] NETWORK oke: " << level_ << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        // Simuleer dat de batterij afneemt
        level_ -= 0;


        if (level_ < 30.0)
        {
            level_ += 0;
	std::cout << "[CheckNetworkError] NETWORK ERROR -> FAILURE. Level: " << level_ << std::endl;
            return BT::NodeStatus::FAILURE;
        }
        std::cout << "[CheckNetworkError] NETWORK oke: " << level_ << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[CheckNetworkError] HALTED" << std::endl;
    }

private:
    double level_;
};

class CheckCollision : public BT::StatefulActionNode
{
public:
    CheckCollision(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), level_(100.0)
    {}

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override
    {
       

        // Als batterij al te laag is, meteen FAILURE
        if (level_ < 30.0)
        {
            std::cout << "[CheckCollision] COLLSION!!!" << std::endl;
            level_ += 0;
            return BT::NodeStatus::FAILURE;
        }

        // Anders RUNNING totdat de volgende tick komt
        std::cout << "[CheckCollision] noCollisionDetected" << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        
        level_ -= 0;

        if (level_ < 30.0)
        {
            std::cout << "[CheckCollision] COLLSION!!!" << std::endl;
            level_ += 0;
            return BT::NodeStatus::FAILURE;
        }
        std::cout << "[CheckCollision] noCollisionDetected" << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[CheckCollision] HALTED" << std::endl;
    }

private:
    double level_;
};

// KOPPEL ONDERAAN BIJ FACTORY.REGISTERNODE DEZE NODE AAN DE XML NODE VOOR SIMULATIE (FOUTEN SIMULEREN) 
class BatterySimOk : public BT::StatefulActionNode
{
public:
    BatterySimOk(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), level_(100.0)
    {}

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override
    {
        std::cout << "[BatteryOk] Starting check, level = " << level_ << "%" << std::endl;
        level_ -= 0.0;
        // Als batterij al te laag is, meteen FAILURE
        if (level_ < 30.0)
        {
      
            std::cout << "[BatteryOk] Battery too low! -> FAILURE" << std::endl;
            level_ += 0;
            return BT::NodeStatus::FAILURE;
        }

        // Anders RUNNING totdat de volgende tick komt
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        // Simuleer dat de batterij afneemt
        level_ -= 0.0;
        std::cout << "[BatteryOk] Battery level = " << level_ << "% -> RUNNING" << std::endl;

        if (level_ < 30.0)
        {
            std::cout << "[BatteryOk] Battery too low! -> FAILURE" << std::endl;
            level_ += 0;
            return BT::NodeStatus::FAILURE;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[BatteryOk] HALTED" << std::endl;
    }

private:
    double level_;
};

// GEBRUIK DEZE BATTERIJCHECK VOOR DE ECHTE FUNCTIONALITEIT TE TESTEN
class BatteryOk : public BT::StatefulActionNode
{
public:
    BatteryOk(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("bt_BatteryOk_node");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/auto_recharge_event", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                last_event_ = msg->data;
            });
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override
    {
        rclcpp::spin_some(node_);
        std::cout << "[BatteryOk] START, last_event=" << last_event_ << std::endl;

        if (last_event_ == "BATTERY-LOW")
        {
            std::cout << "[BatteryOk] Battery low -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);
        if (last_event_ == "BATTERY-LOW")
        {
            std::cout << "[BatteryOk] Battery low detected -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        std::cout << "[BatteryOk] Battery OK -> RUNNING" << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[BatteryOk] HALTED" << std::endl;
    }

private:
    std::string last_event_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};


// Op termijn zal dit verwijdert worden wegens redundante info 
class TimedCondition : public BT::StatefulActionNode
{
public:
    TimedCondition(const std::string &name, const BT::NodeConfiguration &config)
    : BT::StatefulActionNode(name, config), timeout_(7.0)
    {
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<double>("timeout") };
    }

    BT::NodeStatus onStart() override
    {
        // haal timeout uit XML, default 7s
        if (!getInput<double>("timeout", timeout_))
        {
            timeout_ = 7.0;
        }

        start_time_ = std::chrono::steady_clock::now();
        std::cout << "[" << name() << "] START with timeout = " << timeout_ << "s" << std::endl;

        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time_).count();

        std::cout << "[" << name() << "] Running... (" << elapsed << "s)" << std::endl;

        if (elapsed >= timeout_)
        {
            std::cout << "[" << name() << "] Timeout reached " << std::endl;
            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[" << name() << "] HALTED" << std::endl;
    }

protected:
    double timeout_;
    std::chrono::steady_clock::time_point start_time_;
};


class InWorkingZone : public BT::SyncActionNode
{
public:
    InWorkingZone(const std::string &name) : BT::SyncActionNode(name, {}) {
       node_ = rclcpp::Node::make_shared("btInWorkingZone");
       pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
       }
    BT::NodeStatus tick() override {
        std::cout << "[InWorkingZone] Checking if in work zone (sim)" << std::endl;
        std::string state = "InWorkingZone";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_->publish(msg);
        return BT::NodeStatus::FAILURE;
    }
        private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class InChargingStation : public BT::SyncActionNode
{
public:
    InChargingStation(const std::string &name)
        : BT::SyncActionNode(name, {}), is_charging(false)
    {
        node_ = rclcpp::Node::make_shared("btInChargingStation");

        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);

        // Subscriber naar /robot_charging_flag
        sub_ = node_->create_subscription<std_msgs::msg::Bool>(
            "/robot_charging_flag", 10,
            [this](std_msgs::msg::Bool::SharedPtr msg_in)
            {
                is_charging = msg_in->data;
            });
    }

    BT::NodeStatus tick() override
    {
        std::string state = "InChargingStation";
        std_msgs::msg::String msg_send;
        msg_send.data = state;
        pub_->publish(msg_send);


        if (is_charging)
        {
            std::cout<< "[InChargingStation] Battery is charging !!! => SUCCESS" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }
        std::cout<< "[InChargingStation] Battery is not charging !!! => FAILURE" << std::endl;
        return BT::NodeStatus::FAILURE;
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
    bool is_charging;
};


class DriveToChargingStation : public BT::StatefulActionNode
{
public:
    DriveToChargingStation(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config),
          success_received_(false), timeout_(5.0)
    {
        node_ = rclcpp::Node::make_shared("btDriveToChargingStation");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/auto_recharge_event", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                if (msg->data == "DRIVING-TO-DOCK")
                    success_received_ = true;
            });

        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<double>("timeout") };
    }

    BT::NodeStatus onStart() override
    {
        success_received_ = false;
        start_time_ = std::chrono::steady_clock::now();
        getInput("timeout", timeout_);
        std_msgs::msg::String msg;
        msg.data = "DriveToChargingStation";
        pub_->publish(msg);
        std::cout << "[DriveToChargingStation] START waiting for DRIVING-TO-DOCK" << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);
        if (success_received_)
        {
            std::cout << "[DriveToChargingStation] Received DRIVING-TO-DOCK -> SUCCESS" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }

        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time_).count();

        if (elapsed >= timeout_)
        {
            std::cout << "[DriveToChargingStation] Timeout reached -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        std::cout << "[DriveToChargingStation] Waiting... elapsed=" << elapsed << "s" << std::endl;
        return BT::NodeStatus::RUNNING;
    }
    void onHalted() override
    {
        std::cout << "[DriveToChargingStation] HALTED" << std::endl;
    }

private:
    bool success_received_;
    double timeout_;
    std::chrono::steady_clock::time_point start_time_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class BatteryStopDrive : public BT::SyncActionNode
{
public:
    BatteryStopDrive(const std::string &name) : BT::SyncActionNode(name, {})
    {
        node_ = rclcpp::Node::make_shared("btBatteryStopDrive");
        pub_speed_ = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    }

    BT::NodeStatus tick() override
    {
        geometry_msgs::msg::Twist stop_msg;
        stop_msg.linear.x = 0.0;
        stop_msg.angular.z = 0.0;
        pub_speed_->publish(stop_msg);

        std::cout << "[BatteryStopDrive] Robot stopped" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_speed_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_goal_;
};




class MoveLocationWorkarea : public BT::SyncActionNode
{
public:
    MoveLocationWorkarea(const std::string &name) : BT::SyncActionNode(name, {}) {
            node_ = rclcpp::Node::make_shared("btMoveLocationWorkarea");
        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
}
    BT::NodeStatus tick() override {
        std::string state = "MoveLocationWorkarea";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_->publish(msg);
        std::cout << "[MoveLocationWorkarea] Moving to work area (sim)" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
        private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class RobotExplore : public BT::SyncActionNode
{
public:
    RobotExplore(const std::string &name) : BT::SyncActionNode(name, {}) {
    node_ = rclcpp::Node::make_shared("btRobotExplore");
    pub_quiz_ = node_->create_publisher<std_msgs::msg::String>("/rpitopic", 10);
    pub_bt_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);


    }
    BT::NodeStatus tick() override {
    	std::string state = "RobotExplore";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_quiz_->publish(msg);
        
        std::string bt_state = "RobotExplore";
        std_msgs::msg::String bt_msg;
        bt_msg.data = bt_state;
        pub_bt_->publish(bt_msg);

        std::cout << "[RobotExplore] Exploring environment (sim)" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_quiz_;  // bestaande publisher
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_bt_;    // nieuwe publisher
};


class CheckCoordAvailable : public BT::SyncActionNode
{
public:
    CheckCoordAvailable(const std::string &name) : BT::SyncActionNode(name, {}) {
            node_ = rclcpp::Node::make_shared("btCheckCoordAvailable");
        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }
    BT::NodeStatus tick() override {
        std::string state = "CheckCoordAvailable";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_->publish(msg);
        std::cout << "[CheckCoordAvailable] Checking coordinate availability (sim)" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
        private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class MoveToVisitor : public BT::SyncActionNode
{
public:
    MoveToVisitor(const std::string &name) : BT::SyncActionNode(name, {}) {
        node_ = rclcpp::Node::make_shared("btMoveToVisitor");
        
        // Publisher voor BehaviorTreeNode
        pub_bt_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);

        // Extra publisher voor quiz status (zoals bij RobotExplore)
        pub_quiz_ = node_->create_publisher<std_msgs::msg::String>("/rpitopic", 10);
    }

    BT::NodeStatus tick() override {
        std::string bt_state = "MoveToVisitor";
        std_msgs::msg::String bt_msg;
        bt_msg.data = bt_state;
        pub_bt_->publish(bt_msg);

        std::string quiz_state = "RobotGoToVisitors";
        std_msgs::msg::String quiz_msg;
        quiz_msg.data = quiz_state;
        pub_quiz_->publish(quiz_msg);

        std::cout << "[MoveToVisitor] Moving to visitor (sim), published to /rpitopic and /BehaviorTreeNode" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_bt_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_quiz_;
};


class StatusDriveToChargingDock : public BT::StatefulActionNode
{
public:
    StatusDriveToChargingDock(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config),
          status_(""), timeout_(5.0)
    {
        node_ = rclcpp::Node::make_shared("btStatusDriveToChargingDock");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/auto_recharge_event", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                status_ = msg->data;
            });

        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<double>("timeout") };
    }

    BT::NodeStatus onStart() override
    {
        getInput("timeout", timeout_);
        start_time_ = std::chrono::steady_clock::now();
        std_msgs::msg::String msg;
        msg.data = "StatusDriveToChargingDock";
        pub_->publish(msg);
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);

        if (status_ == "DRIVE-TO-DOCK-SUCCESS")
        {
            std::cout << "[StatusDriveToChargingDock] SUCCESS" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }
        else if (status_ == "DRIVE-TO-DOCK-FAILED" || status_ == "DRIVE-TO-DOCK-CANCELED")
        {
            std::cout << "[StatusDriveToChargingDock] FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time_).count();

        if (elapsed >= timeout_)
        {
            std::cout << "[StatusDriveToChargingDock] Timeout -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {
        std::cout << "[StatusDriveToChargingDock] HALTED" << std::endl;
    }


private:
    std::string status_;
    double timeout_;
    std::chrono::steady_clock::time_point start_time_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class IsRobotCharging : public BT::StatefulActionNode
{
public:
    IsRobotCharging(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config),
          event_(""), timeout_(200.0)
    {
        node_ = rclcpp::Node::make_shared("btIsRobotCharging");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/auto_recharge_event", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                event_ = msg->data;
            });
        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }

    static BT::PortsList providedPorts()
    {
        return { BT::InputPort<double>("timeout") };
    }

    BT::NodeStatus onStart() override
    {
        getInput("timeout", timeout_);
        start_time_ = std::chrono::steady_clock::now();
        std_msgs::msg::String msg;
        msg.data = "IsRobotCharging";
        pub_->publish(msg);
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);

        if (event_ == "DOCKING-FAILED")
            return BT::NodeStatus::FAILURE;

        if (event_ == "ROBOT-CHARGING")
            return BT::NodeStatus::SUCCESS;

        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time_).count();

        if (elapsed >= timeout_)
        {
            std::cout << "[IsRobotCharging] Timeout reached -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        return BT::NodeStatus::RUNNING;
    }
    void onHalted() override {
        std::cout << "[IsRobotCharging] HALTED" << std::endl;
    }


private:
    std::string event_;
    double timeout_;
    std::chrono::steady_clock::time_point start_time_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

class IsBatteryFull : public BT::StatefulActionNode
{
public:
    IsBatteryFull(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("btBatteryFull");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/auto_recharge_event", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                last_event_ = msg->data;
            });
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override
    {
        rclcpp::spin_some(node_);
        if (last_event_ == "CHARGING-COMPLETED")
        {
            std::cout << "[IsBatteryFull] CHARGING COMPLETED -> SUCCESS" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);
        if (last_event_ == "CHARGING-COMPLETED")
            return BT::NodeStatus::SUCCESS;
        return BT::NodeStatus::RUNNING;
    }
    void onHalted() override {
        std::cout << "[IsBatteryFull] HALTED" << std::endl;
    }


private:
    std::string last_event_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};


class robotAtPerson : public BT::SyncActionNode
{
public:
    robotAtPerson(const std::string &name) : BT::SyncActionNode(name, {}) {
    node_ = rclcpp::Node::make_shared("btRobotAtPerson");
    pub_quiz_ = node_->create_publisher<std_msgs::msg::String>("/rpitopic", 10);
            pub_bt_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
}
    BT::NodeStatus tick() override {
    	std::string state = "RobotArrivedAtVisitors";
    	std_msgs::msg::String msg;
        msg.data = state;
        pub_quiz_->publish(msg);
        
        std::string bt_state = "robotAtPerson";
        std_msgs::msg::String bt_msg;
        bt_msg.data = bt_state;
        pub_bt_->publish(bt_msg);

        std::cout << "[robotAtPerson] robot arrived at person" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
    private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_quiz_;
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_bt_;    // nieuwe publisher

};
//VOOR TESTEN ZAL CHECKSTARTBUTTON SUCCES GEVEN ALS NODE NIET JUISTE STATUS GEEFT NA 10 SECONDEN
class CheckStartButton : public BT::StatefulActionNode
{
public:
    CheckStartButton(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), timeout_(10.0), received_(false)
    {
        node_= rclcpp::Node::make_shared("btCheckStartButton");
        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/quiz_status", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                if(msg->data == "on_drive_to_quiz_location") {
                    received_ = true;
                }
            });
    }

    static BT::PortsList providedPorts() {
        return { BT::InputPort<double>("timeout") };
    }

    BT::NodeStatus onStart() override {
        if (!getInput<double>("timeout", timeout_)) {
            timeout_ = 10.0; // default
        }
        start_time_ = std::chrono::steady_clock::now();
        received_ = false;
        std::cout << "[" << name() << "] START waiting for start button or drive_to_quiz, timeout = " << timeout_ << "s" << std::endl;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override {
        auto elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time_).count();

        if (received_) {
            std::cout << "[" << name() << "] Received 'on_drive_to_quiz_location', returning SUCCESS" << std::endl;
            return BT::NodeStatus::FAILURE; // geinverteerd om te kunnen testen !!!!!
        }

        if (elapsed >= timeout_) {
            std::cout << "[" << name() << "] Timeout reached, returning FAILURE" << std::endl;
            return BT::NodeStatus::SUCCESS;  // geinverteerd om te kunnen testen !!!!!
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {
        std::cout << "[" << name() << "] HALTED" << std::endl;
    }

private:
    double timeout_;
    bool received_;
    std::chrono::steady_clock::time_point start_time_;
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};


class DriveQuizLocation : public BT::StatefulActionNode
{
public:
    DriveQuizLocation(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), received_success_(false)
    {
        node_ = rclcpp::Node::make_shared("btDriveQuizLocation");

        pub_bt_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
        pub_coord_ = node_->create_publisher<geometry_msgs::msg::PoseStamped>("/btDriveCoord", 10);

        sub_ack_ = node_->create_subscription<std_msgs::msg::String>(
            "/driveCoordStatus", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                // Verwacht formaat: "<status>-<timestamp>-goal verzonden"
                std::string data = msg->data;
                std::cout << "[DriveQuizLocation] Ontvangen bericht: " << data << std::endl;

                // Split op '-'
                std::vector<std::string> parts;
                std::stringstream ss(data);
                std::string segment;
                while (std::getline(ss, segment, '-'))
                {
                    parts.push_back(segment);
                }

                if (parts.size() < 3)
                {
                    std::cout << "[DriveQuizLocation] Ongeldig formaat ontvangen." << std::endl;
                    return;
                }

                std::string status_code = parts[0];
                std::string recv_timestamp = parts[1];

                // Alleen eerste 10 cijfers vóór de punt vergelijken
                std::string sent_prefix = sent_timestamp_.substr(0, 10);
                std::string recv_prefix = recv_timestamp.substr(0, 10);

                if (recv_prefix == sent_prefix)
                {
                    std::cout << "[DriveQuizLocation] Timestamp komt overeen ("
                              << recv_prefix << ")" << std::endl;

                    if (status_code == "04")
                    {
                        received_success_ = true;
                        std::cout << "[DriveQuizLocation] Successtatus ontvangen (04)" << std::endl;
                    }
                    else
                    {
                        std::cout << "[DriveQuizLocation] Statuscode " << status_code
                                  << " ontvangen, nog niet succesvol." << std::endl;
                    }
                }
                else
                {
                    std::cout << "[DriveQuizLocation] Timestamp mismatch: ontvangen "
                              << recv_prefix << " verwacht " << sent_prefix << std::endl;
                }
            });
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::InputPort<double>("timeout"),
            BT::OutputPort<std::string>("sent_timestamp")
        };
    }

    BT::NodeStatus onStart() override
    {
        // Publish BT node status
        std_msgs::msg::String bt_msg;
        bt_msg.data = "DriveQuizLocation";
        pub_bt_->publish(bt_msg);

        // Publish coordinate
        sent_coord_.header.stamp = node_->get_clock()->now();
        sent_coord_.header.frame_id = "map";
        sent_coord_.pose.position.x = 5.0;
        sent_coord_.pose.position.y = 2.5;
        sent_coord_.pose.position.z = 0.0;
        sent_coord_.pose.orientation.w = 1.0;
        pub_coord_->publish(sent_coord_);

        // Timestamp opslaan
        sent_timestamp_ = std::to_string(sent_coord_.header.stamp.sec) + "." +
                          std::to_string(sent_coord_.header.stamp.nanosec);

        // Opslaan in blackboard
        setOutput("sent_timestamp", sent_timestamp_);

        std::cout << "[DriveQuizLocation] Published coordinate at timestamp: " << sent_timestamp_ << std::endl;

        if (!getInput<double>("timeout", timeout_))
            timeout_ = 5.0;

        start_time_ = std::chrono::steady_clock::now();
        received_success_ = false;

        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);
        auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time_).count();

        if (received_success_)
        {
            std::cout << "[DriveQuizLocation] Succesbevestiging ontvangen -> SUCCESS" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }

        if (elapsed >= timeout_)
        {
            std::cout << "[DriveQuizLocation] Timeout (" << timeout_ << "s) -> SUCCESS (simulatie)" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[DriveQuizLocation] HALTED" << std::endl;
    }

private:
    double timeout_;
    bool received_success_;
    std::string sent_timestamp_;
    std::chrono::steady_clock::time_point start_time_;
    geometry_msgs::msg::PoseStamped sent_coord_;

    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_bt_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pub_coord_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_ack_;
};



class IsRobotAtQuiz : public BT::StatefulActionNode
{
public:
    IsRobotAtQuiz(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config), timeout_(10.0),
          success_received_(false), failure_received_(false)
    {
        node_ = rclcpp::Node::make_shared("btIsRobotAtQuiz");

        sub_ = node_->create_subscription<std_msgs::msg::String>(
            "/driveCoordStatus", 10,
            [this](std_msgs::msg::String::SharedPtr msg)
            {
                last_received_msg_ = msg->data;
            });

        pub_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }

    static BT::PortsList providedPorts()
    {
        return {
            BT::InputPort<double>("timeout"),
            BT::InputPort<std::string>("sent_timestamp")  // Timestamp uit blackboard
        };
    }

    BT::NodeStatus onStart() override
    {
        success_received_ = false;
        failure_received_ = false;
        start_time_ = std::chrono::steady_clock::now();

        if (!getInput<double>("timeout", timeout_))
            timeout_ = 10.0;

        if (!getInput<std::string>("sent_timestamp", sent_timestamp_))
            std::cout << "[IsRobotAtQuiz] Geen timestamp ontvangen van blackboard!" << std::endl;
        else
            std::cout << "[IsRobotAtQuiz] Verwachte timestamp = " << sent_timestamp_ << std::endl;

        std_msgs::msg::String msg;
        msg.data = "IsRobotAtQuiz";
        pub_->publish(msg);

        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);
        auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time_).count();

        // Check of driveCoordStatus een bericht heeft met timestamp
        if (!sent_timestamp_.empty())
        {
            std::string expected = "Ontvangen " + sent_timestamp_;
            if (last_received_msg_ == expected)
            {
                std::cout << "[IsRobotAtQuiz] Correcte bevestiging ontvangen -> SUCCESS" << std::endl;
                return BT::NodeStatus::SUCCESS;
            }
        }

        if (elapsed >= timeout_)
        {
            std::cout << "[IsRobotAtQuiz] Timeout (" << timeout_ << "s) -> FAILURE" << std::endl;
            return BT::NodeStatus::FAILURE;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        std::cout << "[IsRobotAtQuiz] HALTED" << std::endl;
    }

private:
    double timeout_;
    bool success_received_;
    bool failure_received_;
    std::chrono::steady_clock::time_point start_time_;
    std::string sent_timestamp_;
    std::string last_received_msg_;

    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};



class RobotAtQuiz : public BT::SyncActionNode
{
public:
    RobotAtQuiz(const std::string &name) : BT::SyncActionNode(name, {}) {
        node_ = rclcpp::Node::make_shared("bt_robot_at_quiz_node");

        // Bestaande publisher behouden
        pub_quiz_ = node_->create_publisher<std_msgs::msg::String>("/rpitopic", 10);

        // Nieuwe publisher voor BehaviorTree-node status
        pub_bt_ = node_->create_publisher<std_msgs::msg::String>("/BehaviorTreeNode", 10);
    }

    BT::NodeStatus tick() override {

        std::string state = "robot-arrived-at-quiz-location";
        std_msgs::msg::String msg;
        msg.data = state;
        pub_quiz_->publish(msg);


        std::string bt_state = "RobotAtQuiz";
        std_msgs::msg::String bt_msg;
        bt_msg.data = bt_state;
        pub_bt_->publish(bt_msg);

        std::cout << "[RobotAtQuiz] Robot arrived at quiz location, published to /quiz_pi_con and /BehaviorTreeNode" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_quiz_;  // bestaande publisher
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_bt_;    // nieuwe publisher
};


class BatteryCharged: public TimedCondition 
{ 
public: 
     BatteryCharged(const std::string &name, const BT::NodeConfiguration &config) : TimedCondition(name, config){} 
     };


// -------------------------
// Timer nodes (met timeout uit XML)
// -------------------------
class Check_at_workarea: public TimedCondition 
{ 
public: 
     Check_at_workarea(const std::string &name, const BT::NodeConfiguration &config) : TimedCondition(name, config){} 
     };
     
class check_ArrivedAtPerson : public TimedCondition 
{ 
public: 
      check_ArrivedAtPerson(const std::string &name, const BT::NodeConfiguration &config) : TimedCondition(name, config) {} 
};


class WaitQuizToEnd : public TimedCondition 
{ 
public: WaitQuizToEnd(const std::string &name, const BT::NodeConfiguration &config) : TimedCondition(name, config) {} 
};

// -------------------------
// MAIN
// -------------------------
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    BT::BehaviorTreeFactory factory;

    // registreer nodes
    // < > geeft de naam van de c++ node, de (" ... ") geeft de naam van de node in de XML file waar je deze c++ code aan koppelt

    factory.registerNodeType<InWorkingZone>("InWorkingZone");
    factory.registerNodeType<MoveLocationWorkarea>("MoveLocationWorkarea");
    factory.registerNodeType<Check_at_workarea>("Check_at_workarea");
    factory.registerNodeType<RobotExplore>("RobotExplore");
    factory.registerNodeType<CheckCoordAvailable>("CheckCoordAvailable");
    factory.registerNodeType<MoveToVisitor>("MoveToVisitor");
    factory.registerNodeType<check_ArrivedAtPerson>("check_ArrivedAtPerson");
    factory.registerNodeType<robotAtPerson>("robotAtPerson");
    factory.registerNodeType<CheckStartButton>("CheckStartButton");
    factory.registerNodeType<DriveQuizLocation>("DriveQuizLocation");
    factory.registerNodeType<IsRobotAtQuiz>("IsRobotAtQuiz");
    factory.registerNodeType<RobotAtQuiz>("RobotAtQuiz");
    factory.registerNodeType<WaitQuizToEnd>("WaitQuizToEnd");
    factory.registerNodeType<BatteryOk>("BatteryOk");
    factory.registerNodeType<InChargingStation>("InChargingStation");
    factory.registerNodeType<DriveToChargingStation>("DriveToChargingStation");
    factory.registerNodeType<StatusDriveToChargingDock>("StatusDriveToChargingDock");
    factory.registerNodeType<IsRobotCharging>("IsRobotCharging");
    factory.registerNodeType<IsBatteryFull>("IsBatteryFull");
    factory.registerNodeType<BatteryCharged>("BatteryCharged");
    factory.registerNodeType<BatteryStopDrive>("BatteryStopDrive");
    factory.registerNodeType<CheckCollision>("CheckCollision");
    factory.registerNodeType<CheckNetworkError>("CheckNetworkError");
    factory.registerNodeType<StopNode>("StopNode");
    factory.registerNodeType<WaitDriving>("WaitDriving");
    factory.registerNodeType<ForceSuccess>("RightBranch");

    // laad boom uit XML
    auto tree = factory.createTreeFromFile("src/mecabot_bt/trees/behavior_tree.xml");

    std::cout << "--- Starting BT in continuous mode ---" << std::endl;
    rclcpp::Rate loop_rate(1.0); // 1 Hz tick = max rate

    while (rclcpp::ok())
    {
        BT::NodeStatus status = tree.tickRoot();

        if (status == BT::NodeStatus::SUCCESS) {
            std::cout << "--- Tree ticked to SUCCESS ---" << std::endl;
            // Optioneel: reset de boom zodat sommige nodes opnieuw kunnen uitvoeren
            tree.rootNode()->halt();
        }
        else if (status == BT::NodeStatus::FAILURE) {
            std::cout << "--- Tree ticked to FAILURE ---" << std::endl;
            // Optioneel: reset de boom om opnieuw te proberen
            tree.rootNode()->halt();
        }

        loop_rate.sleep();
    }

    rclcpp::shutdown();
    return 0;
}

