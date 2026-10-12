// Copyright 2026 INF3995. MIT. See vendor/guide/LICENSE for Unitree algorithms.
#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Joint.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/Util.hh>
#include <gz/plugin/Register.hh>
#include <gz/transport/Node.hh>
#include <gz/msgs/twist.pb.h>
#include <gz/sim/components/JointForceCmd.hh>
#include <gz/sim/components/Pose.hh>
#include "control/CtrlComponents.h"
#include "FSM/State_Trotting.h"
#include <mutex>
#include <algorithm>
#include <memory>
#include <sstream>

class Go2Kinematics : public QuadrupedRobot {
 public: Go2Kinematics(double speedScale=1.0){
  // Guide order FR, FL, RR, RL; lengths and origins from official Go2 URDF.
  for(int i=0;i<4;i++){
   double x=i<2?.1934:-.1934,y=i%2?.0465:-.0465;
   _Legs[i]=new QuadrupedLeg(i,.0955,.213,.213,Vec3(x,y,0));
   _feetPosNormalStand.col(i)=Vec3(x,i%2?.142:-.142,-.32);
  }
  _robVelLimitX=Vec2(-.4,.4)*speedScale;_robVelLimitY=Vec2(-.3,.3)*speedScale;_robVelLimitYaw=Vec2(-.5,.5)*speedScale;
  _mass=15.019;_pcb=Vec3(.01,0,0);_Ib=Vec3(.12,.35,.40).asDiagonal();
 }
};
class SimIO : public IOInterface {
 public: SimIO(){cmdPanel=new CmdPanel();}
 void sendRecv(const LowlevelCmd*,LowlevelState*) override {}
};
class Go2Controller : public gz::sim::System,public gz::sim::ISystemConfigure,public gz::sim::ISystemPreUpdate {
 gz::sim::Model model;gz::sim::Link base;
 std::array<gz::sim::Joint,12> joints;
 std::unique_ptr<CtrlComponents> ctrl;
 std::unique_ptr<State_Trotting> gait;
 gz::transport::Node node;std::mutex mutex;
 double vx=0,vy=0,wz=0,received=-100,time=0;
 double speedScale=1.0;
 bool initialized=false,entered=false;
 std::string name;
 public:
 void Configure(const gz::sim::Entity &e,const std::shared_ptr<const sdf::Element>&sdf,gz::sim::EntityComponentManager &ecm,gz::sim::EventManager&) override {
  model=gz::sim::Model(e);name=model.Name(ecm);base=gz::sim::Link(model.LinkByName(ecm,"base"));
  base.EnableVelocityChecks(ecm);base.EnableAccelerationChecks(ecm);
  if(!ecm.Component<gz::sim::components::WorldPose>(base.Entity()))ecm.CreateComponent(base.Entity(),gz::sim::components::WorldPose());
  const std::array<std::string,4> legs={"FR","FL","RR","RL"};const std::array<std::string,3> axes={"hip","thigh","calf"};
  for(int i=0;i<12;i++){
   joints[i]=gz::sim::Joint(model.JointByName(ecm,legs[i/3]+"_"+axes[i%3]+"_joint"));
   joints[i].EnablePositionCheck(ecm);joints[i].EnableVelocityCheck(ecm);
  }
  inf3995SimMicros=0;
  ctrl=std::make_unique<CtrlComponents>(new SimIO());ctrl->dt=.002;ctrl->ctrlPlatform=CtrlPlatform::GAZEBO;
  speedScale=sdf->HasElement("speed_scale")?sdf->Get<double>("speed_scale"):1.0;
  double gaitPeriod=sdf->HasElement("gait_period")?sdf->Get<double>("gait_period"):.50;
  if(!std::isfinite(speedScale)||speedScale<=0||!std::isfinite(gaitPeriod)||gaitPeriod<=0)throw std::runtime_error("Invalid Go2 speed/gait configuration");
  ctrl->robotModel=new Go2Kinematics(speedScale);ctrl->waveGen=new WaveGenerator(gaitPeriod,.6,Vec4(0,.5,.5,0));ctrl->geneObj();ctrl->setAllStance();
  if(sdf->HasElement("terrain")){
   struct Surface {double xmin,xmax,ymin,ymax;gz::math::Vector3d plane;};
   std::vector<Surface> surfaces;
   auto configuration=sdf->Clone();auto terrain=configuration->GetElement("terrain");
   for(auto s=terrain->GetElement("surface");s;s=s->GetNextElement("surface")){
    Surface surface;std::istringstream bounds(s->Get<std::string>("bounds"));
    if(!(bounds>>surface.xmin>>surface.xmax>>surface.ymin>>surface.ymax))throw std::runtime_error("Invalid terrain bounds");
    surface.plane=s->Get<gz::math::Vector3d>("plane");surfaces.push_back(surface);
   }
   ctrl->terrainSurface=[surfaces](double x,double y){
    for(const auto &s:surfaces)if(x>=s.xmin&&x<=s.xmax&&y>=s.ymin&&y<=s.ymax)
     return Vec3(s.plane.X()*x+s.plane.Y()*y+s.plane.Z(),s.plane.X(),s.plane.Y());
    return Vec3(0,0,0);
   };
  }
  gait=std::make_unique<State_Trotting>(ctrl.get());
  node.Subscribe<gz::msgs::Twist>("/"+name+"/cmd_vel",[this](const gz::msgs::Twist&m){std::lock_guard<std::mutex> guard(mutex);vx=std::clamp(m.linear().x(),-.4*speedScale,.4*speedScale);vy=std::clamp(m.linear().y(),-.3*speedScale,.3*speedScale);wz=std::clamp(m.angular().z(),-.5*speedScale,.5*speedScale);received=time;});
 }
 void PreUpdate(const gz::sim::UpdateInfo &info,gz::sim::EntityComponentManager &ecm) override {
  if(info.paused)return;
  double t=std::chrono::duration<double>(info.simTime).count();
  inf3995SimMicros=static_cast<long long>(t*1e6);
  double x,y,w;{std::lock_guard<std::mutex> guard(mutex);time=t;x=vx;y=vy;w=wz;if(t-received>.5)x=y=w=0;}
  if(!initialized){
   for(int i=0;i<12;i++)joints[i].ResetPosition(ecm,{i%3==0?0.0:i%3==1?.8:-1.5});
   initialized=true;return;
  }
  auto pose=base.WorldPose(ecm);auto v=base.WorldLinearVelocity(ecm);auto omega=base.WorldAngularVelocity(ecm);
  if(!pose||!v||!omega)return;
  auto rot=pose->Rot();auto bodyW=rot.RotateVectorReverse(*omega);
  auto &state=*ctrl->lowState;state.imu.quaternion[0]=rot.W();state.imu.quaternion[1]=rot.X();state.imu.quaternion[2]=rot.Y();state.imu.quaternion[3]=rot.Z();
  for(int k=0;k<3;k++)state.imu.gyroscope[k]=bodyW[k];
  for(int i=0;i<12;i++){auto q=joints[i].Position(ecm);auto dq=joints[i].Velocity(ecm);if(!q||q->empty()||!dq||dq->empty())return;state.motorState[i].q=(*q)[0];state.motorState[i].dq=(*dq)[0];}
  ctrl->estimator->setSimulationTruth(Vec3(pose->Pos().X(),pose->Pos().Y(),pose->Pos().Z()),Vec3(v->X(),v->Y(),v->Z()));
  if(t>2.0&&!entered){gait->enter();entered=true;}
  if(entered){state.userValue.ly=x/(.4*speedScale);state.userValue.lx=-y/(.3*speedScale);state.userValue.rx=-w/(.5*speedScale);ctrl->runWaveGen();gait->run();}
  for(int i=0;i<12;i++){
   double torque;
   if(!entered){double target=i%3==0?0:i%3==1?.8:-1.5;torque=60*(target-state.motorState[i].q)-3*state.motorState[i].dq;}
   else {auto &cmd=ctrl->lowCmd->motorCmd[i];torque=cmd.tau+cmd.Kp*(cmd.q-state.motorState[i].q)+cmd.Kd*(cmd.dq-state.motorState[i].dq);}
   if(!std::isfinite(torque))torque=0;
   // Official Go2 actuator limits: hip/thigh 23.7 Nm, calf 45.43 Nm.
   double limit=i%3==2?45.43:23.7;torque=std::clamp(torque,-limit,limit);
   auto component=ecm.Component<gz::sim::components::JointForceCmd>(joints[i].Entity());
   if(component)component->Data()={torque};else ecm.CreateComponent(joints[i].Entity(),gz::sim::components::JointForceCmd({torque}));
  }
 }
};
GZ_ADD_PLUGIN(Go2Controller,gz::sim::System,Go2Controller::ISystemConfigure,Go2Controller::ISystemPreUpdate)
GZ_ADD_PLUGIN_ALIAS(Go2Controller,"inf3995::Go2Controller")
