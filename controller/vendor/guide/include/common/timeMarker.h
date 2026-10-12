// INF3995 adaptation: use Gazebo simulation time, no wall-clock sleeps.
#ifndef TIMEMARKER_H
#define TIMEMARKER_H
inline thread_local long long inf3995SimMicros=0;
inline long long getSystemTime(){return inf3995SimMicros;}
inline double getTimeSecond(){return inf3995SimMicros*1e-6;}
inline void absoluteWait(long long,long long){}
#endif
